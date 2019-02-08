#include "memblock.h"

namespace stbe {

BlockDecoder::BlockDecoder(const char* buf, size_t len) {
  reset(buf, len);
}

BlockDecoder::BlockDecoder(const std::string& buf)
    : BlockDecoder::BlockDecoder(buf.data(), buf.size()) {}

BlockDecoder::BlockDecoder() {}

bool BlockDecoder::reset(const char* buf, size_t len) {
  if (buf == nullptr || len <= sizeof(uint32_t)) return false;
  buf_ = buf;
  limit_ = buf_ + len;
  values_offset_ = DecodeFixed32(buf_);
  value_ptr_ = buf_ + values_offset_;
  return true;
}

bool BlockDecoder::nextValue(std::string& value) {
  uint32_t node_pos;
  size_t total_size = 0;

  // return false when hit the end.
  if (value_ptr_ == nullptr) return false;

  // Get start position from value_ptr_, and advance it to next value.
  value_ptr_ = GetVarint32Ptr(value_ptr_, limit_, &node_pos);

  // return false when hit the end.
  if (value_ptr_ == nullptr) return false;

  std::vector<std::pair<const char*, uint32_t>> pieces;
  while (node_pos > sizeof(uint32_t)) {  // header size == sizeof(uint32_t)
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

bool BlockDecoder::skip(uint32_t num) {
  uint32_t node_pos;
  for (uint32_t i = 0; i < num; ++i) {
    if (value_ptr_ == nullptr) return false;
    // Get start position from value_ptr_, and advance it to next value.
    value_ptr_ = GetVarint32Ptr(value_ptr_, limit_, &node_pos);
  }
  return (value_ptr_ != nullptr);
}

}  // namespace stbe

