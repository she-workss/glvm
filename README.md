# Game Loop Versatile Modules (GLVM) - C++ Game Engine

Simple game engine for Windows and Linux with Vulkan support.

## Features

- **ECS** based on archetypes.
- `GLTf` and `Wavefront .obj` parsers.
- Simple phong light (directional light, spotlight, point light).
- Basic physics support (collisions, gravity).

## Usage

```cpp
// TBD

auto main() -> i32 {
}
```

## Building

### Basic Build
```bash
# Configure and build (Debug)
cmake --workflow d

# Configure, build and run tests (Debug)
cmake --workflow dfull

# Configure
cmake --preset debug

# Build everything
cmake --build --preset debug
```

## Running

### Examples
```bash
# Run basic GLVM example
cmake --build --preset debug -t run_hello_world
```

See `examples/` folder for usage examples.

### Tests
```bash
# Run all tests (unit + integration)
ctest --preset debug --output-on-failure --parallel

# Run unit tests only
cmake --build --preset debug -t test_basic_glvm

# Run integration tests only
cmake --build --preset debug -t test_basic
```

See [TESTING.md](TESTING.md) for comprehensive testing guide.


### Benchmarks
```bash
# Run ECS benchmarks
cmake --build --preset release -t bench_ecs
```

### Documentation
```bash
# Build docs
cmake --build --preset debug -t docs
```

Then open up `./build/debug/docs/html/index.html` file:

## Requirements

- CMake 3.25+
- C++26 compatible compiler (GCC 14+, Clang 18+, MSVC 19.40+)

## Linux

### Development libraries

* X11, Xi, XRandR;
* Vulkan;
* Alsa;
* PulseAudio.

### Repository specific

#### Gentoo

```bash
emerge --ask x11-libs/libX11 \
    x11-libs/libXi \
    x11-apps/xrandr \
    media-libs/vulkan-loader \
    dev-util/vulkan-tools \
    media-libs/mesa \
    media-libs/alsa-lib \
    media-sound/pulseaudio
```

#### Debian

```bash
apt install libx11-dev \
    libxi-dev \
    libxrandr-dev
    libgl1-mesa-dev \
    libasound2-dev \
    libpulse-dev \
    libudev-dev
```

#### Arch

```bash
pacman -S libxi \
    libxrandr \
    mesa \
    libglvnd \
    alsa-lib \
    pulseaudio
```

#### Fedora

```bash
dnf install libX11-devel \
    libXrandr-devel \
    libXi-devel \
    mesa-libGL-devel \
    alsa-lib-devel \
    pulseaudio-libs-devel \
    libudev-devel \
    libstdc++-static
```

## Windows

### Development libraries

* Vulkan.

### Specific tools

1. Install MSYS2:
    You can get it from official website (https://www.msys2.org/) or using
    `winget`:
    ```bash
    winget install MSYS2.MSYS2
    ```

2. Install Vulkan libs:
    ```bash
    pacman -S mingw-w64-x86_64-vulkan-devel
    ```

### Optionally:

- Doxygen 1.9.5+ (for documentation)
- Graphviz (for class/include diagrams in documentation)

## Contributing

See [CONTRIBUTING.md](CONTRIBUTING.md) for complete documentation.

## Architecture

See [ARCHITECTURE.md](ARCHITECTURE.md) for complete documentation.
