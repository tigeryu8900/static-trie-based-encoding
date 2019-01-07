#ifndef STBE_H_INCLUDED
#define STBE_H_INCLUDED

#include <fstream>

#include "memblock.h"

namespace stbe {

class Builder{
private:
  Trie trie_;
  std::ofstream os_;
  // records offset and num of values in each block.
  std::vector<std::pair<uint32_t, uint32_t>> block_info_;

  void finishBlock();

public:
  Builder(const std::string& fname, std::vector<std::string>& values);
  void initialize(const std::string& filename);
  
  void add(const std::string& value);
  void add(const std::vector<std::string>& values);
  //void finishBlock();
  size_t buildIndexBlock();
  void finalize();
  //void finialize();
};

	
class Decoder {
private:
  memblock::Decoder decoder_;
  std::ifstream inf_;
  std::string buf_;  // data buffer for decoder
  std::string index_block_cache_;
  uint32_t index_block_offset_ = 0;
  uint32_t file_size_ = 0;

public:
  explicit Decoder(const std::string& fname);
  bool loadNextBlock();

  //size_t totalValues() {
  //  return total_values_;
  //}
  bool nextValue(std::string& value);
  const std::string& operator[](const int index) const;
};

}  // namespace stbe

#endif  // STBE_H_INCLUDED

