#include "memblock.h"

namespace memblock {

Decoder::Decoder(const char* buf, size_t len) {
  reset(buf, len);
}

Decoder::Decoder(const std::string& buf) : Decoder::Decoder(buf.data(), buf.size()) {}

Decoder::Decoder() {}

void Decoder::reset(const char* buf, size_t len) {
  buf_ = buf;
  limit_ = buf_ + len;
  block_size_ = DecodeFixed32(buf_);
  values_offset_ = DecodeFixed32(buf_ + sizeof(uint32_t));
  value_ptr_ = buf_ + values_offset_;
}

bool Decoder::nextValue(std::string& value) {
  uint32_t node_pos;
  size_t total_size = 0;

  // return false when hit the end.
  if (value_ptr_ == nullptr) return false;

  // Get start position from value_ptr_, and advance it to next value.
  value_ptr_ = GetVarint32Ptr(value_ptr_, limit_, &node_pos);

  // return false when hit the end.
  if (value_ptr_ == nullptr) return false;

  std::vector<std::pair<const char*, uint32_t>> pieces;
  while (node_pos > 2 * sizeof(uint32_t)) {  // header size == 2 * sizeof(uint32_t)
    uint32_t len;
    const char* ptr = GetVarint32Ptr(buf_ + node_pos, limit_, &node_pos);
    ptr = GetVarint32Ptr(ptr, limit_, &len);
    pieces.emplace_back(std::make_pair(ptr, len));
    total_size += len;
  }
  value.clear();
  value.reserve(total_size);
  std::vector<std::pair<const char*, uint32_t>>::reverse_iterator rit = pieces.rbegin();
  for (; rit!= pieces.rend(); ++rit) {
    value.append(rit->first, rit->second);
  }

  return true;
}

}  // namespace memblock

