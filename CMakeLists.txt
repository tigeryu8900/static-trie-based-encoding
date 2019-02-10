cmake_minimum_required(VERSION 2.8.2)

# Download and unpack googletest at configure time
configure_file(CMakeLists.txt.in googletest-download/CMakeLists.txt)
execute_process(COMMAND ${CMAKE_COMMAND} -G "${CMAKE_GENERATOR}" .
  RESULT_VARIABLE result
  WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/googletest-download )
if(result)
  message(FATAL_ERROR "CMake step for googletest failed: ${result}")
endif()
execute_process(COMMAND ${CMAKE_COMMAND} --build .
  RESULT_VARIABLE result
  WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/googletest-download )
if(result)
  message(FATAL_ERROR "Build step for googletest failed: ${result}")
endif()

# Prevent overriding the parent project's compiler/linker
# settings on Windows
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

# Add googletest directly to our build. This defines
# the gtest and gtest_main targets.
add_subdirectory(${CMAKE_CURRENT_BINARY_DIR}/googletest-src
                 ${CMAKE_CURRENT_BINARY_DIR}/googletest-build
                 EXCLUDE_FROM_ALL)

# The gtest/gtest_main targets carry header search path
# dependencies automatically when using CMake 2.8.11 or
# later. Otherwise we have to add them here ourselves.
if (CMAKE_VERSION VERSION_LESS 2.8.11)
  include_directories("${gtest_SOURCE_DIR}/include")
endif()

add_library(stbe
	src/util/coding.cpp
	src/trie.cpp)

# Now simply link against gtest or gtest_main as needed. Eg

# Setup testing
enable_testing()
#find_package(GTest REQUIRED)
include_directories("${gtest_SOURCE_DIR}/include" include src)

# Add test cpp file
add_executable(memblock_test
    tests/memblock_test.cpp
)
target_link_libraries(memblock_test stbe gtest gtest_main)
message( "   tigerdebug ${gtest}" )

add_test(NAME memblock_test COMMAND memblock_test)

add_executable(stbe_test
  tests/stbe_test.cpp
)
target_link_libraries(stbe_test stbe gtest gtest_main)
add_test(NAME stbe_test COMMAND stbe_test)

add_executable(custom_struct_test
  tests/custom_struct_test.cpp
)
target_link_libraries(custom_struct_test stbe gtest gtest_main)
add_test(NAME custom_struct_test COMMAND custom_struct_test)

