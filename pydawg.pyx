#distutils: language = c++
#distutils: sources = Dawg.cpp
from libcpp.vector cimport vector
from libcpp.string cimport string

__author__ = 'vesely'
cdef extern from "Dawg.h":
    cdef cppclass Dawg:
        Dawg() except +
        void insert(string);
        void finish();
        bint contains(string);
        vector[string] fuzzy_search(string, int);
        void load(string);
        void save(string);

cdef class PyDawg:
    cdef Dawg *thisptr  # hold a C++ instance which we're wrapping
    def __cinit__(self):
        self.thisptr = new Dawg()
    def __dealloc__(self):
        del self.thisptr
    def insert(self, unicode word):
        cdef string w = word.decode("UTF-8")
        self.thisptr.insert(w)
    def finish(self):
        self.thisptr.finish()
    cdef bint contains(self, unicode word):
        cdef string w = word.decode("UTF-8")
        cdef bint result = self.thisptr.contains(w)
        return result
    def fuzzy_search(self, unicode word, int fuzziness):
        cdef string w = word.decode("UTF-8")
        cdef vector[string] result = self.thisptr.fuzzy_search(w, fuzziness)
        return result
    def load(self, unicode filename):
        cdef string f = filename.decode("UTF-8")
        self.thisptr.load(f)
    def save(self, unicode filename):
        cdef string f = filename.decode("UTF-8")
        self.thisptr.save(f)
