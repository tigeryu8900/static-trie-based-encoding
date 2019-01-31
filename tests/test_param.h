// Test parameters
//
#pragma once

#include <vector>
#include <string>

struct TestParam {
  std::vector<std::string> input;
  std::string result;
};

TestParam tests[] = {
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
    },
    { std::vector<std::string>{"abcc"},
      R"(0: 
  1: abcc
)"
    }};

