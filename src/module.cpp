/* Python bindings for pitifful */
#include <iostream>
#include <string>
#include "pitifful.h"
#include <pybind11/pybind11.h>
#include <pybind11/numpy.h>

namespace py = pybind11;

py::array_t<uint8_t> read_frame_8bit(pitifful::TIFFReader& reader, int frame)
{
    const pitifful::IFD& ifd = reader.get_ifd(frame);
    const int height = ifd.height;
    const int width = ifd.width;
    const int samples_per_pixel = ifd.samples_per_pixel;
    py::array_t<uint8_t> out(height*width*samples_per_pixel);
    uint8_t* out_ptr = static_cast<uint8_t*>(out.request().ptr);
    reader.read_frame<uint8_t>(frame, out_ptr);
    if(samples_per_pixel>1){
        out.resize({height, width, samples_per_pixel});
    } else{
        out.resize({height, width});
    }
    return out;
}

py::array_t<uint16_t> read_frame_16bit(pitifful::TIFFReader& reader, int frame)
{
    const pitifful::IFD& ifd = reader.get_ifd(frame);
    const int height = ifd.height;
    const int width = ifd.width;
    const int samples_per_pixel = ifd.samples_per_pixel;
    py::array_t<uint16_t> out(height*width*samples_per_pixel);
    uint16_t* out_ptr = static_cast<uint16_t*>(out.request().ptr);
    reader.read_frame<uint16_t>(frame, out_ptr);
    if(samples_per_pixel>1){
        out.resize({height, width, samples_per_pixel});
    } else{
        out.resize({height, width});
    }
    return out;
}

py::array_t<uint16_t> read_stack_16bit(pitifful::TIFFReader& reader)
{
    const int n_frames = static_cast<int>(reader.get_n_frames());
    const pitifful::IFD& ifd0 = reader.get_ifd(0);
    const int height = ifd0.height;
    const int width = ifd0.width;
    const int samples_per_pixel = ifd0.samples_per_pixel;
    const int bits_per_sample = ifd0.bits_per_sample;
    const int size = n_frames * height * width * samples_per_pixel;
    py::array_t<uint16_t> out(size);
    uint16_t* out_ptr = static_cast<uint16_t*>(out.request().ptr);
    for(uint16_t frame=0; frame<n_frames; ++frame){
        const pitifful::IFD& ifd = reader.get_ifd(frame);
        if(
            (ifd.height!=height)
            || (ifd.width!=width)
            || (ifd.samples_per_pixel!=samples_per_pixel)
            || (ifd.bits_per_sample!=bits_per_sample)
        ){
            throw std::runtime_error(
                "read_stack_16bit only compatible with homogeneous " \
                "image sizes"
            );
        }
        reader.read_frame<uint16_t>(
            frame,
            out_ptr + frame * height * width * samples_per_pixel
        );
    }
    if(samples_per_pixel==1){
        out.resize({n_frames, height, width});
    } else{
        out.resize({n_frames, height, width, samples_per_pixel});
    }
    return out;
}

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
        )
        .def(
            "summary",
            [](const pitifful::IFD& ifd){
                std::cout << "height:\t" << ifd.height << "\n";
                std::cout << "width:\t" << ifd.width << "\n";
                std::cout << "samples_per_pixel:\t" << ifd.samples_per_pixel << "\n";
                std::cout << "bits_per_sample:\t" << ifd.bits_per_sample << "\n";
                std::cout << "compression:\t" << ifd.compression << "\n";
                std::cout << "photometric_interpretation:\t" << ifd.photometric_interpretation << "\n";
                std::cout << "rows_per_strip:\t" << ifd.rows_per_strip << "\n";
                std::cout << "byte_offset:\t" << ifd.byte_offset << "\n";
                std::cout << "next_byte_offset:\t" << ifd.next_byte_offset << "\n";
                std::cout << "count:\t" << ifd.count << "\n";
            }
        );

    py::class_<pitifful::TIFFReader>(m, "TIFFReader", py::module_local())
        .def(py::init<const char*>())
        .def_property_readonly(
            "n_frames",
            &pitifful::TIFFReader::get_n_frames
        )
        .def_property_readonly(
            "max_strip_size",
            &pitifful::TIFFReader::get_max_strip_size
        )
        .def("get_ifd", &pitifful::TIFFReader::get_ifd)
        .def("get_n_samples", &pitifful::TIFFReader::get_n_samples)
        .def("read_frame_8bit", &read_frame_8bit)
        .def("read_frame_16bit", &read_frame_16bit)
        .def("read_stack_16bit", &read_stack_16bit);
}
