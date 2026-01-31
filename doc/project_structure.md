# Project structure {#project_structure}

[TOC]

## Infrastructure

There are 5 major components regarding the infrastructure of this project:

- Documentation
- Continuous integration
- Continuous deployment
- Testing
- Benchmark

### Documentation

Documentation in this project is done via Doxygen, a very popular documentation tool for C and C++ projects. It is then modified by [Doxygen-awesome](https://github.com/jothepro/doxygen-awesome-css) to improve the aesthetic of the documentation. The main configuration file for the doxygen is in this [CMakeLists.txt](https://github.com/YanzhaoW/centipede.cpp/blob/master/doc/CMakeLists.txt), where all CMake variables with the prefix `DOXYGEN_` will be used as the doxygen configuration options. To build the documentation, enable `-DBUILD_DOC` during the CMake configuration and build the target named "doxygen":

```bash
$ cmake --preset default -DBUILD_DOC=ON

$ cd build

$ ninja doxygen
```

The `index.html` will be in `build/doc/html` folder.

### Continuous integration (CI)

### Continuous deployment (CD)

### Testing

All testing files are in the `test` folder. Generally, testings can be categorized into _unit tests_ and the _integration tests_ (there are [other types of testing](https://www.atlassian.com/continuous-delivery/software-testing/types-of-software-testing)). Unit tests are meant to test each public API of a class while integration tests are meant to test whether all components of the software work well. In this project, unit tests are done via googletest and integration tests are done via ctest (from CMake).

#### Unit tests

To add a new unit test, first create a new `.cpp` file in `test/unit_tests` folder. Create a test function with a test suite name and a test name, e.g.:

```cpp
#include <gtest/gtest.h>

TEST(test_suite_name, test_name) 
{ 
    // doing tests here
}
```

### Benchmark

Not yet implemented ...
