//
// Created by vesely on 8/12/15.
//
#include "Dawg.h"


void Dawg::compress_graph(size_t level) {
    for (size_t i = m_unchecked_nodes.size(); i > level; i--) {
        DawgNode *node = m_unchecked_nodes[i - 1];
        const std::unordered_set<DawgNode *>::iterator &it = m_minimized_nodes.find(node);
        m_unchecked_nodes.pop_back();
        if (it == m_minimized_nodes.end()) {
            m_minimized_nodes.insert(node);
        }
        else {
            node->getParent()->insertChild(*it);
            delete node;
        }
    }
}


void Dawg::insert(stringtype &word) {
    size_t prefix_length = 0;
    size_t w_len = std::min(word.size(), m_prev_word.size());
    for (size_t i = 0; i < w_len; i++) {
        if (word[i] != m_prev_word[i])break;
        prefix_length++;
    }
    compress_graph(prefix_length);
    DawgNode *node = m_unchecked_nodes.empty() ? &m_root : m_unchecked_nodes.back();
    for (size_t i = prefix_length; i < word.size(); i++) {
        chartype letter = word[i];
        DawgNode *next_node = new DawgNode(m_node_id++, letter, node);
        node->insertChild(next_node);
        m_unchecked_nodes.push_back(next_node);
        node = next_node;
    }
    node->setFinal(true);
    m_prev_word = word;
}

void Dawg::finish() {
    compress_graph(0);
}

std::vector<stringtype> Dawg::fuzzy_search(stringtype &word, int fuzziness) {
    const size_t word_length = word.size();
    const size_t buffer_size = word_length + 1;
    const size_t buffer_size_f = buffer_size + fuzziness + 1;
    int **rows = new int *[buffer_size_f];
    for (int i = 0; i < buffer_size_f; i++)rows[i] = new int[buffer_size];
    int *row = rows[0];
    chartype path[word_length + fuzziness];
    std::memset(path, 0, sizeof path);
    for (int i = 0; i < buffer_size; i++)row[i] = i;
    std::vector<stringtype> results;
    for (auto it:m_root.getChilds()) {
        path[0] = it->getLetter();
        fuzzy_search_recursive(it, word, results, rows, fuzziness, path, 1);
    }
    for (int i = 0; i < buffer_size_f; i++) delete rows[i];
    delete rows;
    return results;
}


void Dawg::fuzzy_search_recursive(DawgNode *node, stringtype &word, std::vector<stringtype> &results, int **rows,
                                  int fuzziness, chartype *path, int depth) {
    size_t word_length = word.size();
    size_t buffer_size = word_length + 1;

    int *row = rows[depth];
    int *prev_row = rows[depth - 1];
    row[0] = prev_row[0] + 1;
    int minimum = INT32_MAX;
    for (int i = 1; i < buffer_size; i++) {
        int insert_cost = row[i - 1] + 1;
        int delete_cost = prev_row[i] + 1;
        int replace_cost = prev_row[i - 1];
        if (word[i - 1] != node->getLetter()) {
            replace_cost++;
        }
        row[i] = min(insert_cost, delete_cost, replace_cost);
        if (minimum > row[i]) {
            minimum = row[i];
        }
    }
    if (row[word_length] <= fuzziness && node->isFinal()) {
        results.push_back(stringtype(path));
    }
    if (minimum <= fuzziness) {
        size_t child_count = node->getChilds().size();
        const std::vector<DawgNode *> &childs = node->getChilds();
        for (size_t i = 0; i < child_count; i++) {
            path[depth] = childs[i]->getLetter();
            fuzzy_search_recursive(childs[i], word, results, rows, fuzziness, path, depth + 1);
        }
    }
    path[depth] = 0;
}

bool Dawg::contains(stringtype &word) {
    DawgNode *node = &m_root;
    for (size_t i = 0; i < word.size(); i++) {
        const std::vector<DawgNode *> vector = node->getChilds();
        size_t j = 0;
        for (; j < vector.size(); j++) {
            if (vector[j]->getLetter() == word[i]) {
                break;
            }
        }
        if (j == vector.size()) {
            return false;
        }
        node = vector[j];
    }
    return node->isFinal();
}

void DawgNode::insertChild(DawgNode *node) {
    chartype letter = node->getLetter();
    for (size_t i = 0; i < m_childs.size(); i++) {
        if (m_childs[i]->getLetter() == letter) {
            m_childs[i] = node;
            return;
        }
    }
    m_childs.push_back(node);
}

