# fet

**C++ Functional Extensions Library** - A LINQ-like library for C++ with pipe operator syntax

[![License: MPL 2.0](https://img.shields.io/badge/License-MPL%202.0-brightgreen.svg)](https://opensource.org/licenses/MPL-2.0)

## Overview

`fet` (Functional Extensions Library) is a header-only C++ library that provides LINQ-like functionality with a pipe operator syntax. It enables functional programming patterns for data processing pipelines using three core concepts:

- **Source** (`ISource`): Data generators (similar to LINQ's `IEnumerable`)
- **Gate** (`IGate`): Data transformers and filters (similar to LINQ's `Where`, `Select`)  
- **Drain** (`IDrain`): Data consumers and aggregators (similar to LINQ's `ToList`, `Aggregate`)

### Basic Syntax

```cpp
auto result = source | gate | gate | ... | drain;
```

### Type Combination Rules

- `source | gate` → `source`
- `gate | gate` → `gate`  
- `gate | drain` → `drain`
- `source | drain` → `decltype(drain.OnComplete())`

## Requirements

- C++14 compatible compiler
- Header-only library - no compilation required

## Installation

Since `fet` is a header-only library, simply include the headers in your project:

```cpp
#include "fet/core.hpp"
#include "fet/source/container_source.hpp"
#include "fet/gate/filter.hpp"
#include "fet/gate/transform.hpp"
#include "fet/drain/to_container.hpp"
// ... other components as needed
```

## Quick Start

### Basic Example

```cpp
#include <vector>
#include <iostream>
#include "fet/core.hpp"
#include "fet/source/container_source.hpp"
#include "fet/gate/filter.hpp"
#include "fet/gate/transform.hpp"
#include "fet/drain/to_container.hpp"

using namespace fet;

int main() {
    std::vector<int> numbers = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    
    auto result = from_container(numbers)
        | filter([](int x) { return x % 2 == 0; })  // Filter even numbers
        | transform([](int x) { return x * x; })     // Square each number
        | to_vector();                               // Collect to vector
    
    // result contains: {4, 16, 36, 64, 100}
    for (int value : result) {
        std::cout << value << " ";
    }
    return 0;
}
```

## Core Components

### Sources

Sources generate or provide data to the pipeline:

#### Container Source
```cpp
#include "fet/source/container_source.hpp"

std::vector<int> data = {1, 2, 3, 4, 5};
auto source = from_container(data);  // Creates a source from any container
```

#### Enumerator Source
```cpp
#include "fet/source/enumerator_source.hpp"

// Generate sequence of numbers
auto numbers = from_enumerator([](size_t i) { return i * 2; }, 5);  // {0, 2, 4, 6, 8}
```

### Gates (Transformers)

Gates transform, filter, or manipulate data as it flows through the pipeline:

#### Filter
```cpp
#include "fet/gate/filter.hpp"

// Filter elements based on a predicate
auto evens = filter([](int x) { return x % 2 == 0; });

// Filter out null pointers
auto nonNull = filterNull();

// Filter specific tuple element
auto nonNullFirst = filterNull<0>();  // Filters std::get<0> != nullptr
```

#### Transform
```cpp
#include "fet/gate/transform.hpp"

// Transform elements using a function
auto squares = transform([](int x) { return x * x; });

// Transform to different type
auto strings = transform([](int x) { return std::to_string(x); });
```

#### Flat Map
```cpp
#include "fet/gate/flat_map.hpp"

// Flatten nested containers
auto words = std::vector<std::string>{"hello", "world"};
auto chars = from_container(words)
    | flat_map([](const std::string& s) { return s; })  // Flatten to characters
    | to_vector();
```

### Drains (Consumers)

Drains consume the data and produce final results:

#### To Container
```cpp
#include "fet/drain/to_container.hpp"

// Collect to std::vector
auto vec = source | to_vector();

// Collect with transformation
auto strings = source | to_vector([](int x) { return std::to_string(x); });
```

#### Accumulate
```cpp
#include "fet/drain/accumulate.hpp"

// Sum all elements
auto sum = source | accumulate(0, [](int acc, int x) { return acc + x; });

// Custom accumulation
auto product = source | accumulate(1, std::multiplies<int>{});
```

#### Multiplexer
```cpp
#include "fet/drain/multiplexer.hpp"
// Split data to multiple destinations
```

## Advanced Usage

### Chaining Multiple Operations

```cpp
auto result = from_container(data)
    | filter([](int x) { return x > 0; })        // Keep positive numbers
    | transform([](int x) { return x * 2; })      // Double them
    | filter([](int x) { return x < 20; })       // Keep values < 20
    | transform([](int x) { return x + 1; })      // Add 1
    | to_vector();                               // Collect results
```

### Working with Custom Types

```cpp
struct Person {
    std::string name;
    int age;
};

std::vector<Person> people = {
    {"Alice", 30}, {"Bob", 25}, {"Charlie", 35}
};

auto names = from_container(people)
    | filter([](const Person& p) { return p.age >= 30; })
    | transform([](const Person& p) { return p.name; })
    | to_vector();
```

### Performance Considerations

- The library uses perfect forwarding and move semantics where possible
- Operations are lazy and don't create intermediate containers
- Memory is pre-allocated when container sizes are known

## API Reference

### Core Classes

- `ISource`: Base class for data sources
- `IGate`: Base class for data transformers
- `IDrain`: Base class for data consumers
- `SourceInfo<T>`: Contains metadata about data sources

### Type Traits

- `is_src<T>`: Check if type is a source
- `is_gate<T>`: Check if type is a gate  
- `is_drain<T>`: Check if type is a drain
- `is_jct<T>`: Check if type is a junction (gate or drain)

## License

This project is licensed under the Mozilla Public License 2.0 - see the [LICENSE](LICENSE) file for details.

## Contributing

Contributions are welcome! Please feel free to submit pull requests or open issues for bugs and feature requests.

## Compatibility

- Designed for C++14 and later
- Header-only implementation
- Compatible with GCC, Clang, and MSVC compilers
