#include <iostream>
#include <string>
#include "pitifful.h"
#include <pybind11/pybind11.h>
#include <pybind11/numpy.h>

namespace py = pybind11;

PYBIND11_MODULE(_pitifful, m)
{
    py::class_<pitifful::IFD>(m, "IFD", py::module_local())
        .def_property_readonly(
            "byte_offset", 
            [](const pitifful::IFD& ifd){return ifd.byte_offset;}
        )
        .def_property_readonly(
            "next_byte_offset", 
            [](const pitifful::IFD& ifd){return ifd.next_byte_offset;}
        )
        .def_property_readonly(
            "count", 
            [](const pitifful::IFD& ifd){return ifd.count;}
        )
        .def_property_readonly(
            "width",
            [](const pitifful::IFD& ifd){return ifd.width;}
        )
        .def_property_readonly(
            "height",
            [](const pitifful::IFD& ifd){return ifd.height;}
        )
        .def_property_readonly(
            "bits_per_sample",
            [](const pitifful::IFD& ifd){return ifd.bits_per_sample;}
        )
        .def_property_readonly(
            "compression",
            [](const pitifful::IFD& ifd){return ifd.compression;}
        )
        .def_property_readonly(
            "photometric_interpretation",
            [](const pitifful::IFD& ifd){return ifd.photometric_interpretation;}
        )
        .def_property_readonly(
            "samples_per_pixel",
            [](const pitifful::IFD& ifd){return ifd.samples_per_pixel;}
        )
        .def_property_readonly(
            "rows_per_strip",
            [](const pitifful::IFD& ifd){return ifd.rows_per_strip;}
        );

    py::class_<pitifful::TIFFReader>(m, "TIFFReader", py::module_local())
        .def(py::init<const char*>())
        .def_property_readonly(
            "n_frames",
            &pitifful::TIFFReader::get_n_frames
        )
        .def_property_readonly(
            "max_strip_size",
            &pitifful::TIFFReader::max_strip_size
        )
        .def("get_ifd", &pitifful::TIFFReader::get_ifd)
        .def("get_n_samples", &pitifful::TIFFReader::get_n_samples);
}
