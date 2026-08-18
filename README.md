# Centipede - A C++ implementation of the Millepede method

[![dashboard](https://img.shields.io/badge/dashboard-centiepede-blue?labelColor=gray&style=flat)](https://my.cdash.org/index.php?project=centipede-projekt)
[![Codacy Badge](https://app.codacy.com/project/badge/Grade/dcd6886516a040d885721aabfed658f3)](https://app.codacy.com/gh/YanzhaoW/centipede.cpp/dashboard?utm_source=gh&utm_medium=referral&utm_content=&utm_campaign=Badge_grade)
[![codecov](https://codecov.io/gh/YanzhaoW/centipede.cpp/graph/badge.svg?token=CTY7CA0IKH)](https://codecov.io/gh/YanzhaoW/centipede.cpp)
[![CI](https://github.com/YanzhaoW/centipede.cpp/actions/workflows/ci.yml/badge.svg?branch=master)](https://github.com/YanzhaoW/centipede.cpp/actions/workflows/ci.yml)
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)

## Documentation

Please see [the official documentation site](https://yanzhaow.github.io/centipede.cpp/) for usages and references.

## Download

To download the project:

```bash
git clone https://github.com/YanzhaoW/centipede.cpp.git centipede
```

## Requirements

- Compilers:
  - gcc: >= 15
  - clang: >= 21
- CMake
- Conan

## CLI configuration

```bash
centipede -c config.lua
```
Inside `config.lua`, the default settings are:

```lua
require 'centipede'.setup
{
    num_of_runs = 1,
    max_num_of_events = 0,

    input = {
        data_filename = "",
        init_par = {
            filename = "",
            id = "",
            value = "",
        },
    },

    output = {
        par_filename = "",
        only_last_run = true,
    },

    engine = {
        chi2_factor = 50.,
        n_globals = 0,
        fixed_parameter_ids = {},
        alpha = 0.027,
    },

    hooks = {
        post_entrypoint_read = nil,
    },
}
```

## References

- [Millepede-II program](https://www.desy.de/~kleinwrt/MP2/doc/html/)
