#include <iostream>
#include <memory>
#include <vector>

#include "gtest/gtest.h"
#include "trie.h"
#include "stbe.h"
#include "test_param.h"

namespace stbe {

// Record data type to be stored in stbe.
struct Record {
  std::string category;
  uint32_t total_items;
  std::string contact_phone;
};

// for EXPECT_EQ.
bool operator==(const Record& l, const Record& r) {
  return l.category == r.category &&
         l.total_items == r.total_items &&
         l.contact_phone == r.contact_phone;
} 

// Marshaller for our own data type: Record.
template<>
class recordMarshaller<Record> {
public:
  static float avgSize() {
    return kAvgVarintSize * 3;  // stores 3 varint per record
  }
  static void add2Trie(TrieValueEncoder& encoder, const Record& r) {
    encoder.addString2Trie(r.category);
    encoder.addString2Trie(r.contact_phone);
  }
  static void encode(TrieValueEncoder& encoder, const Record& r) {
    encoder.encodeString(0);
    encoder.encodeUint32(r.total_items);
    encoder.encodeString(1);
  }

  static bool decode(TrieValueDecoder& decoder, Record& r) {
    bool res = decoder.decodeString(r.category);
    res &= decoder.decodeUint32(r.total_items);
    res &= decoder.decodeString(r.contact_phone);
    return res;
  }
  static bool skip(TrieValueDecoder& decoder) {
    bool res = decoder.skipString();
    res &= decoder.skipUint32();
    res &= decoder.skipString();
    return res;
  }
};



TEST(CustomStructTest, TestCustomStruct)
{
  Record test_data[] = {
    {"/food/fruit/dried_fruit", 100, "+1-408-996-9900"}, 
    {"/food/fruit/fresh", 200, "+1-408-996-9901"}, 
    {"/food/beverage/juice", 6000, "+1-408-996-9902"}, 
    {"/book/fiction/", 90, "+1-408-996-9903"}, 
    {"/medicine/flu", 890, "+1-408-996-9904"}
  };
  Builder<Record> builder;
  builder.initialize("custom_struct_test_file");
  for (auto& r : test_data) {
    builder.add(r);
  }
  builder.finalize();
 
  Decoder<Record> decoder("custom_struct_test_file");
  ASSERT_EQ(sizeof(test_data)/sizeof(Record), decoder.totalRecords())
      << "Unmatched # of records.";
  Record value;
  for (auto& ori_value : test_data) {
    ASSERT_TRUE(decoder.nextRecord(value)) << "Unexpected end of values.";
    EXPECT_EQ(ori_value, value);
  }
  EXPECT_FALSE(decoder.nextRecord(value)) << "Too many values than expected.";
  
  for (uint32_t i = 0 ; i < decoder.totalRecords(); ++i) {
    EXPECT_EQ(test_data[i], decoder[i]);
  }

  EXPECT_EQ(Record{}, decoder[decoder.totalRecords()]) << "Too many values than expected.";
}

}  // namespace stbe
