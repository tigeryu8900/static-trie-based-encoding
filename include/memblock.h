#pragma once

#include <iostream>
#include <memory>
#include <string>
#include <vector>

#include "util/coding.h"
#include "trie.h"

namespace stbe {

class TrieValueDecoder {
public:
  virtual ~TrieValueDecoder() {}
  virtual std::string getString(size_t position) const = 0;
};

template <typename T>
class recordMarshaller;

template <typename T, typename RecordEncoder = recordMarshaller<T> >
class BlockEncoder {
private:
  Trie trie_;
  std::vector<T> records_;
  std::vector<std::vector<TriePosition>> positions_;

public:
  BlockEncoder() {}

  void add(T record) {
    records_.emplace_back(std::move(record));
    positions_.emplace_back();
    RecordEncoder::add2Trie(trie_, records_.back(), positions_.back());
  }

  std::string serialize();

  size_t numRecords() {
    return records_.size();
  }

  size_t estimatedSize() {
    return trie_.estimatedSize() +
        static_cast<size_t>(records_.size() * RecordEncoder::avgSize());
  }

  void clear() {
    trie_.clear();
    records_.clear();
    positions_.clear();
  }
};

template <typename T, typename RecordDecoder = recordMarshaller<T> >
class BlockDecoder : public TrieValueDecoder {
private:
  const char* buf_ = nullptr;
  const char* limit_ = nullptr;
  uint32_t records_offset_ = 0;
  const char* record_ptr_ = nullptr;

public:
  explicit BlockDecoder(const std::string& buf);
  BlockDecoder(const char* buf, size_t len);
  BlockDecoder();

  std::string getString(size_t position) const override;

  bool reset(const char* buf, size_t len);

  // get the next record.
  bool nextRecord(T& record);
  // skip n records
  bool skip(uint32_t num);
};

template <>
class recordMarshaller<std::string> {
public:
  static float avgSize() {
    return kAvgVarintSize;
  }
  static void add2Trie(Trie& trie, const std::string& record,
                       std::vector<TriePosition>& positions) {
    positions.emplace_back(trie.add(record));
  }
  static void encode(std::string* buf, const std::string& record,
                     const std::vector<TriePosition>& positions) {
    PutVarint32(buf, positions[0].getPosition());
  }


  static const char* decode(const TrieValueDecoder& decoder,
                            const char* record_ptr, const char* limit,
                            std::string& record) {
    uint32_t node_pos;
    const char* ptr = GetVarint32Ptr(record_ptr, limit, &node_pos);
    record = decoder.getString(node_pos); 
    return ptr;
  }
  static const char* skip(const char* record_ptr, const char* limit) {
    uint32_t node_pos;
    return GetVarint32Ptr(record_ptr, limit, &node_pos);
  }
};


// Templates implementation

template <typename T, typename RecordEncoder>
std::string BlockEncoder<T, RecordEncoder>::serialize() {
  std::string buf;

  // Placeholder for recordss offset
  PutFixed32(&buf, 0); 

  // Serialize the trie_
  trie_.serialize(&buf);
  uint32_t records_offset = buf.size();

  // Serialize records
  for (int i = 0; i < records_.size(); ++i) {
    RecordEncoder::encode(&buf, records_[i], positions_[i]);
  }

  // Write records offset
  EncodeFixed32(&buf[0], records_offset);

  return buf;
}

template <typename T, typename RecordDecoder>
BlockDecoder<T, RecordDecoder>::BlockDecoder(const char* buf, size_t len) {
  reset(buf, len);
}

template <typename T, typename RecordDecoder>
BlockDecoder<T, RecordDecoder>::BlockDecoder(const std::string& buf)
    : BlockDecoder::BlockDecoder(buf.data(), buf.size()) {}

template <typename T, typename RecordDecoder>
BlockDecoder<T, RecordDecoder>::BlockDecoder() {}

template <typename T, typename RecordDecoder>
bool BlockDecoder<T, RecordDecoder>::reset(const char* buf, size_t len) {
  if (buf == nullptr || len <= sizeof(uint32_t)) return false;
  buf_ = buf;
  limit_ = buf_ + len;
  records_offset_ = DecodeFixed32(buf_);
  record_ptr_ = buf_ + records_offset_;
  return true;
}

template <typename T, typename RecordDecoder>
std::string BlockDecoder<T, RecordDecoder>::getString(size_t position) const {
  std::string value;
  uint32_t node_pos = position;
  size_t total_size = 0;
  std::vector<std::pair<const char*, uint32_t>> pieces;
  while (node_pos > sizeof(uint32_t)) {  // header is a uint32_t
    uint32_t len;
    const char* ptr = GetVarint32Ptr(buf_ + node_pos, limit_, &node_pos);
    ptr = GetVarint32Ptr(ptr, limit_, &len);
    pieces.emplace_back(std::make_pair(ptr, len));
    total_size += len;
  }
  value.reserve(total_size);
  std::vector<std::pair<const char*, uint32_t>>::reverse_iterator rit = pieces.rbegin();
  for (; rit!= pieces.rend(); ++rit) {
    value.append(rit->first, rit->second);
  }

  return value;
}

template <typename T, typename RecordDecoder>
bool BlockDecoder<T, RecordDecoder>::nextRecord(T& record) {
  // return false when hit the end.
  if (record_ptr_ == nullptr) return false;

  // Get start position from record_ptr_, and advance it to next record.
  record_ptr_ = RecordDecoder::decode(*this, record_ptr_, limit_, record);

  // return false when hit the end.
  if (record_ptr_ == nullptr) return false;

  return true;
}

template <typename T, typename RecordDecoder>
bool BlockDecoder<T, RecordDecoder>::skip(uint32_t num) {
  uint32_t node_pos;
  for (uint32_t i = 0; i < num; ++i) {
    if (record_ptr_ == nullptr) return false;
    record_ptr_ = RecordDecoder::skip(record_ptr_, limit_);
  }
  return (record_ptr_ != nullptr);
}

}  // namespace stbe
