# Architecture

The project follows Rust's crate pattern:
- Each major subsystem is a separate "crate" (CMake library)
- The `glvm` crate re-exports everything via namespace aliases
- Examples are self-contained executables
- Benchmarks measure performance of individual components
