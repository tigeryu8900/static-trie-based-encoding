#pragma once

#include <iostream>
#include <memory>
#include <string>
#include <vector>

#include "util/coding.h"
#include "trie.h"

namespace stbe {

class TrieValueEncoder {
public:
  virtual void addString2Trie(const std::string& value) = 0;
  virtual void encodeString(size_t index) = 0;
  virtual void encodeUint32(uint32_t value) = 0;
};

class TrieValueDecoder {
public:
  virtual bool decodeString(std::string& value) = 0;
  virtual bool decodeUint32(uint32_t& value) = 0;
  virtual bool skipString() = 0;
  virtual bool skipUint32() = 0;
};


template <typename RecordType>
class recordMarshaller;

// Specialize marshaller for RecordType == std::string.
template <>
class recordMarshaller<std::string> {
public:
  static float avgSize() {
    return kAvgVarintSize;
  }
  static void add2Trie(TrieValueEncoder& trie_encoder, const std::string& record) {
    trie_encoder.addString2Trie(record);
  }
  static void encode(TrieValueEncoder& trie_encoder, const std::string& record) {
    trie_encoder.encodeString(0);
  }

  static bool decode(TrieValueDecoder& trie_decoder, std::string& record) {
    return trie_decoder.decodeString(record);
  }
  static bool skip(TrieValueDecoder& trie_decoder) {
    return trie_decoder.skipString();
  }
};



template <typename T, typename RecordEncoder = recordMarshaller<T> >
class BlockEncoder : public TrieValueEncoder {
private:
  Trie trie_;
  std::vector<T> records_;
  std::vector<std::vector<TriePosition>> positions_;
  std::vector<TriePosition>* cur_positions_ = nullptr;  // points to current position vector during serialization
  std::string buf_;  // temporary buffer for serialization
  
public:
  BlockEncoder() {}

  // TrieValueEncoder functions.
  void addString2Trie(const std::string& value) override {
    positions_.back().emplace_back(trie_.add(value));
  }
  void encodeString(size_t index) override {
    PutVarint32(&buf_, cur_positions_->at(index).getPosition());
  }
  void encodeUint32(uint32_t value) override {
    PutVarint32(&buf_, value);
  }

  void add(T record) {
    records_.emplace_back(std::move(record));
    positions_.emplace_back();
    RecordEncoder::add2Trie(*this, records_.back());
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

  // TrieValueDecoder functions.
  bool decodeString(std::string& value) override;
  bool decodeUint32(uint32_t& value) override {
    return nullptr != GetVarint32Ptr(record_ptr_, limit_, &value);
  }
  bool skipUint32() override {
    uint32_t dummy;
    return decodeUint32(dummy);
  }
  bool skipString() override {
    return skipUint32();
  }

  bool reset(const char* buf, size_t len);

  // get the next record.
  bool nextRecord(T& record);
  // skip n records
  bool skip(uint32_t num);
};

// Templates implementation

template <typename T, typename RecordEncoder>
std::string BlockEncoder<T, RecordEncoder>::serialize() {
  buf_.clear();
  // Placeholder for recordss offset
  PutFixed32(&buf_, 0); 

  // Serialize the trie_
  trie_.serialize(&buf_);
  uint32_t records_offset = buf_.size();

  // Serialize records
  for (int i = 0; i < records_.size(); ++i) {
    RecordEncoder::encode(*this, records_[i]);
  }

  // Write records offset
  EncodeFixed32(&buf_[0], records_offset);

  return std::move(buf_);
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


// TrieValueDecoder functions.
template <typename T, typename RecordDecoder>
bool BlockDecoder<T, RecordDecoder>::decodeString(std::string& value) {
  uint32_t node_pos;
  // get the last node position from record.
  if (!decodeUint32(node_pos)) return false;

  size_t total_size = 0;
  std::vector<std::pair<const char*, uint32_t>> pieces;
  while (node_pos > sizeof(uint32_t)) {  // header is a uint32_t
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


template <typename T, typename RecordDecoder>
bool BlockDecoder<T, RecordDecoder>::nextRecord(T& record) {
  // return false when hit the end.
  if (record_ptr_ == nullptr) return false;

  // Get start position from record_ptr_, and advance it to next record.
  if (!RecordDecoder::decode(*this, record)) return false;

  // return false when hit the end.
  if (record_ptr_ == nullptr) return false;

  return true;
}

template <typename T, typename RecordDecoder>
bool BlockDecoder<T, RecordDecoder>::skip(uint32_t num) {
  uint32_t node_pos;
  for (uint32_t i = 0; i < num; ++i) {
    if (record_ptr_ == nullptr || !RecordDecoder::skip(*this)) return false;
  }
  return (record_ptr_ != nullptr);
}

}  // namespace stbe
