#include <iostream>
#include <sstream>
#include <memory>
#include <vector>

#include "gtest/gtest.h"
#include "trie.h"
#include "memblock.h"
#include "test_param.h"

namespace stbe {

class TrieTest : public ::testing::TestWithParam<TestParam> {
};


TEST_P(TrieTest, Serialization)
{
  const TestParam& t = GetParam();
  Trie trie;
  trie.add(t.input);
  std::ostringstream oss;
  oss << trie;
  EXPECT_EQ(t.result, oss.str());
}

TEST_P(TrieTest, Decode)
{
  const TestParam& t = GetParam();
  Trie trie;

  trie.add(t.input);
  std::string buf = trie.serialize();
  BlockDecoder decoder(buf);
  std::string value;
  for (auto& ori_value : t.input) {
    ASSERT_TRUE(decoder.nextValue(value)) << "Unexpected end of values.";
    EXPECT_EQ(ori_value, value);
  }
  EXPECT_FALSE(decoder.nextValue(value)) << "Too many values than expected.";
}

INSTANTIATE_TEST_SUITE_P(Trie, TrieTest, ::testing::ValuesIn(tests));

}  // namespace stbe
