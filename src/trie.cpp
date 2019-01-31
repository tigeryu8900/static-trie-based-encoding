

#include "trie.h"

TrieNode* TrieNode::add(const std::string& value, size_t position) {
  // search for child with the same prefix
  size_t remain_size = value.size() - position;

  for (auto& node : children_) {
    auto& val = node->getVal();
    // Calculate common prefix of val and value.
    size_t com_prex = 0;
    const char* p = val.data(); const char* q = value.data() + position;
    while (*p == *q && com_prex < val.size() && com_prex < remain_size) {
      com_prex++; ++p; ++q;
    }

    if (com_prex >= kM) {
      if (val.size() > com_prex) {
        // split val
        auto new_node = std::make_unique<TrieNode>(val.substr(0, com_prex));
        node->setVal(val.substr(com_prex));
        new_node->addChild(std::move(node));
        node = std::move(new_node);
        auto new_child = std::make_unique<TrieNode>(value.substr(position + com_prex));
        return node->addChild(std::move(new_child));
      } else { // val.size() == com_prex
        // add a child to node
        if (remain_size == val.size()) return node.get();
        return node->add(value, position + com_prex);
      }
    } else if (remain_size == com_prex) {
      // exact match
      return node.get();
    }
  }

  // Base case: no match
  children_.push_back(std::make_unique<TrieNode>(value.substr(position)));
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
/*
void TrieNode::serialize(std::ostream& os, size_t parent_pos, uint32_t& pos) {
  position_ = pos;
  std::string buf;
  PutVarint32Varint32(&buf, parent_pos, value_.size());
  //buf.append(value_);
  os << buf << value_;
  pos += buf.size() + value_.size();
  for (auto& c : children_) {
    c->serialize(os, position_, pos);
  }
}
*/

void Trie::add(const std::string& value) {
  value_nodes_.push_back(root_.add(value, 0));
}

void Trie::add(const std::vector<std::string>& values) {
  for (auto& s : values) {
    value_nodes_.push_back(root_.add(s, 0));
  }
}
std::ostream& operator<< (std::ostream &out, const Trie &trie) {
  trie.root_.print(out, 0);
  return out;
}

std::string Trie::serialize() {
  std::string buf;

  // Placeholder for values offset
  PutFixed32(&buf, 0); 

  // Serialize the root_
  root_.serialize(&buf, 0);
  uint32_t values_offset = buf.size();

  // Serialize value_nodes_
  for (auto& n : value_nodes_) {
    PutVarint32(&buf, n->getPosition());
  }

  // Write values offset
  EncodeFixed32(&buf[0], values_offset);
  return buf;
}

