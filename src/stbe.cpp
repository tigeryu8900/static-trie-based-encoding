#include "stbe.h"

namespace stbe {

Builder::Builder(const std::string& fname, std::vector<std::string>& values) {
  initialize(fname);
  add(values);
  buildIndexBlock();
  finalize();
}

void Builder::initialize(const std::string& filename) {
  os_.open(filename, std::ofstream::trunc | std::ofstream::binary);
}

void Builder::add(const std::string& value) {
  trie_.add(value);
  // check if the trie_ is over estimated size limit, then create a new block.
  if (trie_.numValues() >= 2) finishBlock();
}

void Builder::add(const std::vector<std::string>& values) {
  for (auto& s : values) {
    add(s);
  }
}

void Builder::finishBlock() {
  block_info_.emplace_back(std::make_pair<uint32_t, uint32_t>(os_.tellp(), trie_.numValues()));
  trie_.serialize(os_);
  trie_.reset();
}

size_t Builder::buildIndexBlock() {
  size_t index_block_offset = os_.tellp();
  std::string buf; 
  for(auto& b : block_info_) {
    buf.clear();
    PutVarint32Varint32(&buf, b.first, b.second);
    os_ << buf;
  }

  return index_block_offset; 
}

void Builder::finalize() {
  if (trie_.numValues() > 0) {
    finishBlock();  
  }

  // Write footer
  std::string buf;
  PutFixed32(&buf, buildIndexBlock());
  os_ << buf;
  os_.close();
}



Decoder::Decoder(const std::string& fname) : inf_(fname) {

  // go to the end of the file and read out index_block_offset_
  int cur = inf_.tellg();
  inf_.seekg(-sizeof(uint32_t), std::ifstream::end);
  char buf[sizeof(uint32_t)];
  inf_.read(buf, sizeof(uint32_t));
  index_block_offset_ = DecodeFixed32(buf);
  //std::cout << "index offset: " << index_block_offset_ << std::endl;
  inf_.seekg(cur, std::ifstream::beg);
}

bool Decoder::loadNextBlock() {
  // determine whether there are still more blocks
  // return if current position >= index_block_offset_
  //if (inf_.eof()) return false;

  buf_.resize(sizeof(uint32_t));
  inf_.read(&buf_[0], sizeof(uint32_t));
  if (inf_.tellg() >= index_block_offset_) return false;
  uint32_t block_len = DecodeFixed32(buf_.data());
  buf_.resize(block_len);
  inf_.read(&buf_[sizeof(uint32_t)], block_len - sizeof(uint32_t));
  decoder_.reset(buf_.data(), block_len);
  return true;
}


bool Decoder::nextValue(std::string& value) {
  // return false if the file cursor has already passed the last element
  if (inf_.eof()) return false;
  bool has_value = decoder_.nextValue(value);
  if (has_value) return true;

  if (!loadNextBlock()) return false;

  return decoder_.nextValue(value);
}

const std::string& Decoder::operator[](const int index) const {
  for (int i = 0; i <= index; i++) {
    
  }
}

}  // namespace stbe
