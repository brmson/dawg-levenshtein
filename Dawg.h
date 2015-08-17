//
// Created by vesely on 8/12/15.
//

#ifndef FUZZY_DAWG_H
#define FUZZY_DAWG_H

#include <cstring>
#include <vector>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include "utils.h"
#include <climits>
#include <cstdint>
#include <iostream>
#include <fstream>
#include <map>
#include <algorithm>
#include <queue>

typedef char chartype;
typedef std::string stringtype;

enum OpType {
    DELETE = 1, REPLACE = 2, INSERT = 4, NOTHING = 0
};

class EditOperation {
    public:
    EditOperation(const chartype &from, const chartype &to, size_t pos, const OpType &op) : m_from(from), m_to(to),
                                                                                              m_pos(pos), m_op(op) { }


    chartype getFrom() const {
        return m_from;
    }

    chartype getTo() const {
        return m_to;
    }

    size_t getPos() const {
        return m_pos;
    }

    const OpType &getOp() const {
        return m_op;
    }

    friend std::ostream &operator<<(std::ostream &, const EditOperation &);
    private:

    chartype m_from;
    chartype m_to;
    size_t m_pos;
    OpType m_op;
};

class WordResult {
    public:
    WordResult(int editDistance, const std::vector<EditOperation*> &operations, const stringtype &word) : m_edit_distance(
            editDistance), m_operations(operations), m_word(word) { }

    stringtype getWord(){
        return m_word;
    }


    int getEditDistance() const {
        return m_edit_distance;
    }

    const std::vector<EditOperation*> &getEditOperations() const {
        return m_operations;
    }

private:
    int m_edit_distance;
    std::vector<EditOperation*> m_operations;
    stringtype m_word;
};

class DawgNode {

public:
    DawgNode(int id, chartype letter, DawgNode *parent) : m_id(id), m_final(false), m_letter(letter),
                                                          m_parent(parent) { }

    DawgNode() : DawgNode(-1, -1, nullptr) { }

    inline bool isFinal() const {
        return m_final;
    }

    inline int getId() const {
        return m_id;
    }

    inline chartype getLetter() const {
        return m_letter;
    }

    void setFinal(bool final) {
        m_final = final;
    }

    void setParent(DawgNode *parent) {
        DawgNode::m_parent = parent;
    }

    inline const std::vector<DawgNode *> &getChilds() const {
        return m_childs;
    }

    void insertChild(DawgNode *node);

    DawgNode *getParent() const {
        return m_parent;
    }

    void toBytes(std::vector<uint8_t> &data);

    bool operator==(const DawgNode &other) const;

    static void fromBytes(std::unordered_map<int, DawgNode *> &id_to_node, std::deque<uint8_t> &bytes);

private:
    int m_id;
    bool m_final;
    chartype m_letter;
    DawgNode *m_parent;
    std::vector<DawgNode *> m_childs;
};

namespace std {
    template<>
    struct hash<DawgNode *> {
        typedef DawgNode *argument_type;
        typedef std::size_t result_type;

        result_type operator()(const argument_type &t) const {
            size_t seed = 0;
            hash_combine(seed, t->isFinal());
            hash_combine(seed, t->getLetter());
            for (auto node: t->getChilds()) {
                hash_combine(seed, node->getId());
            }
            return seed;
        }
    };

    template<>
    struct equal_to<DawgNode *> {
        typedef DawgNode *argument_type;
        typedef std::size_t result_type;

        result_type operator()(const argument_type x, const argument_type y) const {
            return (unsigned long) ((*x) == (*y));
        }
    };
}

class Dawg {
public:

    Dawg() : m_node_id(0), m_root() { }

    void insert(stringtype &word);

    void finish();

    bool contains(stringtype &word);

    std::vector<WordResult*> fuzzy_search(stringtype &word, int fuzziness);

    void load(const std::string &filename);

    void save(const std::string &filename);

    void toDot(const std::string &filename);

    virtual ~Dawg();

private:
    stringtype m_prev_word;
    int m_node_id;
    DawgNode m_root;
    std::vector<DawgNode *> m_unchecked_nodes;
    std::unordered_set<DawgNode *> m_minimized_nodes;

    void compress_graph(size_t level);

    void fuzzy_search_recursive(DawgNode *node, stringtype &word, std::vector<WordResult*>& results, int **rows,
                                char **path_rows, int fuzziness, chartype *path, int depth);

    std::vector<EditOperation*> get_segmented(stringtype &word, chartype *similar_word, char **paths, int depth,
                                                    int c);
};


#endif //FUZZY_DAWG_H
