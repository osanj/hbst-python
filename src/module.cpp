#include <pybind11/pybind11.h>
#include <pybind11/numpy.h>
#include <srrg_hbst/types/binary_matchable.hpp>
#include <srrg_hbst/types/binary_node.hpp>
#include <srrg_hbst/types/binary_tree.hpp>

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

    void add(ObjectType imageId, py::array_t<ObjectType> descriptorIds, py::array_t<uint8_t> descriptors, srrg_hbst::SplittingStrategy trainMode) {
        MatchableVector matchables = std::move(buildMatchableVector(imageId, descriptorIds, descriptors));
        BinaryTree::add(matchables, trainMode);
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
            .def("read", &cls::read)
            .def("write", &cls::write)
            .def("clear", &cls::clear)
            .def("train", &cls::train)
            .def("numberOfMatchablesUncompressed", &cls::numberOfMatchablesUncompressed)
            .def("numberOfMatchablesCompressed", &cls::numberOfMatchablesCompressed)
            .def("size", &cls::size);
    }

private:
    static MatchableVector buildMatchableVector(ObjectType imageId, py::array_t<ObjectType>& descriptorIds, py::array_t<uint8_t>& descriptors) {
        if (descriptorIds.ndim() != 1) {
            throw std::runtime_error("Incompatible buffer shape for descriptor ids, expected 1d array");
        }
        if (descriptors.ndim() != 2) {
            throw std::runtime_error("Incompatible buffer shape for descriptors, expected 2d array");
        }
        // if (descriptors.ndim() != 2) {
        //     throw std::runtime_error("Incompatible buffer shape for descriptors, expected 1d array");
        // }

        // BinaryTree<BinaryNodeType_>::BinaryMatchable::descriptor_size_bits_in_bytes;

        MatchableVector matchables(descriptorIds.shape(0));
        for (uint64_t i = 0; i < descriptorIds.shape(0); ++i) {
            matchables[i] = new Matchable(descriptorIds.at(i), descriptors.at(i), imageId);
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
    BinaryTreeAdapter256::bind(m, "BinaryTree256");
    BinaryTreeAdapter512::bind(m, "BinaryTree512");

#ifdef VERSION_INFO
    m.attr("__version__") = MACRO_STRINGIFY(VERSION_INFO);
#else
    m.attr("__version__") = "dev";
#endif
}