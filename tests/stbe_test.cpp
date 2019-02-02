#include <iostream>
#include <memory>
#include <vector>

#include "gtest/gtest.h"
#include "trie.h"
#include "stbe.h"
#include "test_param.h"

namespace stbe {

class STBETest : public ::testing::TestWithParam<TestParam> {
};


TEST_P(STBETest, Decode)
{
  const TestParam& t = GetParam();
  Builder builder;
  builder.initialize("test_file");
  builder.add(t.input);
  builder.finalize();
    
  Decoder decoder("test_file");
  ASSERT_EQ(t.input.size(), decoder.totalRecords()) << "Unmatched # of records.";
  std::string value;
  for (auto& ori_value : t.input) {
    auto b = decoder.nextValue(value);
    ASSERT_TRUE(b) << "Unexpected end of values.";
    EXPECT_EQ(ori_value, value);
  }
  EXPECT_FALSE(decoder.nextValue(value)) << "Too many values than expected.";
  
  for (uint32_t i = 0 ; i < decoder.totalRecords(); ++i) {
    EXPECT_EQ(t.input[i], decoder[i]);
  }

  EXPECT_EQ("", decoder[decoder.totalRecords()]) << "Too many values than expected.";
}

INSTANTIATE_TEST_SUITE_P(stbe, STBETest, ::testing::ValuesIn(tests));

}  // namespace stbe
