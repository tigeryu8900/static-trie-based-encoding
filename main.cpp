#include <iostream>
#include <memory>
#include <vector>

#include "trie.h"

int main() {
  {
    Trie t;
    t.add(std::vector<std::string>{"abc", "abd"});
    std::cout << t;
  }
  {
    Trie t;
    t.add(std::vector<std::string>{"a", "aa", "abc", "aabc", "abcc"});
    std::cout << t;
  }
}
