# Programming styles and conventions {#program_style_conventions}

[TOC]

## Using test driven development (TDD)

All new code added to this project must be tested via unit tests. The recommended workflow for the development of this project is to define the interfaces without implementation, add the tests and then write the implementation such that the all written tests pass.

Tests written for a class, should only be oriented towards its public interfaces (or public member functions). The private methods, which are eventually called by those public interfaces do not need to be tested.

The test coverage, which is automatically calculated from one of the CI pipelines, should be 100\%.

## Naming convention

|        Categories        | Conventions                 | Examples          |
| :----------------------: | :-------------------------- | :---------------- |
|          class           | PascalCase                  | `MyClass`         |
|         concept          | PascalCase                  | `MyConcept`       |
|    (member) function     | snake_case                  | `my_function`     |
| (struct member) variable | snake_case                  | `my_variable`     |
|  class member variable   | snake*case\_ (trailing `*`) | `member_var_`     |
|        file name         | snake_case                  | `source_file.cpp` |
|        namespace         | snake_case                  | `centipede`       |

## Following the best practice

Following the best practice of C++ is the best way to make sure the program is reliable, fast and easy to maintain in the long term. During the CI process, all code in the program is automatically checked by the static analyzers, namely clang-tidy and clang-format, and the pull request can only be accepted when these static analyzers emit no errors or warnings. For an easier recognition of some bad practice, it's highly recommended to utilize Clangd in your IDE during the development.

Following standards are used in this project. Some of them are automatically enforced from clang-tidy:

### Explicitly specifying ownerships

1. <span style="color:red">**NO NEW AND DELETE !!!!**</span>

2. Use `unique_ptr` for most (99%) of the cases. In some rare situations where resources are shared between multiple threads, use `share_ptr`.

### Curly brackets for the initialization

_example_:

```cpp
auto vec = std::vector<double>{};
```

### Always auto

Always use auto for variable and function declarations, except of the member variables:

_variables_:

```cpp
const auto val = int{};

auto obj = MyClass{};
```

_functions_:

```cpp
auto my_func_return_int() -> int;

void my_func_return_void();
```

### OOP related convention

1. Follow the _rule of zero_, or _rule of five_.

2. Don't omit the default constructor. Always write it out explicitly when possible:

   ```cpp
   class MyClass
   {
     public:
       MyClass() = default;
   };
   ```

3. **No dynamic polymorphism**. In other words, no virtual member functions. Instead, use template and _deduce this_ (static polymorphism).

4. No _friend_.

### Parameter passing of functions

1. Pass by copy for input parameters of types whose sizes are less than or equal to 8 bytes. This includes built-in types such as, `int`, `float`, `double`, etc. For example:

   ```cpp
   void my_fun(int val);

   void my_fun(double val);

   void my_fun(uint32_t val);
   ```

2. Pass by const ref for input parameters of types whose sizes are greater than 8 bytes. For example:

   ```cpp
   void my_func(const MyData& input);
   ```

3. Pass by non const ref for output parameter of types whose sizes are greater than 8 bytes. For example:

   ```cpp
   void my_func(MyData& output);
   ```
