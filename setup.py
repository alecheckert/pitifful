"""Compile pitifful Python bindings"""
from setuptools import setup
from pybind11.setup_helpers import Pybind11Extension, build_ext

ext_modules = [
    Pybind11Extension(
        "_pitifful",
        ["src/module.cpp"],
        include_dirs=["include"],
        libraries=["z"],
        cxx_std=14,
    ),
]

setup(
    ext_modules=ext_modules,
    cmdclass={"build_ext": build_ext},
)
