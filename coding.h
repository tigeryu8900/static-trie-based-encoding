#pragma once

#include <algorithm>
#include <stdint.h>
#include <string.h>
#include <string>

#include "port.h"

// The maximum length of a varint in bytes for 64-bit.
const unsigned int kMaxVarint64Length = 10;

// Standard Put... routines append to a string
extern void PutFixed16(std::string* dst, uint16_t value);
extern void PutFixed32(std::string* dst, uint32_t value);
extern void PutFixed64(std::string* dst, uint64_t value);
extern void PutVarint32(std::string* dst, uint32_t value);
extern void PutVarint32Varint32(std::string* dst, uint32_t value1,
                                uint32_t value2);
extern void PutVarint32Varint32Varint32(std::string* dst, uint32_t value1,
                                        uint32_t value2, uint32_t value3);
extern void PutVarint64(std::string* dst, uint64_t value);
extern void PutVarint64Varint64(std::string* dst, uint64_t value1,
                                uint64_t value2);
extern void PutVarint32Varint64(std::string* dst, uint32_t value1,
                                uint64_t value2);
extern void PutVarint32Varint32Varint64(std::string* dst, uint32_t value1,
uint32_t value2, uint64_t value3);

// Standard Get... routines parse a value from the beginning of a Slice
// and advance the slice past the parsed value.
extern bool GetVarint32(std::string* input, uint32_t* value);
extern bool GetVarint64(std::string* input, uint64_t* value);

// Borrowed from
// https://github.com/facebook/fbthrift/blob/449a5f77f9f9bae72c9eb5e78093247eef185c04/thrift/lib/cpp/util/VarintUtils-inl.h#L202-L208
constexpr inline uint64_t i64ToZigzag(const int64_t l) {
  return (static_cast<uint64_t>(l) << 1) ^ static_cast<uint64_t>(l >> 63);
}
inline int64_t zigzagToI64(uint64_t n) {
  return (n >> 1) ^ -static_cast<int64_t>(n & 1);
}

// Pointer-based variants of GetVarint...  These either store a value
// in *v and return a pointer just past the parsed value, or return
// nullptr on error.  These routines only look at bytes in the range
// [p..limit-1]
extern const char* GetFixed64Ptr(const char* p,const char* limit, uint64_t* v);
extern const char* GetFixed32Ptr(const char* p,const char* limit, uint32_t* v);
extern const char* GetFixed16Ptr(const char* p,const char* limit, uint16_t* v);

extern const char* GetVarint32Ptr(const char* p,const char* limit, uint32_t* v);
extern const char* GetVarint64Ptr(const char* p,const char* limit, uint64_t* v);
inline const char* GetVarsignedint64Ptr(const char* p, const char* limit,
                                        int64_t* value) {
  uint64_t u = 0;
  const char* ret = GetVarint64Ptr(p, limit, &u);
  *value = zigzagToI64(u);
  return ret;
}

// Returns the length of the varint32 or varint64 encoding of "v"
extern int VarintLength(uint64_t v);

// Lower-level versions of Put... that write directly into a character buffer
// REQUIRES: dst has enough space for the value being written
extern void EncodeFixed16(char* dst, uint16_t value);
extern void EncodeFixed32(char* dst, uint32_t value);
extern void EncodeFixed64(char* dst, uint64_t value);

// Lower-level versions of Put... that write directly into a character buffer
// and return a pointer just past the last byte written.
// REQUIRES: dst has enough space for the value being written
extern char* EncodeVarint32(char* dst, uint32_t value);
extern char* EncodeVarint64(char* dst, uint64_t value);


// Lower-level versions of Get... that read directly from a character buffer
// without any bounds checking.

inline uint16_t DecodeFixed16(const char* ptr) {
  if (port::kLittleEndian) {
    // Load the raw bytes
    uint16_t result;
    memcpy(&result, ptr, sizeof(result));  // gcc optimizes this to a plain load
    return result;
  } else {
    return ((static_cast<uint16_t>(static_cast<unsigned char>(ptr[0]))) |
            (static_cast<uint16_t>(static_cast<unsigned char>(ptr[1])) << 8));
  }
}

