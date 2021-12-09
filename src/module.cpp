#include <pybind11/pybind11.h>
#include <pybind11/numpy.h>
#include <srrg_hbst/types/binary_tree.hpp>
#include <srrg_hbst/types/binary_node.hpp>

namespace py = pybind11;

#define STRINGIFY(x) #x
#define MACRO_STRINGIFY(x) STRINGIFY(x)


typedef srrg_hbst::BinaryTree128<uint64_t> _BinaryTree128;
typedef srrg_hbst::BinaryTree256<uint64_t> _BinaryTree256;
typedef srrg_hbst::BinaryTree512<uint64_t> _BinaryTree512;


class BinaryTree256 : public _BinaryTree256 {
public:
    /* Inherit the constructors */
    using _BinaryTree256::_BinaryTree256;

    void add(uint64_t imageId, py::array_t<uint64_t> descriptorIds, py::array_t<uint64_t> descriptors, srrg_hbst::SplittingStrategy trainMode) {
        BinaryTree256::MatchableVector matchables = std::move(buildMatchableVector(imageId, descriptorIds, descriptors));
        _BinaryTree256::add(matchables, trainMode);
    }

private:
    static BinaryTree256::MatchableVector buildMatchableVector(uint64_t imageId, py::array_t<uint64_t>& descriptorIds, py::array_t<uint64_t>& descriptors) {
        if (descriptorIds.ndim() != 1) {
            throw std::runtime_error("Incompatible buffer shape for descriptor ids, expected 1d array");
        }
        if (descriptors.ndim() != 1) {
            throw std::runtime_error("Incompatible buffer shape for descriptors, expected 1d array");
        }

        BinaryTree256::MatchableVector matchables(descriptorIds.shape(0));
        for (uint64_t i = 0; i < descriptorIds.shape(0); ++i) {
            matchables[i] = new BinaryTree256::Matchable(descriptorIds.at(i), descriptors.at(i), imageId);
        }

        return matchables;
    }
};


int add(int i, int j) {
    return i + j;
}

namespace py = pybind11;

PYBIND11_MODULE(hbst, m) {
    py::enum_<srrg_hbst::SplittingStrategy>(m, "SplittingStrategy")
        .value("DoNothing", srrg_hbst::SplittingStrategy::DoNothing)
        .value("SplitEven", srrg_hbst::SplittingStrategy::SplitEven)
        .value("SplitUneven", srrg_hbst::SplittingStrategy::SplitUneven)
        .value("SplitRandomUniform", srrg_hbst::SplittingStrategy::SplitRandomUniform)
        .export_values();

    py::class_<BinaryTree256>(m, "BinaryTree256")
        .def(py::init<>())
        .def("add", &BinaryTree256::add)
        .def("read", &BinaryTree256::read)
        .def("write", &BinaryTree256::write)
        .def("clear", &BinaryTree256::clear)
        .def("train", &BinaryTree256::train)
        .def("numberOfMatchablesUncompressed", &BinaryTree256::numberOfMatchablesUncompressed)
        .def("numberOfMatchablesCompressed", &BinaryTree256::numberOfMatchablesCompressed)
        .def("size", &BinaryTree256::size);

    m.def("add", &add, R"pbdoc(
        Add two numbers
        Some other explanation about the add function.
    )pbdoc");

#ifdef VERSION_INFO
    m.attr("__version__") = MACRO_STRINGIFY(VERSION_INFO);
#else
    m.attr("__version__") = "dev";
#endif
}