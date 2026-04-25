#pragma once

#include "BTree.hpp"
#include "Location.hpp"
#include <memory>

class IndexBase {
public:
    virtual ~IndexBase() = default;
};

template<typename KeyType>
class Index : public IndexBase {
private:
    BTree<KeyType> tree;
    
public:
    Index(int minDegree = 3) : tree(minDegree) {}
    
    void insert(const KeyType& key, const Location& loc) {
        tree.insert(key, loc);
    }
    
    Location search(const KeyType& key) {
        return tree.search(key);
    }
};