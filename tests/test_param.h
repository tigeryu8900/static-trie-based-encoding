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
     { {},  // no values
      R"(0: 
)"
     },
     { {""},  // single empty value
      R"(0: 
  1: 
)"
     },
     { {"", "", ""},  // multiple empty values
      R"(0: 
  1: 
)"
     },
     { {"abcc"},  // single value
      R"(0: 
  1: abcc
)"
    },
     { {"abcc", "abcc", "abcc"},  // duplicated values
      R"(0: 
  1: abcc
)"
    },
    { {"abc", "abd"},  // two values with common prefix
      R"(0: 
  1: ab
    2: c
    2: d
)"
    },
    { {"abc", "akk"},  // two values with common prefix less than threshold
      R"(0: 
  1: abc
  1: akk
)"
    },
    { {"abc", "def"},  // two values with no common prefix
      R"(0: 
  1: abc
  1: def
)"
    },
    { {"a", "aa", "abc", "aabc", "abcc"},  // multi-values
      R"(0: 
  1: a
    2: a
      3: bc
    2: bc
      3: c
)"
    },
    { {"a", "aa", "abc", "aabc", "", "abcc"},  // multi-values with empty string
      R"(0: 
  1: a
    2: a
      3: bc
    2: bc
      3: c
  1: 
)"
    },
   { {"128.217.62.224", "128.217.62.24", "128.217.62.2"},
      R"(0: 
  1: 128.217.62.2
    2: 24
    2: 4
)"
    }};

