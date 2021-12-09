#include "pybind11/pybind11.h"
#include "pybind11/stl.h"

#define STRINGIFY(x) #x
#define MACRO_STRINGIFY(x) STRINGIFY(x)

namespace py = pybind11;


PYBIND11_MODULE(hbst, m) {
    py::module_ m_tree = m.def_submodule("tree");

    m.attr("__version__") = "0.0.1";
}