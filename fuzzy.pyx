from libc.stdlib cimport malloc
from libc.stdlib cimport free
from pyhashxx import hashxx
import cython
#cython: boundscheck=False
#cython: wraparound=False

cdef class DawgNode:
    cdef public int id
    cdef public bint final
    cdef public DawgNode parent
    cdef public unicode letter
    cdef public dict edges

    def __init__(self, int id, DawgNode parent, unicode letter):
        self.id = id
        self.final = False
        self.parent = parent
        self.letter = letter
        self.edges = {}

    def __hash__(self):
        arr = [self.final]
        for (label, node) in self.edges.iteritems():
            arr.append(label)
            arr.append(str(node.id))
        return hashxx(bytes(arr))

    def __richcmp__(x, y, op):
        if op == 2:
            if x.final != y.final or len(x.edges) != len(y.edges):
                return False
            for (label_x, node_x), (label_y, node_y) in zip(x.edges.iteritems(), y.edges.iteritems()):
                if label_x != label_y or node_x.id != node_y.id:
                    return False
            return True

cdef class Dawg:
    cdef unicode prev_word
    cdef int id
    cdef public DawgNode root
    cdef list unchecked_nodes
    cdef dict minimized_nodes

    def __init__(self):
        self.prev_word = u""
        self.root = DawgNode(0, None, u"")
        self.id = 1
        self.unchecked_nodes = []
        self.minimized_nodes = {}

    def insert(self, unicode word):
        cdef int prefix_length = 0
        cdef int i
        cdef unicode letter
        for i in range(min(len(word), len(self.prev_word))):
            if word[i] != self.prev_word[i]: break
            prefix_length += 1
        self._minimize(prefix_length)
        if len(self.unchecked_nodes) == 0:
            node = self.root
        else:
            node = self.unchecked_nodes[-1][2]
        for letter in word[prefix_length:]:
            next_node = DawgNode(self.id, node, letter)
            self.id += 1
            node.edges[letter] = next_node
            self.unchecked_nodes.append((node, letter, next_node))
            node = next_node
        node.final = True
        self.prev_word = word

    def finish(self):
        self._minimize(0)

    def _minimize(self, int level):
        cdef int i
        cdef unicode letter
        for i in range(len(self.unchecked_nodes) - 1, level - 1, -1):
            (parent, letter, child) = self.unchecked_nodes[i]
            if child in self.minimized_nodes:
                parent.edges[letter] = self.minimized_nodes[child]
                self.minimized_nodes[child].parent=parent
            else:
                self.minimized_nodes[child] = child
            self.unchecked_nodes.pop()

    def contains(self, word):
        node = self.root
        for letter in word:
            if letter not in node.edges:
                return False
            node = node.edges[letter]
        return node.final

    def node_count(self):
        return len(self.minimized_nodes)

    def edge_count(self):
        count = 0
        for node in self.minimized_nodes:
            count += len(node.edges)
        return count

    def fuzzy_search(self, unicode word, int fuzziness):
        cdef word_length = len(word)
        cdef int *current_row = <int*> malloc((word_length + 1) * sizeof(int))
        cdef int i
        cdef unicode letter
        try:
            for i in range(word_length + 1):
                current_row[i] = i
            results = []
            for letter in self.root.edges.keys():
                self._search_recursive(self.root.edges[letter], letter, word, current_row, results, fuzziness,
                                       letter)
            return results
        finally:
            free(current_row)

    cdef _search_recursive(self, node, unicode letter, unicode word, int*previous_row, list results, int fuzziness,
                           unicode path):
        cdef int columns = len(word) + 1
        cdef int column, insert_cost, delete_cost, replace_cost
        cdef int minimum = -1
        cdef int *current_row = <int*> malloc((columns) * sizeof(int))
        try:
            current_row[0] = previous_row[0] + 1
            for column in xrange(1, columns):
                insert_cost = current_row[column - 1] + 1
                delete_cost = previous_row[column] + 1
                if word[column - 1] != letter:
                    replace_cost = previous_row[column - 1] + 1
                else:
                    replace_cost = previous_row[column - 1]
                current_row[column] = min(insert_cost, delete_cost, replace_cost)
                if minimum == -1 or minimum > current_row[column]:
                    minimum = current_row[column]
            if current_row[columns - 1] <= fuzziness and node.final:
                results.append((path.decode("utf-8"), current_row[columns - 1]))
            if minimum <= fuzziness:
                for letter in node.edges.keys():
                    self._search_recursive(node.edges[letter], letter, word, current_row, results, fuzziness,
                                           path + str(letter))
        finally:
            free(current_row)

    #TODO: save, load model
    #TODO: optimize for speed