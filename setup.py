__author__ = 'vesely'
from distutils.core import setup, Extension
from Cython.Build import cythonize

setup(ext_modules=cythonize(Extension(
    "pydawg",  # the extesion name
    sources=["pydawg.pyx", "Dawg.cpp"],  # the Cython source and
    extra_compile_args=['-Ofast', '-march=native', '-std=c++11'],  # additional C++ source files
    language="c++",  # generate and compile C++ code
)))
