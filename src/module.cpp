#include <pybind11/pybind11.h>
#include <pybind11/numpy.h>
#include <srrg_hbst/types/binary_matchable.hpp>
#include <srrg_hbst/types/binary_node.hpp>
#include <srrg_hbst/types/binary_tree.hpp>
#include <math.h>

namespace py = pybind11;

#define STRINGIFY(x) #x
#define MACRO_STRINGIFY(x) STRINGIFY(x)


template <typename BinaryNodeType_>
class BinaryTreeAdapter : public srrg_hbst::BinaryTree<BinaryNodeType_> {
public:
    using BinaryTree = srrg_hbst::BinaryTree<BinaryNodeType_>;
    using ObjectType = typename BinaryTree::ObjectType;
    using Matchable = typename BinaryTree::Matchable;
    using MatchableVector = typename BinaryTree::MatchableVector;
    using Descriptor = typename BinaryTree::Descriptor;

    void add(uint16_t imageId, py::array_t<ObjectType> descriptorIds, py::array_t<uint8_t> descriptors, srrg_hbst::SplittingStrategy trainMode, bool padDescriptorsIfRequired) {
        MatchableVector matchables = std::move(buildMatchableVector(imageId, descriptorIds, descriptors, padDescriptorsIfRequired));
        BinaryTree::add(matchables, trainMode);
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

    // std::vector<_BinaryMatchable256> match(py::array_t<uint8_t> queryDescriptors, uint32_t maximumDistance, bool lazy) {

    //     if (lazy) {
    //         matchLazy(, maximumDistance);
    //     }
    // }


    static void bind(pybind11::module_& m, std::string name) {
        using cls = BinaryTreeAdapter<BinaryNodeType_>;
        py::class_<cls>(m, name.c_str())
            .def(py::init<>())
            .def("add", &cls::add)
            .def("train", &cls::train)
            .def("clear", &cls::clear)
            .def("read", &cls::read)
            .def("write", &cls::write)
            .def_static("get_desc_size_in_bits", &cls::getDescriptorSizeInBits)
            .def_static("get_desc_overflow_bits", &cls::getDescriptorOverflowBits)
            .def_static("get_desc_size_in_bytes", &cls::getDescriptorSizeInBytes)
            // .def("numberOfMatchablesUncompressed", &cls::numberOfMatchablesUncompressed)
            // .def("numberOfMatchablesCompressed", &cls::numberOfMatchablesCompressed)
            .def("size", &cls::size);
    }

private:
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

    static MatchableVector buildMatchableVector(uint64_t imageId, py::array_t<ObjectType>& descriptorIds, py::array_t<uint8_t>& descriptors, bool pad) {
        if (descriptorIds.ndim() != 1) {
            throw std::runtime_error("Incompatible buffer shape for descriptor ids, expected 1d array");
        }
        if (descriptors.ndim() != 2) {
            throw std::runtime_error("Incompatible buffer shape for descriptors, expected 2d array");
        }
        uint32_t expectedByteCount = getDescriptorSizeInBytes();
        if (!pad && descriptors.shape(1) != expectedByteCount) {
            throw std::runtime_error("Incompatible buffer shape for descriptors, dimension 0: number of descriptors, dimension 1: " + std::to_string(expectedByteCount));
        }
        if (pad && descriptors.shape(1) > expectedByteCount) {
            throw std::runtime_error("Incompatible buffer shape for descriptors, dimension 0: number of descriptors, dimension 1: <=" + std::to_string(expectedByteCount));
        }

        MatchableVector matchables(descriptorIds.shape(0));
        for (uint64_t i = 0; i < descriptorIds.shape(0); ++i) {
            Descriptor d = buildDescriptor(descriptors.data(i));
            matchables[i] = new Matchable(descriptorIds.at(i), d, imageId); // TODO: turn uint8 array row into a bitset
        }

        return matchables;
    }
};


typedef BinaryTreeAdapter<srrg_hbst::BinaryNode128<uint64_t>> BinaryTreeAdapter128;
typedef BinaryTreeAdapter<srrg_hbst::BinaryNode256<uint64_t>> BinaryTreeAdapter256;
typedef BinaryTreeAdapter<srrg_hbst::BinaryNode512<uint64_t>> BinaryTreeAdapter512;


PYBIND11_MODULE(hbst, m) {
    py::enum_<srrg_hbst::SplittingStrategy>(m, "SplittingStrategy")
        .value("DoNothing", srrg_hbst::SplittingStrategy::DoNothing)
        .value("SplitEven", srrg_hbst::SplittingStrategy::SplitEven)
        .value("SplitUneven", srrg_hbst::SplittingStrategy::SplitUneven)
        .value("SplitRandomUniform", srrg_hbst::SplittingStrategy::SplitRandomUniform)
        .export_values();

    BinaryTreeAdapter128::bind(m, "BinaryTree128");
    // BinaryTreeAdapter256::bind(m, "BinaryTree256");
    // BinaryTreeAdapter512::bind(m, "BinaryTree512");

#ifdef VERSION_INFO
    m.attr("__version__") = MACRO_STRINGIFY(VERSION_INFO);
#else
    m.attr("__version__") = "dev";
#endif
}