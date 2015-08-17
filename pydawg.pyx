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
        WordResult(int, vector[EditOperation*], string)  except +
        string getWord();
        int getEditDistance();
        vector[EditOperation*] getEditOperations();

    cdef cppclass Dawg:
        Dawg() except +
        void insert(string);
        void finish();
        bint contains(string);
        vector[WordResult*] fuzzy_search(string, int);
        void load(string);
        void save(string);

cdef class PyWordResult:
    cdef WordResult *thisptr
    def __cinit__(self):
        pass
    def __dealloc__(self):
        del self.thisptr

    property word:
        def __get__(self): return self.thisptr.getWord()
    property edit_distance:
        def __get__(self): return self.thisptr.getEditDistance()
    property edit_operations:
        def __get__(self):
            ops = self.thisptr.getEditOperations()
            operations = []
            for i in range(ops.size()):
                op = PyEditOperation()
                op.thisptr = ops[i]
                operations.append(op)
            return operations

cdef class PyEditOperation:
    cdef EditOperation *thisptr
    def __cinit__(self):
        pass
    def __dealloc__(self):
        del self.thisptr

    property e_from:
        def __get__(self): return (<bytes> self.thisptr.getFrom()).decode("UTF-8") if self.thisptr.getFrom() else ""

    property e_to:
        def __get__(self): return (<bytes> self.thisptr.getTo()).decode("UTF-8") if self.thisptr.getTo() else ""

    property position:
        def __get__(self): return self.thisptr.getPos()

    property operation:
        def __get__(self): return self.thisptr.getOp()

    def __str__(self):
        op='Insert'
        if self.operation==DELETE:
            op='Delete'
        if self.operation==REPLACE:
            op='Replace'
        return "%s : %s -> %s, pos=%d" % (op, self.e_from, self.e_to, self.position)

    def __repr__(self):
        return self.__str__()

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
        cdef vector[WordResult*] results = self.thisptr.fuzzy_search(w, fuzziness)
        pyres = []
        for i in range(results.size()):
            res = PyWordResult()
            res.thisptr = results[i]
            pyres.append(res)
        return pyres
    def load(self, unicode filename):
        cdef string f = filename.decode("UTF-8")
        self.thisptr.load(f)
    def save(self, unicode filename):
        cdef string f = filename.decode("UTF-8")
        self.thisptr.save(f)
