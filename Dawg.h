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
#include <queue>

typedef char chartype;
typedef std::string stringtype;

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

    std::vector<stringtype> fuzzy_search(stringtype &word, int fuzziness);

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

    void fuzzy_search_recursive(DawgNode *node, stringtype &word, std::vector<stringtype> &results, int **rows,
                                int fuzziness, chartype *path, int depth);

};


#endif //FUZZY_DAWG_H
