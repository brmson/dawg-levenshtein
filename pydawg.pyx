#distutils: language = c++
#distutils: sources = Dawg.cpp
from libcpp.vector cimport vector
from libcpp.string cimport string

__author__ = 'vesely'
cdef extern from "Dawg.h":
    cdef enum OpType:
        DELETE = 1, REPLACE = 2, INSERT = 4, NOTHING = 0

    cdef cppclass EditOperation:
        EditOperation(int, int, int, OpType) except +
        char getFrom()
        char getTo()
        int getPos()
        OpType getOp()

    cdef cppclass WordResult:
        WordResult(int, vector[EditOperation], string)  except +
        string getWord();
        int getEditDistance();
        vector[EditOperation] getEditOperations();

    cdef cppclass Dawg:
        Dawg() except +
        void insert(string);
        void finish();
        bint contains(string);
        vector[WordResult] fuzzy_search(string, int);
        void load(string);
        void save(string);

class PyWordResult:
    def __init__(self, word, edit_distance, edit_operations):
        self.word = word
        self.edit_distance = edit_distance
        self.edit_operations = edit_operations


class PyEditOperation:

    def __init__(self, e_from, e_to, position, operation):
        self.e_from = e_from
        self.e_to = e_to
        self.position = position
        self.operation = operation

    def __str__(self):
        op = 'Insert'
        if self.operation == DELETE:
            op = 'Delete'
        if self.operation == REPLACE:
            op = 'Replace'
        return "%s : %s -> %s, pos=%d" % (op, self.e_from, self.e_to, self.position)

    def __repr__(self):
        return self.__str__()

cdef to_unicode(char str):
    return (<bytes> str).decode("UTF-8") if str else ""

cdef class PyDawg:
    cdef Dawg *thisptr
    def __cinit__(self):
        self.thisptr = new Dawg()
    def __dealloc__(self):
        del self.thisptr
    def insert(self, unicode word):
        cdef string w = word.decode("UTF-8")
        self.thisptr.insert(w)
    def finish(self):
        self.thisptr.finish()
    def contains(self, unicode word):
        cdef string w = word.decode("UTF-8")
        cdef bint result = self.thisptr.contains(w)
        return result
    def fuzzy_search(self, unicode word, int fuzziness):
        cdef string w = word.decode("UTF-8")
        cdef vector[WordResult] results = self.thisptr.fuzzy_search(w, fuzziness)
        pyres = []
        for i in range(results.size()):
            ops = results[i].getEditOperations()
            operations = [
                PyEditOperation(
                    to_unicode(ops[j].getFrom()),
                    to_unicode(ops[j].getTo()),
                    ops[j].getPos(),
                    ops[j].getOp()) for j in range(ops.size())]
            res = PyWordResult(results[i].getWord(), results[i].getEditDistance(), operations)
            pyres.append(res)
        return pyres
    def load(self, unicode filename):
        cdef string f = filename.decode("UTF-8")
        self.thisptr.load(f)
    def save(self, unicode filename):
        cdef string f = filename.decode("UTF-8")
        self.thisptr.save(f)
