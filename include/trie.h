#pragma once

#include <iostream>
#include <memory>
#include <string>
#include <vector>

#include "util/coding.h"

namespace stbe {

constexpr int kMinCommPrefix = 2;
constexpr float kAvgVarintSize = 3.0;

class TrieNode;

// TriePosition represent an handle of string after added to a Trie.
class TriePosition {
  friend class Trie;
private:
  TrieNode* trie_node_;
  // private constructor for Trie to make instance.
  explicit TriePosition(TrieNode* trie_node) : trie_node_(trie_node) {}

public:
  TriePosition() : trie_node_(nullptr) {}
  size_t getPosition() const;
};

class TrieNode {
private:
  std::string value_;
  std::vector<std::unique_ptr<TrieNode>> children_;
  size_t position_ = 0;
public:
  TrieNode(const std::string& value = "") : value_(value) {}

  std::string& getVal() {
    return value_;
  }
  void setVal(const std::string& val) {
    value_ = val;
  }
  size_t getPosition() {
    return position_;
  }
  // Adds a new value to the trie, returns the last node representing the value
  // It also accumlates number of new nodes and new value size added.
  TrieNode* add(const std::string& value, size_t position, size_t& new_nodes, size_t& new_value_size);
  // Returns pointer to the child just added.
  TrieNode* addChild(std::unique_ptr<TrieNode> child);

  void serialize(std::string* buf, size_t parent_pos);

//  void serialize(std::ostream& os, size_t parent_pos, uint32_t& pos);

  void clear() {
    value_.clear();
    children_.clear();
    position_ = 0;
  }
  void print(std::ostream &out, size_t level) const;
};

class Trie {
private:
  TrieNode root_;
  size_t num_nodes_ = 1;
  size_t node_value_size_ = 0;
public:
  Trie() {}

  TriePosition add(const std::string& value);
  void add(const std::vector<std::string>& values);
  void serialize(std::string* buf);
  size_t estimatedSize() const {
    return node_value_size_ + static_cast<size_t>((num_nodes_ * 2) * kAvgVarintSize);
  }

  void clear() {
    root_.clear();
  }

  friend std::ostream& operator<< (std::ostream &os, const Trie &trie);
};

}  // namespace stbe
