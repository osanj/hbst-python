#include <pybind11/pybind11.h>
#include <srrg_hbst/types/binary_tree.hpp>
#include <srrg_hbst/types/binary_node.hpp>

#define STRINGIFY(x) #x
#define MACRO_STRINGIFY(x) STRINGIFY(x)


//typedef srrg_hbst::BinaryTree128<uint32_t> _BinaryTree128;
typedef srrg_hbst::BinaryTree256<uint64_t> _BinaryTree256;
//typedef srrg_hbst::BinaryTree512<long> _BinaryTree512;

//  using BinaryTree128 = BinaryTree<BinaryNode128<ObjectType_>>;
//  template <typename ObjectType_>
//  using BinaryTree256 = BinaryTree<BinaryNode256<ObjectType_>>;
//  template <typename ObjectType_>
//  using BinaryTree512 = BinaryTree<BinaryNode512<ObjectType_>>;


class BinaryTree256 : public _BinaryTree256 {
public:
    /* Inherit the constructors */
    using _BinaryTree256::_BinaryTree256;

    static BinaryTree256 create(std::vector<uint64_t> imageIds, std::vector<uint64_t> descriptors, srrg_hbst::SplittingStrategy trainMode) {
        if (imageIds.size() == 0) {
            return BinaryTree256();
        }
        BinaryTree256::MatchableVector matchables = buildMatchableVector(imageIds, descriptors);
        return BinaryTree256(matchables, trainMode);
    }

    void add(std::vector<uint64_t> imageIds, std::vector<uint64_t> descriptors, srrg_hbst::SplittingStrategy trainMode) {
        BinaryTree256::MatchableVector matchables = buildMatchableVector(imageIds, descriptors);
        _BinaryTree256::add(matchables, trainMode);
    }

    // std::string addMultiple(int n_times) override {
    //     PYBIND11_OVERRIDE_PURE(
    //         std::string, /* Return type */
    //         Animal,      /* Parent class */
    //         go,          /* Name of function in C++ (must match Python name) */
    //         n_times      /* Argument(s) */
    //     );
    // }

private:
    static BinaryTree256::MatchableVector buildMatchableVector(std::vector<uint64_t>& imageIds, std::vector<uint64_t>& descriptors) {
        BinaryTree256::MatchableVector matchables(imageIds.size());

        for (uint64_t i = 0; i < imageIds.size(); ++i) {
            matchables[i] = new BinaryTree256::Matchable(imageIds[i], descriptors[i]);
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
        .def(py::init(&BinaryTree256::create))
        .def("add", &BinaryTree256::add)
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