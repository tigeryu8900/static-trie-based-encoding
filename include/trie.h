#ifndef TRIE_H_INCLUDED
#define TRIE_H_INCLUDED

#include <iostream>
#include <memory>
#include <string>
#include <vector>

#include "util/coding.h"

constexpr int kM = 2;

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
  TrieNode* add(const std::string& value, size_t position);
  // returns pointer to the child just added.
  TrieNode* addChild(std::unique_ptr<TrieNode> child);

  void serialize(std::string* buf, size_t parent_pos);

//  void serialize(std::ostream& os, size_t parent_pos, uint32_t& pos);

  void reset() {
    value_.clear();
    children_.clear();
    position_ = 0;
  }
  void print(std::ostream &out, size_t level) const;
};

class Trie {
private:
  TrieNode root_;
  std::vector<TrieNode*> value_nodes_;
public:
  Trie() {}

  void add(const std::string& value);
  void add(const std::vector<std::string>& values);
  std::string serialize();
  size_t numValues() {
    return value_nodes_.size();
  }

  void reset() {
    value_nodes_.clear();
    root_.reset();
  }

  friend std::ostream& operator<< (std::ostream &os, const Trie &trie);
};

#endif  // TRIE_H_INCLUDED
