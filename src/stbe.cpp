#include "stbe.h"

namespace stbe {

Builder::Builder(uint32_t block_size) : block_size_(block_size) {
}

Builder::Builder(const std::string& fname, std::vector<std::string>& values,
                 uint32_t block_size) : block_size_(block_size) {
  initialize(fname);
  add(values);
  finalize();
}

void Builder::initialize(const std::string& filename) {
  os_.open(filename, std::ofstream::trunc | std::ofstream::binary);
}

void Builder::add(const std::string& value) {
  trie_.add(value);
  // check if the trie_ is over estimated size limit, then create a new block.
  if (trie_.estimatedSize() >= block_size_) finishBlock();
}

void Builder::add(const std::vector<std::string>& values) {
  for (auto& s : values) {
    add(s);
  }
}

void Builder::writeBlock(const std::string& block) {
  // write a block header, currently it only contains a block length.
  std::string header;
  PutFixed32(&header, block.size());
  os_ << header << block;
}

void Builder::finishBlock() {
  block_info_.emplace_back(std::make_pair<uint32_t, uint32_t>(os_.tellp(), trie_.numValues()));
  writeBlock(trie_.serialize());
  trie_.reset();
}

size_t Builder::buildIndexBlock() {
  size_t index_block_offset = os_.tellp();
  std::string buf;
  // Index block consists: <# of blocks>[<block offset, # of items block>]
  PutVarint32(&buf, block_info_.size());
  for(auto& b : block_info_) {
    PutVarint32Varint32(&buf, b.first, b.second);
  }
  writeBlock(buf);

  return index_block_offset; 
}

void Builder::finalize() {
  // finish the last block if not empty.
  if (trie_.numValues() > 0) {
    finishBlock();  
  }

  // Write the index block.
  std::string buf;
  PutFixed32(&buf, buildIndexBlock());

  // Append the offset of index block as Fixed32 at the end of file.
  os_ << buf;
  os_.close();
}



Decoder::Decoder(const std::string& fname) : inf_(fname, std::ifstream::binary) {
  // go to the end of the file and read out index_block_offset_
  int cur = inf_.tellg();
  inf_.seekg(-sizeof(uint32_t), std::ifstream::end);
  file_size_ = inf_.tellg();
  char buf[sizeof(uint32_t)];
  inf_.read(buf, sizeof(uint32_t));
  index_block_offset_ = DecodeFixed32(buf);
  loadBlockIndex();
  inf_.seekg(cur, std::ifstream::beg);
}

bool Decoder::loadBlockIndex() {
std::cout << "Decoder::cacheIndex called" << std::endl;
  // return true if the index block is already cached
  if (blocks_info_.size() > 0) return true;
  if (!loadBlock(index_block_offset_, file_size_)) return false;

  // load the index block to cache
  const char* ptr = buf_.data();
  const char* limit = buf_.data() + buf_.size();
  uint32_t num_blocks = 0;
  ptr = GetVarint32Ptr(ptr, limit, &num_blocks);
  if (ptr == nullptr) return false;
  blocks_info_.resize(num_blocks);
  uint32_t total_records = 0;
  for (int i = 0; i < num_blocks; ++i) {
    ptr = GetVarint32Ptr(ptr, limit, &blocks_info_[i].offset);
    if (ptr == nullptr) break;
    ptr = GetVarint32Ptr(ptr, limit, &blocks_info_[i].num_records);
    if (ptr == nullptr) break;
    total_records += blocks_info_[i].num_records;
    blocks_info_[i].accumlated_records = total_records;
  }
std::cout << "blocks_info_: " << blocks_info_.size() << std::endl;
  for (auto& i : blocks_info_) {
std::cout << "" << i.offset << ' ' << i.num_records << ' ' << i.accumlated_records << std::endl;
  }
  return true;
}

bool Decoder::loadBlock(uint32_t offset, uint32_t limit) {
  inf_.seekg(offset, std::ifstream::beg);
  char header[sizeof(uint32_t)];
  inf_.read(header, sizeof(uint32_t));
  uint32_t block_len = DecodeFixed32(header);
  // sanity check, verify integrity of the block 
  if (offset + block_len + sizeof(uint32_t) > limit) return false;
  buf_.resize(block_len);
  inf_.read(&buf_[0], block_len);
  return true;
}

bool Decoder::loadDataBlock(uint32_t index) {
  if (index >= blocks_info_.size() || 
      !loadBlock(blocks_info_[index].offset, index_block_offset_)) {
    return false;
  }
  current_block_num_ = index;
  return decoder_.reset(buf_.data(), buf_.size());
}

bool Decoder::nextValue(std::string& value) {
  // return false if the file cursor has already passed the last element
  bool has_value = decoder_.nextValue(value);
  if (has_value) return true;

  if (!loadDataBlock(current_block_num_ + 1)) return false;

  return decoder_.nextValue(value);
}

bool Decoder::skip(uint32_t num) {
  return decoder_.skip(num);
}

uint32_t Decoder::locateBlock(uint32_t record_index, uint32_t begin, uint32_t end) const {
  if (begin == end) {
    return record_index <= blocks_info_[begin].accumlated_records 
        ? begin : blocks_info_.size();
  }
  uint32_t mid = (begin + end) / 2;
  if (record_index <= blocks_info_[mid].accumlated_records) {
    return locateBlock(record_index, begin, mid);
  } else {
    return locateBlock(record_index, mid + 1, end);
  }
}

const std::string Decoder::operator[](const int index) {
  uint32_t block_num = locateBlock(index);
  // returns empty string if index out of range.
  if (block_num >= blocks_info_.size() || !loadDataBlock(block_num)) return "";

  uint32_t index_offset = index;
  if (block_num > 0) {
    index_offset -= blocks_info_[block_num - 1].accumlated_records;
  }
  if (!skip(index_offset)) return "";
  std::string value;
  nextValue(value);
  return value;
}

}  // namespace stbe
