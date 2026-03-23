# Testing Guide

Comprehensive testing strategy for GLVM C++, mirroring Rust's testing approach.

## Test Organization

```
glvm/
├── crates/
│   ├── glvm_ecs/
│   │   └── tests/   # Unit tests for glvm_ecs
│   ├── glvm_render/
│   │   └── tests/   # Unit tests for glvm_render
│   └── ...
└── tests/           # Integration tests (cross-crate)
    └── basic.cpp
```

## Test Types

### 1. Unit Tests (crates/*/tests/)

Test individual crate functionality in isolation.

**Location:** Inside each crate's `tests/` directory
**Purpose:** Test internal implementation details
**Dependencies:** Only the crate being tested + GTest

**Example:**
```cpp
// crates/glvm_ecs/tests/test_main.cpp
#include <gtest/gtest.h>
#include "glvm_ecs/lib.hpp"

TEST(GlvmEcsTest, InitializationTest) {
    EXPECT_TRUE(true);
}
```

### 2. Integration Tests (tests/)

Test how multiple crates work together.

**Location:** Root `tests/` directory
**Purpose:** Test public APIs and crate interactions
**Dependencies:** Multiple crates + GTest

**Example:**
```cpp
// tests/ecs_render_test.cpp
#include <gtest/gtest.h>
#include "glvm_ecs/lib.hpp"
#include "glvm_render/lib.hpp"

TEST(IntegrationEcsRender, InitializeBothSystems) {
    glvm_render::init_render();
    EXPECT_TRUE(true);
}
```

## Running Tests

### All Tests
```bash
# Build and run all tests
cmake --build build
ctest --test-dir build --output-on-failure
```

### Verbose Output
```bash
ctest --test-dir build --verbose
```

### Specific Test Suite
```bash
# Run only unit tests
./build/crates/glvm_ecs/glvm_ecs_tests
./build/crates/glvm_render/glvm_render_tests

# Run only integration tests
./build/tests/integration_ecs_types
./build/tests/integration_ecs_render
./build/tests/integration_full_glvm
```

### With GTest Filters
```bash
# Run specific test
./build/tests/integration_full_glvm --gtest_filter=IntegrationFullGlvm.PreludeImportsTypes

# Run tests matching pattern
./build/tests/integration_full_glvm --gtest_filter=*Prelude*
```

### Parallel Testing
```bash
# Run tests in parallel
ctest --test-dir build -j8
```

## Writing Integration Tests

### Test Structure

```cpp
#include <gtest/gtest.h>
#include "glvm/prelude.hpp"  // Or specific crates

using namespace glvm::prelude;

TEST(TestSuiteName, TestName) {
    // Arrange
    i32 value = 42;

    // Act
    some_fn();

    // Assert
    EXPECT_EQ(value, 42);
}
```

### Feature-Conditional Tests

```cpp
#ifdef GLVM_FEATURE_RENDER
TEST(Integration, RenderFeatureTest) {
    init_render();
    EXPECT_TRUE(true);
}
#else
TEST(Integration, RenderFeatureTest) {
    GTEST_SKIP() << "Render feature disabled";
}
#endif
```

### Complex Scenarios

```cpp
TEST(Integration, GameLoopSimulation) {
    // Setup
    some_fn();

#ifdef GLVM_FEATURE_RENDER
    init_render();
#endif

    // Simulate game state
    struct GameState {
        i32 frame_count = 0;
        Vec<String> systems_active;
    };

    GameState state;
    state.systems_active.push_back("ECS");

#ifdef GLVM_FEATURE_RENDER
    state.systems_active.push_back("Render");
#endif

    // Verify
    EXPECT_GE(state.systems_active.size(), 1);
}
```

## Test Patterns

### Testing Option Types
```cpp
TEST(TypesTest, OptionHandling) {
    Option<i32> some_value = 42;
    Option<i32> no_value = None;

    ASSERT_TRUE(some_value.has_value());
    EXPECT_EQ(some_value.value(), 42);

    EXPECT_FALSE(no_value.has_value());
}
```

### Testing Result Types
```cpp
TEST(TypesTest, ResultHandling) {
    auto divide = [](i32 a, i32 b) -> Result<i32, String> {
        if (b == 0) {
            return std::unexpected(String("Division by zero"));
        }
        return a / b;
    };

    auto success = divide(10, 2);
    ASSERT_TRUE(success.has_value());
    EXPECT_EQ(success.value(), 5);

    auto failure = divide(10, 0);
    ASSERT_FALSE(failure.has_value());
    EXPECT_EQ(failure.error(), "Division by zero");
}
```

### Testing Smart Pointers
```cpp
TEST(TypesTest, SmartPointers) {
    Box<i32> unique = std::make_unique<i32>(42);
    Rc<String> shared = std::make_shared<String>("data");

    EXPECT_EQ(*unique, 42);
    EXPECT_EQ(*shared, "data");
    EXPECT_EQ(shared.use_count(), 1);
}
```

### Testing Containers
```cpp
TEST(TypesTest, Containers) {
    Vec<i32> numbers = {1, 2, 3, 4, 5};
    Array<f32, 3> coords = {1.0f, 2.0f, 3.0f};

    EXPECT_EQ(numbers.size(), 5);
    EXPECT_EQ(coords.size(), 3);

    for (usize i = 0; i < numbers.size(); ++i) {
        EXPECT_EQ(numbers[i], static_cast<i32>(i + 1));
    }
}
```

## Adding New Tests

### 1. Create Test File

```cpp
// tests/my_integration_test.cpp
#include <gtest/gtest.h>
#include "glvm/prelude.hpp"

TEST(MyIntegration, BasicTest) {
    // Your test here
    EXPECT_TRUE(true);
}
```

### 2. Add to CMakeLists.txt

```cmake
# tests/CMakeLists.txt
add_executable(integration_my_test
    my_integration_test.cpp
)

target_link_libraries(integration_my_test
    PRIVATE
        glvm
        GTest::gtest_main
)

add_test(NAME integration_my_test COMMAND integration_my_test)
```

### 3. Build and Run

```bash
cmake --build build
./build/tests/integration_my_test
```

## Test Coverage

While C++ doesn't have built-in coverage like Rust's `cargo tarpaulin`, you can use:

### GCC/Clang Coverage

```bash
# Configure with coverage flags
cmake -B build -DCMAKE_CXX_FLAGS="--coverage"

# Build and run tests
cmake --build build
ctest --test-dir build

# Generate coverage report
gcov build/tests/*.gcda
lcov --capture --directory build --output-file coverage.info
genhtml coverage.info --output-directory coverage_html
```

### 3. Use Fixtures for Complex Setup
```cpp
class GameStateTest : public ::testing::Test {
protected:
    void SetUp() override {
        some_fn();
#ifdef GLVM_FEATURE_RENDER
        init_render();
#endif
    }

    void TearDown() override {
        // Cleanup if needed
    }
};

TEST_F(GameStateTest, StateManagement) {
    // Test using the setup from SetUp()
    EXPECT_TRUE(true);
}
```

## Continuous Integration

Example GitHub Actions workflow:

```yaml
name: Tests

on: [push, pull_request]

jobs:
  test:
    runs-on: ubuntu-latest
    steps:
      - uses: actions/checkout@v3

      - name: Configure
        run: cmake -B build

      - name: Build
        run: cmake --build build

      - name: Test
        run: ctest --test-dir build --output-on-failure
```
