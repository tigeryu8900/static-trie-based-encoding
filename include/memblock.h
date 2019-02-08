#pragma once

#include <iostream>
#include <memory>
#include <string>
#include <vector>

#include "util/coding.h"
#include "trie.h"

namespace stbe {

class BlockDecoder {
private:
  const char* buf_ = nullptr;
  const char* limit_ = nullptr;
  uint32_t values_offset_ = 0;
  const char* value_ptr_ = nullptr;

public:
  explicit BlockDecoder(const std::string& buf);
  BlockDecoder(const char* buf, size_t len);
  BlockDecoder();

  bool reset(const char* buf, size_t len);

  //size_t totalValues() {
  //  return total_values_;
  //}
  // get the next value.
  bool nextValue(std::string& value);
  // skip n values
  bool skip(uint32_t num);
};

}  // namespace stbe
