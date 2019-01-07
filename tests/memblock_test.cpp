#include <iostream>
#include <memory>
#include <vector>

#include "gtest/gtest.h"
#include "../trie.h"
#include "../memblock.h"

namespace memblock {

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
    Trie trie;
    trie.add(t.input);
    std::cout << trie;

    std::string buf = trie.serialize();
    std::cout << "Serialized size: " << buf.size() << std::endl;
    std::cout << "Decoded values:" << std::endl;
    Decoder decoder(buf);
    std::string value;
    while (decoder.nextValue(value)) {
      std::cout << value << std::endl;
    }
  }
}

}  // namespace memblock
