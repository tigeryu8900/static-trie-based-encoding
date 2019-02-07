# Static Trie-based Encoding: A Compressed Read-Only File Format for Fast Reading

This project provides a library that can build a trie data structure from string values and serializes them into files and allows random access to string values compressed in this way.

## Getting Started

These instructions will get you a copy of the project up and running on your local machine for development and testing purposes.

### Prerequisites

#### CMake

Go to the [CMake download page](https://cmake.org/download/) and download CMake.

Read the [tutorial](https://cmake.org/cmake-tutorial/) if you want to contribute.

### Installing

Build the project using this command:

```
cmake -Hpath/to/source -Bpath/to/build
```

`path/to/source` is the path to the source folder (which contains CMakeLists.txt), and `path/to/build` is the path to the build folder. Don't add a space between the paths and the tags.

Go to the build folder and run `make` to make the project.

Now, you can run the executables that are created.

## Running the tests

You can add another test file in the tests folder and add it to CMakeLists.txt (read the [tutorial](https://cmake.org/cmake-tutorial/) to know how) and make the project, or you can run one of the existing tests.

## Examples

These projects uses this project as a library.

### [STBE-based Weblog](../../../stbe-based-weblog)

A library and Tools for encoding Apache weblogs and decoding and reading encoded weblog files (tests use [NASA's weblog files](http://ita.ee.lbl.gov/html/contrib/NASA-HTTP.html))

## Built With

* [Vim](https://www.vim.org/docs.php) - The editor used to write the code
* [CMake](https://cmake.org/documentation/) - The tool used to test and build the code
* [Google Test](https://github.com/google/googletest) - The test framework to test the code

## Versioning

We use [SemVer](http://semver.org/) for versioning. For the versions available, see the [tags on this repository](../../tags) (none so far). 

## Authors

* **Tiger Yu** - *Initial work* - [tigeryu8900](../../..)

See also the list of [contributors](../../graphs/contributors) who participated in this project. (only one so far)

## Acknowledgments

* [Facebook](https://github.com/facebook) for [coding.h](https://github.com/facebook/rocksdb/blob/master/util/coding.h) (slightly modified), [port.h](https://github.com/facebook/rocksdb/blob/master/port/port.x), [port_posix.h](https://github.com/facebook/rocksdb/blob/master/port/port_posix.h), and [port_posix.cc](https://github.com/facebook/rocksdb/blob/master/port/port_posix.cc) (port_posix.h and port_posix.cc are merged into port.h).
* [Google](https://github.com/google) for [Google Test](https://github.com/google/googletest).
