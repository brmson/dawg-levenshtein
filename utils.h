/*
 * utils.h
 *
 *  Created on: Aug 12, 2015
 *      Author: vesely
 */

#ifndef UTILS_H_
#define UTILS_H_

#include <vector>
#include <queue>

template<class T>
inline void hash_combine(std::size_t &seed, const T &v) {
    std::hash<T> hasher;
    seed ^= hasher(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
}

template<typename T>
void typeToBytes(T value, std::vector<uint8_t> &byte_array) {
    for (size_t i = 0; i < sizeof(T); i++) {
        byte_array.push_back((value >> (8 * i)));
    }
}

template<typename T>
T typeFromBytes(std::deque<uint8_t> &byte_array) {
    T value = 0;
    for (size_t i = 0; i < sizeof(T); i++) {
        value += byte_array.front() << (8 * i);
        byte_array.pop_front();
    }
    return value;
}

static inline int
min(int a, int b, int c) {
    int m = a;
    if (m > b) m = b;
    if (m > c) m = c;
    return m;
}

template<typename T>
T** newArray(const size_t n1, const size_t n2){
    T** ar=new T*[n1];
    for(size_t i=0;i<n1;i++){
        ar[i]=new T[n2];
    }
    return ar;
}

template<typename T>
void deleteArray(T** ar, const size_t n1){
    for(size_t i=0;i<n1;i++){
        delete[] ar[i];
    }
     delete[] ar;
}

template<typename T, typename C>
void append(std::vector<T>& results, const std::vector<T>& items, const C& item){
    for(auto it:items){
        results.push_back(it+item);
    }
}

#endif /* UTILS_H_ */
