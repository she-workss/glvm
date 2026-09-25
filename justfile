# A thin wrapper around the existing CMake presets. CMakePresets.json stays the
# source of truth; this file only selects presets, targets and toolchains.
# Examples and benchmarks are discovered from examples/CMakeLists.txt and
# benches/CMakeLists.txt, so newly added ones need no justfile changes.
#
#   just run hello_world                    # debug preset, default compiler
#   just run --release tps                  # release preset
#   just run --profile=profiling lightning  # any CMake configure preset
#   just run --msvc hello_world             # debug-msvc (activate an MSVC env yourself first)
#   just build --release --clang-cl         # release-clang-cl
#   just test                               # build everything, then run all tests
#   just bench                              # run every bench target (release)
#   just workflow debug-full                # cmake --workflow debug-full
#   just examples                           # list discovered examples
#
# Options: --release/-r, --debug/-d, --profile=<name>, --msvc, --clang-cl,
# --clang, --gcc, --dry-run/-n. MSVC-family presets (--msvc, --clang-cl) need
# an MSVC environment activated by the user beforehand; just never sets one
# up itself.

set shell := ["sh", "-cu"]

alias r := run
alias b := build
alias t := test

# Show available recipes.
default:
    @just --list

# Run an example: `just run hello_world`, `just run --release clay`.
run *args: (_glvm "run" args)

# Build everything; configures the preset first if needed.
build *args: (_glvm "build" args)

# Build everything and run all tests.
test *args: (_glvm "test" args)

# Run benchmarks (release by default; pass --debug to override).
bench *args: (_glvm "bench" args)

# Run a CMake workflow preset: `just workflow debug-full`.
workflow *args: (_glvm "workflow" args)

# List examples discovered from examples/CMakeLists.txt.
examples: (_glvm "examples" "")

[private]
[positional-arguments]
_glvm action rest="":
    #!/usr/bin/env sh
    # `just` passes a variadic argument list to dependencies as one
    # space-separated string; split it back into positional parameters.
    set -efu

    action="$1"
    rest="${2:-}"

    usage() {
      echo "usage: just <run|build|test|bench|workflow|examples> [options] [names...]" >&2
      echo "options: --release/-r --debug/-d --profile=<name> --msvc --clang-cl --clang --gcc --dry-run/-n" >&2
      echo "MSVC-family presets need an MSVC environment activated beforehand; just never sets one up." >&2
    }

    die() { echo "just: $*" >&2; exit 2; }

    examples_list() { sed -n 's/.*glvm_add_example(\([a-z0-9_]*\)).*/\1/p' examples/CMakeLists.txt | sort; }
    benches_list() { sed -n 's/.*glvm_add_bench(\([a-z0-9_]*\)).*/\1/p' benches/CMakeLists.txt | sort; }

    profile=debug
    profile_set=0
    toolchain=
    dry=0
    names=
    expect_profile=0
    for arg in $rest; do
      if [ "$expect_profile" = 1 ]; then
        if [ -z "$arg" ] || [ "${arg#-}" != "$arg" ]; then
          usage; die "--profile requires a preset name"
        fi
        profile="$arg"; profile_set=1; expect_profile=0
        continue
      fi
      case "$arg" in
        -r|--release) profile=release; profile_set=1 ;;
        -d|--debug) profile=debug; profile_set=1 ;;
        --profile=*)
          profile="${arg#--profile=}"
          if [ -z "$profile" ]; then
            usage; die "--profile requires a preset name"
          fi
          profile_set=1 ;;
        --profile) expect_profile=1 ;;
        --msvc|--clang-cl|--clang|--gcc) toolchain="${arg#--}" ;;
        -n|--dry-run) dry=1 ;;
        -h|--help) usage; exit 0 ;;
        -*) usage; die "unknown option '$arg'" ;;
        *) names="${names}${names:+ }${arg}" ;;
      esac
    done
    if [ "$expect_profile" = 1 ]; then
      usage; die "--profile requires a preset name"
    fi

    # Benchmarking without an explicit profile means release.
    if [ "$action" = bench ] && [ "$profile_set" = 0 ]; then
      profile=release
    fi

    preset="$profile"
    if [ -n "$toolchain" ]; then
      preset="$preset-$toolchain"
    fi

    run_cmd() {
      prog="$1"; shift
      cmdline="$prog $*"
      if [ "$dry" = 1 ]; then
        echo "+ $cmdline"
        return 0
      fi
      echo "+ $cmdline" >&2
      "$prog" "$@"
    }

    ensure_configured() {
      if [ ! -f "build/$preset/CMakeCache.txt" ]; then
        run_cmd cmake --preset "$preset"
      fi
    }

    # noglob is on, names are plain tokens; split the collected names.
    set -- $names

    case "$action" in
      run)
        if [ "$#" -eq 0 ]; then
          usage
          echo "examples:" >&2
          examples_list | sed 's/^/  /' >&2
          exit 2
        fi
        if [ "$#" -ne 1 ]; then
          die "run takes exactly one example name (got: $*)"
        fi
        name="$1"
        if ! examples_list | grep -qx "$name"; then
          echo "just: unknown example '$name', available:" >&2
          examples_list | sed 's/^/  /' >&2
          exit 2
        fi
        ensure_configured
        run_cmd cmake --build --preset "$preset" -t "run_$name"
        ;;
      build)
        if [ "$#" -ne 0 ]; then
          die "build takes no names (got: $*)"
        fi
        ensure_configured
        run_cmd cmake --build --preset "$preset"
        ;;
      test)
        if [ "$#" -ne 0 ]; then
          die "test takes no names (got: $*)"
        fi
        ensure_configured
        run_cmd cmake --build --preset "$preset"
        run_cmd ctest --test-dir "build/$preset" --output-on-failure
        ;;
      bench)
        if [ "$#" -eq 0 ]; then
          set -- $(benches_list)
        fi
        ensure_configured
        for name in "$@"; do
          if ! benches_list | grep -qx "$name"; then
            echo "just: unknown benchmark '$name', available:" >&2
            benches_list | sed 's/^/  /' >&2
            exit 2
          fi
          run_cmd cmake --build --preset "$preset" -t "bench_$name"
        done
        ;;
      workflow)
        if [ "$#" -gt 1 ]; then
          die "workflow takes at most one preset name (got: $*)"
        fi
        workflow_name="${1:-$profile}"
        if [ -n "$toolchain" ]; then
          workflow_name="$workflow_name-$toolchain"
        fi
        run_cmd cmake --workflow "$workflow_name"
        ;;
      examples)
        examples_list
        ;;
      *)
        usage
        die "unknown action '$action'"
        ;;
    esac
