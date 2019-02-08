#pragma once

#include <fstream>

#include "memblock.h"

namespace stbe {

constexpr size_t kDefaultBlockSize = 4 * 1024 * 1024;  // 4MB

template <typename T, typename RecordEncoder = recordMarshaller<T> >
class Builder{
private:
  BlockEncoder<T, RecordEncoder> encoder_;
  std::ofstream os_;
  uint32_t block_size_;
  // records offset and num of records in each block.
  std::vector<std::pair<uint32_t, uint32_t>> block_info_;

  void writeBlock(const std::string& block);
  size_t buildIndexBlock();
  void finishBlock();

public:
  explicit Builder(uint32_t block_size = kDefaultBlockSize);
  Builder(const std::string& fname, std::vector<T>& records,
          uint32_t block_size = kDefaultBlockSize);
  void initialize(const std::string& filename);
  
  void add(const T& record);
  void add(const std::vector<T>& records);
  void finalize();
};

	
template <typename T, typename RecordDecoder = recordMarshaller<T> >
class Decoder {
private:
  BlockDecoder<T, RecordDecoder> decoder_;
  std::ifstream inf_;
  std::string buf_;  // data buffer for decoder
  uint32_t index_block_offset_ = 0;
  uint32_t file_size_ = 0;
  int32_t current_block_num_ = -1;

  struct BlockInfo {
    uint32_t offset;
    uint32_t num_records;
    uint32_t accumlated_records;
  };
  std::vector<BlockInfo> blocks_info_;

  bool loadBlockIndex();
  bool loadBlock(uint32_t offset, uint32_t limit);
  bool loadDataBlock(uint32_t offset);

  // locate the block number by record_index, returns blocks_info_.size() + 1 
  // if record_index is out of range.
  uint32_t locateBlock(uint32_t record_index, uint32_t begin, uint32_t end) const;
  uint32_t locateBlock(uint32_t record_index) const {
    return locateBlock(record_index, 0, blocks_info_.size() - 1);
  }

public:
  //Decoder() {};
  explicit Decoder(const std::string& fname);

  size_t totalRecords() {
    return blocks_info_.back().accumlated_records;
  }
  bool nextRecord(T& record);
  bool skip(uint32_t num);
  const T operator[](const int index);
};

// Templates implementation

template <typename T, typename RecordEncoder>
Builder<T, RecordEncoder>::Builder(uint32_t block_size) : block_size_(block_size) {
}

template <typename T, typename RecordEncoder>
Builder<T, RecordEncoder>::Builder(const std::string& fname, std::vector<T>& records,
                 uint32_t block_size) : block_size_(block_size) {
  initialize(fname);
  add(records);
  finalize();
}

template <typename T, typename RecordEncoder>
void Builder<T, RecordEncoder>::initialize(const std::string& filename) {
  os_.open(filename, std::ofstream::trunc | std::ofstream::binary);
}

template <typename T, typename RecordEncoder>
void Builder<T, RecordEncoder>::add(const T& record) {
  encoder_.add(record);
  // check if the block is over estimated size limit, then create a new block.
  if (encoder_.estimatedSize() >= block_size_) finishBlock();
}

template <typename T, typename RecordEncoder>
void Builder<T, RecordEncoder>::add(const std::vector<T>& records) {
  for (auto& r : records) {
    add(r);
  }
}

template <typename T, typename RecordEncoder>
void Builder<T, RecordEncoder>::writeBlock(const std::string& block) {
  // write a block header, currently it only contains a block length.
  std::string header;
  PutFixed32(&header, block.size());
  os_ << header << block;
}

template <typename T, typename RecordEncoder>
void Builder<T, RecordEncoder>::finishBlock() {
  block_info_.emplace_back(std::make_pair<uint32_t, uint32_t>(os_.tellp(), encoder_.numRecords()));
  writeBlock(encoder_.serialize());
  encoder_.clear();
}

template <typename T, typename RecordEncoder>
size_t Builder<T, RecordEncoder>::buildIndexBlock() {
  size_t index_block_offset = os_.tellp();
  std::string buf;
  // Index block consists: <# of blocks>[<block offset, # of items in block>]
  PutVarint32(&buf, block_info_.size());
  for(auto& b : block_info_) {
    PutVarint32Varint32(&buf, b.first, b.second);
  }
  writeBlock(buf);

  return index_block_offset; 
}

template <typename T, typename RecordEncoder>
void Builder<T, RecordEncoder>::finalize() {
  // finish the last block if not empty.
  if (encoder_.numRecords() > 0) {
    finishBlock();  
  }

  // Write the index block.
  std::string buf;
  PutFixed32(&buf, buildIndexBlock());

  // Append the offset of index block as Fixed32 at the end of file.
  os_ << buf;
  os_.close();
}



template <typename T, typename RecordDecoder>
Decoder<T, RecordDecoder>::Decoder(const std::string& fname) : inf_(fname, std::ifstream::binary) {
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

template <typename T, typename RecordDecoder>
bool Decoder<T, RecordDecoder>::loadBlockIndex() {
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
  return true;
}

template <typename T, typename RecordDecoder>
bool Decoder<T, RecordDecoder>::loadBlock(uint32_t offset, uint32_t limit) {
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

template <typename T, typename RecordDecoder>
bool Decoder<T, RecordDecoder>::loadDataBlock(uint32_t index) {
  if (index >= blocks_info_.size() || 
      !loadBlock(blocks_info_[index].offset, index_block_offset_)) {
    return false;
  }
  current_block_num_ = index;
  return decoder_.reset(buf_.data(), buf_.size());
}

template <typename T, typename RecordDecoder>
bool Decoder<T, RecordDecoder>::nextRecord(T& record) {
  // return false if the file cursor has already passed the last element
  bool has_record = decoder_.nextRecord(record);
  if (has_record) return true;

  if (!loadDataBlock(current_block_num_ + 1)) return false;

  return decoder_.nextRecord(record);
}

template <typename T, typename RecordDecoder>
bool Decoder<T, RecordDecoder>::skip(uint32_t num) {
  return decoder_.skip(num);
}

template <typename T, typename RecordDecoder>
uint32_t Decoder<T, RecordDecoder>::locateBlock(uint32_t record_index, uint32_t begin, uint32_t end) const {
  if (begin == end) {
    return record_index < blocks_info_[begin].accumlated_records 
        ? begin : blocks_info_.size();
  }
  uint32_t mid = (begin + end) / 2;
  if (record_index <= blocks_info_[mid].accumlated_records) {
    return locateBlock(record_index, begin, mid);
  } else {
    return locateBlock(record_index, mid + 1, end);
  }
}

template <typename T, typename RecordDecoder>
const T Decoder<T, RecordDecoder>::operator[](const int index) {
  uint32_t block_num = locateBlock(index);
  T record;
  // returns empty record if index out of range.
  if (block_num >= blocks_info_.size() || !loadDataBlock(block_num)) {
    return record;
  }

  uint32_t index_offset = index;
  index_offset -= blocks_info_[block_num].accumlated_records -
                  blocks_info_[block_num].num_records;
  if (skip(index_offset)) { 
    nextRecord(record);
  }
  return record;
}

}  // namespace stbe

