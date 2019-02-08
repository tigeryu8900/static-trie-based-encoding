#include "trie.h"

namespace stbe {


size_t TriePosition::getPosition() const {
  return trie_node_->getPosition();
}


TrieNode* TrieNode::add(const std::string& value, size_t position, size_t& new_nodes, size_t& new_value_size) {
  // search for child with the same prefix
  size_t remain_size = value.size() - position;

  for (auto& node : children_) {
    auto& val = node->getVal();
    // Calculate common prefix of val and value.
    size_t com_prex = 0;
    const char* p = val.data();
    const char* q = value.data() + position;
    while (*p == *q && com_prex < val.size() && com_prex < remain_size) {
      com_prex++; ++p; ++q;
    }

    if (com_prex >= kMinCommPrefix) {
      if (val.size() > com_prex) {
        // split val
        auto new_node = std::make_unique<TrieNode>(val.substr(0, com_prex));
        node->setVal(val.substr(com_prex));
        new_node->addChild(std::move(node));
        node = std::move(new_node);
        new_nodes++;
        if (remain_size == com_prex) {
          return node.get();
        } else {
          auto new_child = std::make_unique<TrieNode>(value.substr(position + com_prex));
          new_nodes++;
          new_value_size += remain_size - com_prex;
          return node->addChild(std::move(new_child));
        }
      } else { // val.size() == com_prex
        // add a child to node
        if (remain_size == val.size()) return node.get();
        return node->add(value, position + com_prex, new_nodes, new_value_size);
      }
    } else if (com_prex == remain_size) {
      // exact match
      return node.get();
    }
  }

  // Base case: no match
  children_.push_back(std::make_unique<TrieNode>(value.substr(position)));
  new_nodes++;
  new_value_size += remain_size;
  return children_.back().get();
}

void TrieNode::print(std::ostream &out, size_t level) const {
  out << std::string(level * 2, ' ') << level << ": " << value_ << std::endl;
  for (auto& c : children_) {
    c->print(out, level + 1);
  }
}

TrieNode* TrieNode::addChild(std::unique_ptr<TrieNode> child) {
  children_.push_back(std::move(child));
  return children_.back().get();
}


void TrieNode::serialize(std::string* buf, size_t parent_pos) {
  position_ = buf->size();
  PutVarint32Varint32(buf, parent_pos, value_.size());
  buf->append(value_);
  for (auto& c : children_) {
    c->serialize(buf, position_);
  }
}

TriePosition Trie::add(const std::string& value) {
  TrieNode* node = root_.add(value, 0, num_nodes_, node_value_size_);
  return TriePosition{node};
}

void Trie::add(const std::vector<std::string>& values) {
  for (auto& s : values) {
    add(s);
  }
}

std::ostream& operator<< (std::ostream &out, const Trie &trie) {
  trie.root_.print(out, 0);
  return out;
}

void Trie::serialize(std::string* buf) {
  root_.serialize(buf, 0);
}

}  // namespace stbe
