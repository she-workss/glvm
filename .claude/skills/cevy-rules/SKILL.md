---
name: glvm-rules
allowed-tools: Bash(rg *) PowerShell(rg *) mcp__context7__*
description: "GLVM engine coding conventions — write C++26 that looks like Rust via the rusty crate, per-file namespaces and preludes, Rust naming, Doxygen docs, examples. Use when writing or reviewing any GLVM engine code."
---

# GLVM Rules

Apply these conventions when writing or reviewing any code for the GLVM engine.

## Project Context

Game engine in C++26 using Vulkan and modern C++ features — concepts, constexpr, consteval, ranges, etc. Crate-like structure: `crates/glvm_render`, `crates/glvm_ecs`, etc. Benchmarks in `benches/`, examples in `examples/`, tests in `tests/`.

## 1. The `rusty` crate comes first

Before writing **any** code, check what already exists in `crates/rusty` (`rusty/prelude.hpp` is the entry point) and use it. Never write the raw C++ spelling when a rusty analogue exists:

| C++                                      | Write instead                                                                                                                                              |
| ---------------------------------------- | ---------------------------------------------------------------------------------------------------------------------------------------------------------- |
| `int`, `int32_t`, `uint32_t`, `size_t`…  | `i32`, `u32`, `usize` (also `i8..i64`, `u8..u64`, `isize`, `f32`, `f64`, `f16`, `f128`)                                                                    |
| `float`, `double`                        | `f32`, `f64`                                                                                                                                               |
| `static_cast<To>(val)`                   | `as<To>(val)` or `val \| cast<To>`                                                                                                                         |
| `std::optional<T>`, `std::nullopt`       | `Option<T>`, `None`, `Some(val)`                                                                                                                           |
| `std::expected<T, E>`                    | `Result<T, E>`, `Ok(val)`, `Err(err)`                                                                                                                      |
| `std::vector<T>`                         | `Vec<T>` (or `vec(...)` to build)                                                                                                                          |
| `std::string`, `std::string_view`        | `String`, `str`                                                                                                                                            |
| `std::span<T>`                           | `Slice<T>`                                                                                                                                                 |
| `std::array<T, N>`                       | `Array<T, N>`                                                                                                                                              |
| `std::unordered_map/set`                 | `HashMap<K, V>`, `HashSet<T>`                                                                                                                              |
| `std::map/set`                           | `BTreeMap<K, V>`, `BTreeSet<T>`                                                                                                                            |
| `std::unique_ptr<T>` / `make_unique`     | `Box<T>` / `make_box<T>(...)`                                                                                                                              |
| `std::shared_ptr<T>` / `make_shared`     | `Rc<T>` / `make_rc<T>(...)`                                                                                                                                |
| `std::weak_ptr<T>`                       | `Weak<T>`                                                                                                                                                  |
| `std::variant<Ts...>` + `std::visit`     | `Enum<Ts...>` + `match(var, [](A a) {...}, ...)`                                                                                                           |
| `std::monostate`                         | `Unit`                                                                                                                                                     |
| `std::deque<T>`                          | `VecDeque<T>`                                                                                                                                              |
| `std::mutex`, `std::lock_guard`          | `Mutex`, `MutexGuard<Mutex>`                                                                                                                               |
| `std::atomic_*`                          | `AtomicI32`, `AtomicUsize`, `AtomicBool`, …                                                                                                                |
| `std::cout` / `std::print`               | `rsprintln("...")`, `rsprint(...)`, `rseprintln(...)` (stderr)                                                                                             |
| `throw` / `assert`                       | `panic("...")`, `assert_eq(a, b)`, `assert_ne(a, b)`                                                                                                       |
| `// TODO` / `assert(false)`              | `todo("...")`, `unimplemented("...")`, `unreachable_safe()`                                                                                                |
| early error return                       | `TRY(expr)` (the `?` operator)                                                                                                                             |
| debug print of a value                   | `dbg_rs(expr)` (prints expression, file:line, value)                                                                                                       |
| `std::formattable`, `std::copy_constructible`, `std::equality_comparable`, … | `Display`, `Clone`, `PartialEq`, `Eq`, `PartialOrd`, `Ord`, `Hash`, `Into`, `From`, `Fn`, `FnMut`, `Iterator` concepts |

