#ifndef MEMBLOCK_H_INCLUDED
#define MEMBLOCK_H_INCLUDED

#include <iostream>
#include <memory>
#include <string>
#include <vector>

#include "coding.h"
#include "trie.h"

namespace memblock {

class Decoder {
private:
  const char* buf_ = nullptr;
  const char* limit_ = nullptr;
  uint32_t block_size_ = 0;
  uint32_t values_offset_ = 0;
  const char* value_ptr_ = nullptr;

public:
  explicit Decoder(const std::string& buf);
  Decoder(const char* buf, size_t len);
  Decoder();

  void reset(const char* buf, size_t len);

  //size_t totalValues() {
  //  return total_values_;
  //}
  bool nextValue(std::string& value);
};


}  // namespace memblock

#endif  // MEMBLOCK_H_INCLUDED