inline uint32_t DecodeFixed32(const char* ptr) {
  if (port::kLittleEndian) {
    // Load the raw bytes
    uint32_t result;
    memcpy(&result, ptr, sizeof(result));  // gcc optimizes this to a plain load
    return result;
  } else {
    return ((static_cast<uint32_t>(static_cast<unsigned char>(ptr[0])))
        | (static_cast<uint32_t>(static_cast<unsigned char>(ptr[1])) << 8)
        | (static_cast<uint32_t>(static_cast<unsigned char>(ptr[2])) << 16)
        | (static_cast<uint32_t>(static_cast<unsigned char>(ptr[3])) << 24));
  }
}

inline uint64_t DecodeFixed64(const char* ptr) {
  if (port::kLittleEndian) {
    // Load the raw bytes
    uint64_t result;
    memcpy(&result, ptr, sizeof(result));  // gcc optimizes this to a plain load
    return result;
  } else {
    uint64_t lo = DecodeFixed32(ptr);
    uint64_t hi = DecodeFixed32(ptr + 4);
    return (hi << 32) | lo;
  }
}

// Internal routine for use by fallback path of GetVarint32Ptr
extern const char* GetVarint32PtrFallback(const char* p,
                                          const char* limit,
                                          uint32_t* value);
inline const char* GetVarint32Ptr(const char* p,
                                  const char* limit,
                                  uint32_t* value) {
  if (p < limit) {
    uint32_t result = *(reinterpret_cast<const unsigned char*>(p));
    if ((result & 128) == 0) {
      *value = result;
      return p + 1;
    }
  }
  return GetVarint32PtrFallback(p, limit, value);
}

// -- Implementation of the functions declared above
inline void EncodeFixed16(char* buf, uint16_t value) {
  if (port::kLittleEndian) {
    memcpy(buf, &value, sizeof(value));
  } else {
    buf[0] = value & 0xff;
    buf[1] = (value >> 8) & 0xff;
  }
}

inline void EncodeFixed32(char* buf, uint32_t value) {
  if (port::kLittleEndian) {
    memcpy(buf, &value, sizeof(value));
  } else {
    buf[0] = value & 0xff;
    buf[1] = (value >> 8) & 0xff;
    buf[2] = (value >> 16) & 0xff;
    buf[3] = (value >> 24) & 0xff;
  }
}

inline void EncodeFixed64(char* buf, uint64_t value) {
  if (port::kLittleEndian) {
    memcpy(buf, &value, sizeof(value));
  } else {
    buf[0] = value & 0xff;
    buf[1] = (value >> 8) & 0xff;
    buf[2] = (value >> 16) & 0xff;
    buf[3] = (value >> 24) & 0xff;
    buf[4] = (value >> 32) & 0xff;
    buf[5] = (value >> 40) & 0xff;
    buf[6] = (value >> 48) & 0xff;
    buf[7] = (value >> 56) & 0xff;
  }
}

// Pull the last 8 bits and cast it to a character
inline void PutFixed16(std::string* dst, uint16_t value) {
  if (port::kLittleEndian) {
    dst->append(const_cast<const char*>(reinterpret_cast<char*>(&value)),
                sizeof(value));
  } else {
    char buf[sizeof(value)];
    EncodeFixed16(buf, value);
    dst->append(buf, sizeof(buf));
  }
}

inline void PutFixed32(std::string* dst, uint32_t value) {
  if (port::kLittleEndian) {
    dst->append(const_cast<const char*>(reinterpret_cast<char*>(&value)),
      sizeof(value));
  } else {
    char buf[sizeof(value)];
    EncodeFixed32(buf, value);
    dst->append(buf, sizeof(buf));
  }
}

inline void PutFixed64(std::string* dst, uint64_t value) {
  if (port::kLittleEndian) {
    dst->append(const_cast<const char*>(reinterpret_cast<char*>(&value)),
      sizeof(value));
  } else {
    char buf[sizeof(value)];
    EncodeFixed64(buf, value);
    dst->append(buf, sizeof(buf));
  }
}

inline void PutVarint32(std::string* dst, uint32_t v) {
  char buf[5];
  char* ptr = EncodeVarint32(buf, v);
  dst->append(buf, static_cast<size_t>(ptr - buf));
}

inline void PutVarint32Varint32(std::string* dst, uint32_t v1, uint32_t v2) {
  char buf[10];
  char* ptr = EncodeVarint32(buf, v1);
  ptr = EncodeVarint32(ptr, v2);
  dst->append(buf, static_cast<size_t>(ptr - buf));
}