bool DawgNode::operator==(const DawgNode &y) const {
    if (isFinal() != y.isFinal() || getLetter() != y.getLetter()) {
        return false;
    }
    const std::vector<DawgNode *> &childs_x = getChilds();
    const std::vector<DawgNode *> &childs_y = y.getChilds();
    if (childs_x.size() != childs_y.size()) {
        return false;
    }
    size_t count = childs_x.size();
    for (size_t i = 0; i < count; i++) {
        if (childs_x[i]->getId() != childs_y[i]->getId()) {
            return false;
        }
    }
    return true;
}

void Dawg::save(const std::string &filename) {
    std::vector<uint8_t> data;
    m_root.toBytes(data);
    for (auto item:m_minimized_nodes) {
        item->toBytes(data);
    }
//    std::vector<uint8_t> compressed_data(data.size());
//    int compressed_size = LZ4_compressHC((char *) &data[0], (char *) &compressed_data[0], (int) data.size());
    std::ofstream file(filename, std::ios::binary);
//    file.write((char *) &compressed_data[0], compressed_size);
    file.write((char *) &data[0], data.size());
    file.close();
}

void Dawg::load(const std::string &filename) {
    std::ifstream ifs(filename, std::ios::binary);
    ifs.seekg(0, std::ios::end);
    std::ifstream::pos_type filesize = ifs.tellg();
    ifs.seekg(0, std::ios::beg);
    std::vector<char> bytes((unsigned long) filesize);
    ifs.read(&bytes[0], filesize);
    std::deque<uint8_t> bytes_q(bytes.begin(), bytes.end());
    std::unordered_map<int, DawgNode *> nodes;
    nodes[m_root.getId()] = &m_root;
    while (!bytes_q.empty()) {
        DawgNode::fromBytes(nodes, bytes_q);
    }
    nodes.erase(m_root.getId());
    for (auto item:nodes) {
        m_minimized_nodes.insert(item.second);
    }
}

void DawgNode::toBytes(std::vector<uint8_t> &data) {
    typeToBytes(getId(), data);
    typeToBytes(getLetter(), data);
    uint8_t flag = (uint8_t) (isFinal() ? 1 : 0);
    if (m_childs.empty()) {
        flag += 2;
    }
    typeToBytes(flag, data);
    size_t i = 0;
    for (auto node:m_childs) {
        typeToBytes(node->getId(), data);
        flag = 0;
        if (i == m_childs.size() - 1) {
            flag += 2;
        }
        typeToBytes(flag, data);
        i++;
    }
}


void DawgNode::fromBytes(std::unordered_map<int, DawgNode *> &id_to_node, std::deque<uint8_t> &bytes) {
    int id = typeFromBytes<int>(bytes);
    auto it = id_to_node.find(id);
    DawgNode *node;
    if (it == id_to_node.end()) {
        node = new DawgNode();
        id_to_node[id] = node;
    } else {
        node = it->second;
    }
    chartype letter = typeFromBytes<chartype>(bytes);
    uint8_t flag = typeFromBytes<uint8_t>(bytes);
    bool final = (flag & 1) != 0;
    bool last = (flag & 2) != 0;
    node->m_id = id;
    node->m_letter = letter;
    node->m_final = final;
    while (!last) {
        int child_id = typeFromBytes<int>(bytes);
        it = id_to_node.find(child_id);
        DawgNode *child_node;
        if (it == id_to_node.end()) {
            child_node = new DawgNode();
            id_to_node[child_id] = child_node;
        } else {
            child_node = it->second;
        }
        node->m_childs.push_back(child_node);
        uint8_t child_flag = typeFromBytes<uint8_t>(bytes);
        last = (child_flag & 2) != 0;
    }
}

void Dawg::toDot(const std::string &filename) {
    std::ofstream file(filename);
    file << "digraph structs {" << std::endl;
    for (auto node:m_minimized_nodes) {
        char l[1];
        wctomb(l, node->getLetter());
        file << node->getId() << " [label=" << node->getLetter() << "];" << std::endl;
    }
    for (auto node:m_minimized_nodes) {
        for (auto c: node->getChilds()) {
            file << node->getId() << " -> " << c->getId() << ";" << std::endl;
        }
    }
    file << "}";
}

Dawg::~Dawg() {
    for (auto node:m_minimized_nodes) {
        delete node;
    }
    for (auto node:m_unchecked_nodes) {
        delete node;
    }
}