Fall back to plain `std::` only for things rusty does not cover. If you repeatedly need a missing Rust analogue, add it to `crates/rusty` first, then use it — do not create local typedefs in engine crates.

After writing code, verify no raw standard type leaked in instead of a rusty alias (empty output = pass):

```sh
rg -n --glob "!crates/rusty/include/rusty/types.hpp" --glob "!crates/glvm/include/glvm/glm_compat.hpp" --glob "!crates/rusty/include/rusty/**" --glob "!*.slang" --glob "!*.cmake" "\bstd::(int8_t|int16_t|int32_t|int64_t|uint8_t|uint16_t|uint32_t|uint64_t|size_t|ptrdiff_t|float16_t|float32_t|float128_t|string|string_view|vector|span|array|unique_ptr|shared_ptr|weak_ptr|unordered_map|unordered_set|map|set|byte|nullopt|optional|expected|variant|monostate|mutex|lock_guard|atomic\w+|deque)\b|\b(char8_t|char16_t|char32_t|wchar_t|char|short|int|long|float|double|signed|unsigned|size_t|ssize_t|ptrdiff_t|intmax_t|uintmax_t|intptr_t|uintptr_t|u?int(8|16|32|64)_t|u?intleast(8|16|32|64)_t|u?intfast(8|16|32|64)_t|float(16|32|64|128)_t)\b" crates examples benches tests | rg -v "^[^:]+:\d+:\s*(//|/\*)"
```

`Option`/`Result` are `[[nodiscard]]` and support `unwrap()`, `expect()`, `unwrap_or()`, `unwrap_or_else()`, `map()`, `map_err()`, `and_then()`, `or_else()`, `is_ok()/is_some()`, `take()`, `ok()/err()`, and `operator bool` (`if (opt) { ... }`).

## 2. Namespaces: one module per file

A crate mirrors a Rust crate; every header is a module.

- `crates/glvm_math/include/glvm_math/lib.hpp` — crate root: `namespace glvm_math {}` and includes of all modules.
- Every other file is a module named after the file: `math.hpp` → `namespace glvm_math::math { ... }`, `primitives.hpp` → `namespace glvm_math::primitives`.
- `prelude.hpp` re-exports the public API with explicit `using` declarations (one per line, sorted alphabetically), never wildcard `using namespace` for crate modules:

```cpp
namespace glvm_math::prelude {
using glvm_math::math::Vec2;
using glvm_math::primitives::Rectangle;
} // namespace glvm_math::prelude
```

- At the top of a module namespace, bring in preludes via `using namespace` — `using namespace rusty::prelude;` for rusty aliases (as in `glvm_color/hsla.hpp`) and `using namespace glvm_foo::prelude;` for cross-crate imports. Explicit `using glvm_foo::bar::Baz;` is only for `prelude.hpp` re-exports, never inside module headers.
- `#pragma once` instead of include guards. Include order: own crate headers, then `rusty/*`, then std.
- Public entry point of the whole engine: `glvm/prelude.hpp`; examples and apps start with `using namespace glvm::prelude;`.

## 3. Naming

The source of truth is `.clang-tidy` (`readability-identifier-naming`) — it enforces Rust rules and CI fails on violations:

- Types, enums, enum constants, concepts, template parameters — `CamelCase` (`Camera2d`, not `Camera2D`).
- Functions, methods, variables, members, parameters, namespaces — `snake_case`. No `m_`, `s_`, `g_` prefixes.
- Getters and setters — `get_*` / `set_*` (`get_position()`, `set_position()`), not bare field names: C++, unlike Rust, cannot have a method with the same name as a field.
- `constexpr` globals, static and global constants — `UPPER_CASE`.
- Whitelisted Rust-style exceptions (see `.clang-tidy`): functions `Some`, `Ok`, `Err`; constexpr variables `None`, `cast`.
- Always trailing return type: `auto setup(Commands commands) -> void`. Never `void setup(...)`. Lambdas too: `[](i32 x) -> auto { ... }`, with a concrete return type only when `auto` cannot be deduced.
- Always `enum struct`, never plain `enum` or `enum class`.
- Files in `snake_case` named after their module: `mesh_renderer.hpp`, `transform_bundle.hpp`.

