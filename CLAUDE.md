# CLAUDE.md

Guidance for agents when working in the `glvm` repository.

## Project

Game engine in C++26 using Vulkan.

## Project structure

- [crates/](crates/) - libs (like crates in Rust).
- [benches/](benches/) - benchmarks
- [examples/](examples/) - code examples
- [tests/](tests/) - tests

## Building and running

`cmake --workflow d` - ONLY for a clean build (after deleting `build/`), after adding a new crate, or after adding a new dependency to CPM. In all other cases use the direct commands below, depending on what changed (code, tests, benchmarks, or docs):

- `cmake --preset debug` - configure once
- `cmake --build --preset debug` - build all
- `ctest --test-dir build --output-on-failure` - run all tests (also refer to [TESTING.md](TESTING.md) for comprehensive guide)
- `cmake --build --preset release -t bench_ecs` - run a specific benchmark from [benches/](benches/) folder by target name (e.g. `ecs`)
- `cmake --build --preset debug -t docs` - generate docs (and then access them via `./build/debug/docs/html/index.html`)
- `cmake --build --preset debug -t run_snake` - run a specific example from [examples/](examples/) folder by target name (e.g. `snake`)

## Verification after code changes

After writing, reviewing, or refactoring any code, always verify that compilation does not fail before finishing: `cmake --build --preset debug` (plus `ctest --test-dir build --output-on-failure` when tests are affected). Do not leave the workspace in a non-compiling state.

## Coding conventions

Strict GLVM coding conventions - Rust-style naming, type aliases, ownership rules, C++26 idioms - apply to **all** code written or reviewed in this repository. They are defined in `.claude/skills/glvm-rules/SKILL.md` and available as the `/glvm-rules` skill.

Read `.claude/skills/glvm-rules/SKILL.md` and follow it whenever writing or reviewing code.
