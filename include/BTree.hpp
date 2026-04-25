#pragma once

#include <vector>
#include <utility>
#include <algorithm>
#include <iostream>
#include "Location.hpp"

template<typename KeyType>
class BTree;

template<typename KeyType>
struct BTreeNode {
    std::vector<KeyType> keys;
    std::vector<BTreeNode<KeyType>*> children;
    std::vector<Location> values;
    bool isLeaf;
    
    BTreeNode(bool leaf = true) : isLeaf(leaf) {}
};

template<typename KeyType>
class BTree {
private:
    BTreeNode<KeyType>* root;
    int t;
    
    void splitChild(BTreeNode<KeyType>* parent, int index, BTreeNode<KeyType>* child) {
        BTreeNode<KeyType>* newChild = new BTreeNode<KeyType>(child->isLeaf);
        int midIndex = t - 1;
        
        for (int i = midIndex + 1; i < child->keys.size(); i++) {
            newChild->keys.push_back(child->keys[i]);
            newChild->values.push_back(child->values[i]);
        }
        
        if (!child->isLeaf) {
            for (int i = midIndex + 1; i < child->children.size(); i++) {
                newChild->children.push_back(child->children[i]);
            }
        }
        
        child->keys.resize(midIndex);
        child->values.resize(midIndex);
        if (!child->isLeaf) {
            child->children.resize(midIndex + 1);
        }
        
        KeyType midKey = child->keys[midIndex];
        Location midValue = child->values[midIndex];
        
        parent->keys.insert(parent->keys.begin() + index, midKey);
        parent->values.insert(parent->values.begin() + index, midValue);
        parent->children.insert(parent->children.begin() + index + 1, newChild);
    }
    
    void insertNonFull(BTreeNode<KeyType>* node, const KeyType& key, const Location& value) {
        int i = node->keys.size() - 1;
        
        if (node->isLeaf) {
            node->keys.push_back(key);
            node->values.push_back(value);
            
            while (i >= 0 && node->keys[i] > key) {
                std::swap(node->keys[i], node->keys[i + 1]);
                std::swap(node->values[i], node->values[i + 1]);
                i--;
            }
        } else {
            while (i >= 0 && key < node->keys[i]) {
                i--;
            }
            i++;
            
            if (node->children[i]->keys.size() == 2 * t - 1) {
                splitChild(node, i, node->children[i]);
                if (key > node->keys[i]) {
                    i++;
                }
            }
            insertNonFull(node->children[i], key, value);
        }
    }
    
    Location searchNode(BTreeNode<KeyType>* node, const KeyType& key) {
        if (!node) return Location();
        
        int i = 0;
        while (i < node->keys.size() && key > node->keys[i]) {
            i++;
        }
        
        if (i < node->keys.size() && key == node->keys[i]) {
            return node->values[i];
        }
        
        if (node->isLeaf) {
            return Location();
        }
        
        return searchNode(node->children[i], key);
    }
    
public:
    BTree(int minDegree = 3) : root(nullptr), t(minDegree) {
        if (t < 2) t = 2;
    }
    
    ~BTree() {
        // TODO: recursive delete – for later
    }
    
    void insert(const KeyType& key, const Location& value) {
        if (!root) {
            root = new BTreeNode<KeyType>(true);
            root->keys.push_back(key);
            root->values.push_back(value);
            return;
        }
        
        if (root->keys.size() == 2 * t - 1) {
            BTreeNode<KeyType>* newRoot = new BTreeNode<KeyType>(false);
            newRoot->children.push_back(root);
            splitChild(newRoot, 0, root);
            root = newRoot;
        }
        
        insertNonFull(root, key, value);
    }
    
    Location search(const KeyType& key) {
        return searchNode(root, key);
    }
    
    void print() {
        std::cout << "B-Tree (t=" << t << ") - Root exists: " << (root != nullptr) << std::endl;
        if (root && !root->keys.empty()) {
            std::cout << "Root keys: ";
            for (const auto& k : root->keys) {
                std::cout << k << " ";
            }
            std::cout << std::endl;
        }
    }
};