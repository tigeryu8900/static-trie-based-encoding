#include <iostream>
#include <memory>
#include <vector>

#include "gtest/gtest.h"
#include "../trie.h"
#include "../stbe.h"

namespace stbe {

TEST(trie_case, sample_test)
{
  struct {
    std::vector<std::string> input;
    std::string result;
  } tests[] = {
    { std::vector<std::string>{"abc", "abd"},
      R"(0: 
  1: ab
    2: c
    2: d
)"
    },
    { std::vector<std::string>{"a", "aa", "abc", "aabc", "abcc"},
      R"(0: 
  1: a
  1: aa
    2: bc
  1: abc
    2: c
)"
    }
  };
  for (auto& t : tests) {
    Builder builder("test_file", t.input);

    std::cout << "Decoded values:" << std::endl;
    Decoder decoder("test_file");
    std::string value;
    int i = 0;
    while (decoder.nextValue(value)) {
      std::cout << value << std::endl;
    }
  }
}

}  // namespace stbe
