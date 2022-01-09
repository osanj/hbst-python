#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/numpy.h>
#include <srrg_hbst/types/binary_matchable.hpp>
#include <srrg_hbst/types/binary_node.hpp>
#include <srrg_hbst/types/binary_tree.hpp>
#include <algorithm>
#include <math.h>
#include "threadpool.h"

namespace py = pybind11;

#define STRINGIFY(x) #x
#define MACRO_STRINGIFY(x) STRINGIFY(x)


template <typename BinaryNodeType_>
class BinaryTreeAdapter : public srrg_hbst::BinaryTree<BinaryNodeType_> {
public:
    using BinaryTree = srrg_hbst::BinaryTree<BinaryNodeType_>;
    using ObjectType = typename BinaryTree::ObjectType;
    using Match = typename BinaryTree::Match;
    using Matchable = typename BinaryTree::Matchable;
    using MatchableVector = typename BinaryTree::MatchableVector;
    using MatchVector = typename BinaryTree::MatchVector;
    using Descriptor = typename BinaryTree::Descriptor;

    BinaryTreeAdapter(uint32_t _threads, bool _padDescriptorsIfRequired) : padDescriptorsIfRequired(_padDescriptorsIfRequired) {
        if (_threads > 1) {
            threadpool = new Threadpool(_threads);
        }
    }

    ~BinaryTreeAdapter() {
        if (threadpool != NULL) {
            delete threadpool;
        }
    }

    void add(uint16_t imageId, py::array_t<ObjectType> descriptorIds, py::array_t<uint8_t> descriptors) {
        MatchableVector matchables = std::move(buildMatchableVector(imageId, descriptorIds, descriptors));
        BinaryTree::add(matchables, srrg_hbst::SplittingStrategy::DoNothing);
    }

    static uint32_t getDescriptorSizeInBits() {
        return Matchable::descriptor_size_bits;
    }

    static uint32_t getDescriptorOverflowBits() {
        return Matchable::descriptor_size_bits_overflow;
    }

    static uint32_t getDescriptorSizeInBytes() {
        return ceil(getDescriptorSizeInBits() / 8.);
    }

    MatchVector match(py::array_t<ObjectType> queryDescriptorIds, py::array_t<uint8_t> queryDescriptors, uint32_t maximumDistance, bool lazy) {
        MatchVector allMatches;
        MatchableVector query = buildMatchableVector(0, queryDescriptorIds, queryDescriptors);

        if (threadpool != NULL) {
            uint32_t splitStep = query.size() / threadpool->size();
            std::vector<MatchVector*> matches;
            std::vector<std::future<void>> futures;

            for (int i = 0; i < threadpool->size(); i++) {
                MatchVector* m = new MatchVector();
                auto t = [&]() -> void {
                    matchTask(query, i * splitStep, (i + 1) * splitStep, *m, maximumDistance, lazy);
                };
                matches.push_back(m);
                futures.push_back(threadpool->enqueue<std::function<void()>, void>(t));
            }

            for (int i = 0; i < threadpool->size(); i++) {
                futures[i].get();
                MatchVector* m = matches[i];
                allMatches.insert(allMatches.end(), m->begin(), m->end());
                delete m;
            }

        } else {
            matchTask(query, 0, query.size(), allMatches, maximumDistance, lazy);
        }

        return allMatches;
    }

    void matchTask(MatchableVector& query, uint32_t offset, uint32_t size, MatchVector& matches, uint32_t maximumDistance, bool lazy) {
        MatchableVector mv;
        if (offset == 0 && size >= query.size()) {
            mv = query;
        } else {
            int limit = std::min((uint32_t) query.size(), offset + size);
            mv.insert(mv.end(), query.begin() + offset, query.end() + limit);
        }
        if (lazy) {
            BinaryTree::matchLazy(mv, matches, maximumDistance);
        } else {
            BinaryTree::match(mv, matches, maximumDistance);
        }
    }

