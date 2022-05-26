# TreeSitterPlusPlus

C++ bindings for [Tree-sitter](https://tree-sitter.github.io/tree-sitter/).

```c++
#include "tree_sitter/cxx/api.h"

using namespace TreeSitter;

int main() {
    Parser parser(Language::C);
    auto tree = parser.parse("int a = 2;");
    auto node = tree.rootNode()
        .firstChild().value();
        .firstChild().value();

    std::cout << node.text() << "\n"; // Should output "int"

    return 0;
}
```

Using it in CMake is as simple as:

```cmake
find_package(Tree-Sitter CONFIG REQUIRED)
add_library(main main.cpp)
target_link_libraries(main PRIVATE Tree-Sitter::Tree-Sitter)
```

## Status

The following languages are built-in:

* C
* C++
* C#
* Go
* Java
* JavaScript
* Python
* Rust
* TypeScript
* TSX

Any language can be added by passing a `TSLanguage` pointer
to `TreeSitter::Language()`.

## Building

### Requirements

TreeSitterPlusPlus has the following system requirements:

* [Git](http://git-scm.com/downloads)
* [CMake](https://cmake.org/install/)

### Compiling

Build like any other CMake project:

    cmake -B build -DENABLE_TESTS=ON
    cmake --build build

### Running Tests

To run tests, run this command after a successful build:

    ctest -C Debug --test-dir build

### Installing

To install, run this:

    cmake --build build --target install

## FAQ

> Can I use the regular C API?

Yes. You can use any of the regular Tree-sitter functions. In fact,
`tree_sitter/api.h` is distributed along with `tree_sitter/cxx/api.h`.
TreeSitterPlusPlus was made to make Tree-sitter easier to use, including
distributing on vcpkg.

> Why the heavy use of `std::optional`?

It makes everything safer. A common problem with ASTs and syntax parsers is
accessing null nodes. Instead of returning pointers, we return regular objects.
Because of RAII, this is much more memory safe than entrusting the user not
to access an invalid node.

Making your code a bit longer is a small price to pay for the extra safety
this provides. `std::optional` is a modern C++17 feature that makes it much
easier to write safer C++; let's use it.

Internally, this is also why classes store all private members inside a
`std::unique_ptr`. Smart pointers are inherently safer.

> Is TreeSitterPlusPlus thread-safe?

Not yet. Classes are not guaranteed to be any more thread-safe than
the regular Tree-sitter functions.

## License

Copyright (c) Alex Shaw.

Licensed under the [MIT License](LICENSE.md).