## 4. Doxygen documentation

Every new function, method, and public field gets a `///` Doxygen comment — the first line is the summary (like a Rust doc comment), then a blank `///` line, then `@param` / `@return` / `@tparam`:

```cpp
/// Adds a mesh to the collection and returns its handle.
///
/// @param mesh The mesh to add.
/// @return A handle referencing the added mesh.
auto add(Mesh mesh) -> Handle<Mesh>;
```

```cpp
/// The hue in degrees.
f32 hue = 0.0f;
```

- Comments in English only, minimal, no obvious things, no decoration (`// ── Foo ──` → `// Foo`).
- In code comments use only ASCII hyphens `-` as dashes. Never Unicode en dash `–` or em dash `—`.

## 5. Formatting

- No blank lines inside function bodies — code flows as a solid block:

```cpp
auto f() -> void {
    auto x = ...;
    x.spawn(Camera2d());
}
```

- Unit structs and empty constructor calls with parentheses, not braces: `Camera2d()`, never `Camera2d {}`.
- Braces `{}` only for designated initializers of structs with fields: `Mesh2d{.handle = h}`. Never `T{}` as a constructor call.
- Each variable definition on its own line (no comma-separated declarations). Always use `auto` for variable declarations wherever possible; spell out the type only when `auto` is impossible (no initializer, deduced type differs from the desired one).
- Even a one-line `if`/`for`/`while`/`else` (or any other block) always gets braces, and the single statement goes on its own line — never on the same line as the condition:

```cpp
if (something) {
    do_it();
}
```
- Prefer Rust-style method chaining (`map`, `and_then`, ranges/views) over nested imperative code.

## 6. Factories and construction

- Static factory that constructs a type: `create()`, never `new_()` or `new()` — `App::create()`.
- Constructor parameters mirror the Rust equivalent's arguments.

## 7. Types and ownership

- `struct` for data (components, resources, bundles). Never use `class`.
- Traits → Concepts by default, but pure-virtual interface structs are acceptable when a concept is a poor fit (runtime dispatch, heterogeneous collections, plugin boundaries). Use virtual interfaces deliberately, not habitually.
- Variables `const` by default; mutability only when truly needed (as in Rust). Prefer `constexpr`/`consteval` wherever a value or computation can be compile-time.
- Global-scope variables (outside any struct/function) are immutable, like Rust's `const`/`static`: always `inline constexpr`/`const`. Never create mutable global state.
- Never raw `new`/`delete` — `make_box` (≈ `Box`), `make_rc` (≈ `Arc`), RAII destructors (≈ `Drop`).
- Prefer move semantics (`std::move`) — as Rust moves by default.
- Designated initializers for structs with fields: `RenderConfig{.width = 1920, .height = 1080}`.

## 8. Errors — always the Rust way

- Error handling is always Rust-style: `Result`, `Option`, `TRY`. Never throw or catch exceptions — not even once, for any reason. Absence of a value is `Option<T>`, failure is `Result<T, E>`, never a `throw` and never a sentinel value.
- No exceptions for control flow. Mark functions `noexcept` where possible.
- Both `Option` and `Result` are `[[nodiscard]]` — errors must never be silently dropped (as Rust warns on unused `Result`).
- Propagate with `TRY(expr)`; abort on invariant violation with `panic("...")`; placeholder code with `todo()`/`unimplemented()`; impossible branches with `unreachable_safe()`.
- `static_assert` to verify invariants at compile time.

## 9. C++26 Features

- Use maximally: concepts, constexpr, consteval, ranges/views, std::format, structured bindings, `if constexpr`, fold expressions, pack indexing, etc.
- `[[nodiscard]]` on functions whose return value must not be ignored.