    static std::pair<std::unordered_map<ObjectType, MatchVector>, std::vector<ObjectType>> partitionMatches(MatchVector& matches) {
        std::unordered_map<ObjectType, MatchVector> partitions;

        for (const auto& match : matches) {
            for (const auto& ref : match.matchable_references) {
                for (const auto& kvPair : ref->objects) {
                    ObjectType key = kvPair.first;
                    if (partitions.find(key) == partitions.end()) {
                        partitions[key] = MatchVector();
                    }
                    partitions.at(key).push_back(match);
                }
            }
        }

        std::vector<ObjectType> sortedImageIds;
        sortedImageIds.reserve(partitions.size());
        for (const auto& kvPair : partitions) {
            sortedImageIds.push_back(kvPair.first);
        }

        std::sort(sortedImageIds.begin(), sortedImageIds.end(), [&](const ObjectType &a, const ObjectType &b) {
            return partitions.at(a).size() > partitions.at(b).size();
        });

        return std::make_pair(partitions, sortedImageIds);
    }

    static void bind(pybind11::module_& m, std::string name) {
        using clsTree = BinaryTreeAdapter<BinaryNodeType_>;
        auto tree = py::class_<clsTree>(m, name.c_str());
        tree.def(py::init<uint32_t, bool>(), py::arg("threads") = 1, py::arg("pad_descriptors_if_required") = false);
        tree.def("add", &clsTree::add, py::arg("image_id"), py::arg("descriptor_ids"), py::arg("descriptors"));
        tree.def("train", &clsTree::train, py::arg("mode") = srrg_hbst::SplittingStrategy::SplitEven);
        tree.def("match", &clsTree::match, py::arg("query_descriptor_ids"), py::arg("query_descriptors"), py::arg("max_distance") = 25, py::arg("lazy") = false);
        tree.def("clear", &clsTree::clear);
        tree.def("read", &clsTree::read, py::arg("file_path"));
        tree.def("write", &clsTree::write, py::arg("file_path"));
        tree.def_static("partition_matches", &clsTree::partitionMatches, py::arg("matches"));
        tree.def_static("get_desc_size_in_bits", &clsTree::getDescriptorSizeInBits);
        tree.def_static("get_desc_overflow_bits", &clsTree::getDescriptorOverflowBits);
        tree.def_static("get_desc_size_in_bytes", &clsTree::getDescriptorSizeInBytes);
        tree.def("size", &clsTree::size);

        using clsMatchable = BinaryTreeAdapter<BinaryNodeType_>::Matchable;
        auto matchable = py::class_<clsMatchable>(tree, "Matchable");
        // matchable.def_property_readonly("descriptor", [](const clsMatchable &m) { return m.descriptor; }; // TODO: convert to numpy array
        matchable.def_readonly("descriptor_id_by_image_id", &clsMatchable::objects); // check memory situation

        using clsMatch = BinaryTreeAdapter<BinaryNodeType_>::Match;
        auto match = py::class_<clsMatch>(tree, "Match");
        match.def_readonly("distance", &clsMatch::distance);
        match.def_readonly("query_descriptor", &clsMatch::matchable_query);
        match.def_readonly("query_descriptor_id", &clsMatch::object_query);
        match.def_readonly("match_ids", &clsMatch::object_references);
        match.def_readonly("match_refs", &clsMatch::matchable_references);
        match.def_property_readonly("first_match_id", [](const clsMatch &m) { return m.object_references.front(); });
    }

private:
    bool padDescriptorsIfRequired;
    Threadpool* threadpool;


    static Descriptor buildDescriptor(const u_char* descriptor) {
        // see getDescriptor for SRRG_HBST_HAS_OPENCV in binary_matchable.hpp
        Descriptor binaryDescriptor; // padding is done implicitely, instantiation zeros all bits
        uint32_t overflow = getDescriptorOverflowBits();
        uint32_t sizeInBytes = Matchable::raw_descriptor_size_bytes; // not using getDescriptorSizeInBytes because original library does not use ceil

        for (uint64_t byteIndex = 0; byteIndex < sizeInBytes; ++byteIndex) {
            const uint32_t bitIndexStart = byteIndex * 8;
            const std::bitset<8> descriptorByte(descriptor[byteIndex]);
            for (uint8_t v = 0; v < 8; ++v) {
                binaryDescriptor[bitIndexStart + v] = descriptorByte[v];
            }
        }

        // in case the last byte is not fully used
        if (overflow > 0) {
            const std::bitset<8> descriptorByte(descriptor[sizeInBytes]);
            for (uint32_t v = 0; v < overflow; ++v) {
                binaryDescriptor[Matchable::descriptor_size_bits_in_bytes + v] = descriptorByte[8 - overflow + v];
            }
        }
        return binaryDescriptor;
    }

