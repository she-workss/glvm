# Contributing

## Adding a New Crate

1. Create directory under `crates/`
2. Add CMakeLists.txt
3. Create `include/`, `src/`, and `tests/` directories
4. Add to root CMakeLists.txt
5. Re-export in `crates/glvm/` if needed

## Adding an Test

1. Create `.cpp` file in `tests/`
2. Add executable to `tests/CMakeLists.txt`

## Adding an Example

1. Create `.cpp` file in `examples/`
2. Add executable to `examples/CMakeLists.txt`

## Adding a Benchmark

1. Create `.cpp` file in `benches/`
2. Add executable to `benches/CMakeLists.txt`