inline void PutVarint32Varint32Varint32(std::string* dst, uint32_t v1,
                                        uint32_t v2, uint32_t v3) {
  char buf[15];
  char* ptr = EncodeVarint32(buf, v1);
  ptr = EncodeVarint32(ptr, v2);
  ptr = EncodeVarint32(ptr, v3);
  dst->append(buf, static_cast<size_t>(ptr - buf));
}

inline char* EncodeVarint64(char* dst, uint64_t v) {
  static const unsigned int B = 128;
  unsigned char* ptr = reinterpret_cast<unsigned char*>(dst);
  while (v >= B) {
    *(ptr++) = (v & (B - 1)) | B;
    v >>= 7;
  }
  *(ptr++) = static_cast<unsigned char>(v);
  return reinterpret_cast<char*>(ptr);
}

inline void PutVarint64(std::string* dst, uint64_t v) {
  char buf[kMaxVarint64Length];
  char* ptr = EncodeVarint64(buf, v);
  dst->append(buf, static_cast<size_t>(ptr - buf));
}

inline void PutVarsignedint64(std::string* dst, int64_t v) {
  char buf[kMaxVarint64Length];
  // Using Zigzag format to convert signed to unsigned
  char* ptr = EncodeVarint64(buf, i64ToZigzag(v));
  dst->append(buf, static_cast<size_t>(ptr - buf));
}

inline void PutVarint64Varint64(std::string* dst, uint64_t v1, uint64_t v2) {
  char buf[20];
  char* ptr = EncodeVarint64(buf, v1);
  ptr = EncodeVarint64(ptr, v2);
  dst->append(buf, static_cast<size_t>(ptr - buf));
}

inline void PutVarint32Varint64(std::string* dst, uint32_t v1, uint64_t v2) {
  char buf[15];
  char* ptr = EncodeVarint32(buf, v1);
  ptr = EncodeVarint64(ptr, v2);
  dst->append(buf, static_cast<size_t>(ptr - buf));
}

inline void PutVarint32Varint32Varint64(std::string* dst, uint32_t v1,
                                        uint32_t v2, uint64_t v3) {
  char buf[20];
  char* ptr = EncodeVarint32(buf, v1);
  ptr = EncodeVarint32(ptr, v2);
  ptr = EncodeVarint64(ptr, v3);
  dst->append(buf, static_cast<size_t>(ptr - buf));
}

inline int VarintLength(uint64_t v) {
  int len = 1;
  while (v >= 128) {
    v >>= 7;
    len++;
  }
  return len;
}

inline const char* GetFixed64(const char* input, const char* limit, uint64_t* value) {
  if (limit - input < sizeof(uint64_t)) {
    return nullptr;
  }
  *value = DecodeFixed64(input);
  return input + sizeof(uint64_t);
}

inline const char* GetFixed32(const char* input, const char* limit, uint32_t* value) {
  if (limit - input < sizeof(uint32_t)) {
    return nullptr;
  }
  *value = DecodeFixed32(input);
  return input + sizeof(uint32_t);
}

inline const char* GetFixed16(const char* input, const char* limit, uint16_t* value) {
  if (limit - input < sizeof(uint16_t)) {
    return nullptr;
  }
  *value = DecodeFixed16(input);
  return input + sizeof(uint16_t);
}

inline bool GetVarint32(std::string* input, uint32_t* value) {
  const char* p = input->data();
  const char* limit = p + input->size();
  const char* q = GetVarint32Ptr(p, limit, value);
  return (q != nullptr);
}

inline bool GetVarint64(std::string* input, uint64_t* value) {
  const char* p = input->data();
  const char* limit = p + input->size();
  const char* q = GetVarint64Ptr(p, limit, value);
  return (q != nullptr);
}

// Provide an interface for platform independent endianness transformation
inline uint64_t EndianTransform(uint64_t input, size_t size) {
  char* pos = reinterpret_cast<char*>(&input);
  uint64_t ret_val = 0;
  for (size_t i = 0; i < size; ++i) {
    ret_val |= (static_cast<uint64_t>(static_cast<unsigned char>(pos[i]))
                << ((size - i - 1) << 3));
  }
  return ret_val;
}


