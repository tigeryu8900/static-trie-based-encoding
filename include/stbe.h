#pragma once

#include <fstream>

#include "memblock.h"

namespace stbe {

constexpr size_t kDefaultBlockSize = 4 * 1024 * 1024;  // 4MB

class Builder{
private:
  Trie trie_;
  std::ofstream os_;
  uint32_t block_size_;
  // records offset and num of values in each block.
  std::vector<std::pair<uint32_t, uint32_t>> block_info_;

  void writeBlock(const std::string& block);
  size_t buildIndexBlock();
  void finishBlock();

public:
  explicit Builder(uint32_t block_size = kDefaultBlockSize);
  Builder(const std::string& fname, std::vector<std::string>& values,
          uint32_t block_size = kDefaultBlockSize);
  void initialize(const std::string& filename);
  
  void add(const std::string& value);
  void add(const std::vector<std::string>& values);
  void finalize();
};

	
class Decoder {
private:
  BlockDecoder decoder_;
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
  bool nextValue(std::string& value);
  bool skip(uint32_t num);
  const std::string operator[](const int index);
};

}  // namespace stbe