    MatchableVector buildMatchableVector(uint64_t imageId, py::array_t<ObjectType>& descriptorIds, py::array_t<uint8_t>& descriptors) {
        if (descriptorIds.ndim() != 1) {
            throw std::runtime_error("Incompatible buffer shape for descriptor ids, expected 1d array");
        }
        if (descriptors.ndim() != 2) {
            throw std::runtime_error("Incompatible buffer shape for descriptors, expected 2d array");
        }
        if (descriptorIds.shape(0) != descriptors.shape(0)) {
            throw std::runtime_error("Inconsistent buffer shapes, descriptor id count and descriptor count does not match");
        }
        uint32_t expectedByteCount = getDescriptorSizeInBytes();
        if (!padDescriptorsIfRequired && descriptors.shape(1) != expectedByteCount) {
            throw std::runtime_error("Incompatible buffer shape for descriptors, dimension 0: number of descriptors, dimension 1: " + std::to_string(expectedByteCount));
        }
        if (padDescriptorsIfRequired && descriptors.shape(1) > expectedByteCount) {
            throw std::runtime_error("Incompatible buffer shape for descriptors, dimension 0: number of descriptors, dimension 1: <=" + std::to_string(expectedByteCount));
        }

        MatchableVector matchables(descriptorIds.shape(0));
        for (uint64_t i = 0; i < descriptorIds.shape(0); ++i) {
            Descriptor d = buildDescriptor(descriptors.data(i));
            matchables[i] = new Matchable(descriptorIds.at(i), d, imageId);
        }

        return matchables;
    }
};

namespace srrg_hbst {
    template <typename ObjectType_>
    using BinaryMatchable488 = srrg_hbst::BinaryMatchable<ObjectType_, 488>;
    template <typename ObjectType_>
    using BinaryNode488 = srrg_hbst::BinaryNode<BinaryMatchable488<ObjectType_>>;
}

typedef BinaryTreeAdapter<srrg_hbst::BinaryNode128<uint64_t>> BinaryTreeAdapter128;
typedef BinaryTreeAdapter<srrg_hbst::BinaryNode256<uint64_t>> BinaryTreeAdapter256;
typedef BinaryTreeAdapter<srrg_hbst::BinaryNode488<uint64_t>> BinaryTreeAdapter488;
typedef BinaryTreeAdapter<srrg_hbst::BinaryNode512<uint64_t>> BinaryTreeAdapter512;


PYBIND11_MODULE(hbst, m) {
    py::enum_<srrg_hbst::SplittingStrategy>(m, "SplittingStrategy")
        .value("DoNothing", srrg_hbst::SplittingStrategy::DoNothing)
        .value("SplitEven", srrg_hbst::SplittingStrategy::SplitEven)
        .value("SplitUneven", srrg_hbst::SplittingStrategy::SplitUneven)
        .value("SplitRandomUniform", srrg_hbst::SplittingStrategy::SplitRandomUniform)
        .export_values();

    BinaryTreeAdapter128::bind(m, "BinaryTree128");
    BinaryTreeAdapter256::bind(m, "BinaryTree256");
    BinaryTreeAdapter488::bind(m, "BinaryTree488");
    BinaryTreeAdapter512::bind(m, "BinaryTree512");

#ifdef VERSION_INFO
    m.attr("__version__") = MACRO_STRINGIFY(VERSION_INFO);
#else
    m.attr("__version__") = "dev";
#endif
}