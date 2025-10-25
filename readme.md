# Build

## Prerequisites

- CMake 3.10 or higher
- C++20 compatible compiler (GCC 10+, Clang 11+, or MSVC 2019+)
- Qt5 (Widgets, Core, Gui)
- OpenCV 4.x

## Building the Project

### Using the build script (recommended)

```bash
./build.sh
```

### Using CMake directly

```bash
mkdir -p build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build . -j$(nproc)
```

## Running

After building, run the application:

```bash
./run.sh
```

Or execute the binary directly:

```bash
./build/pixlscan
```

# Assets

- **qdarkstyle/** - QDarkStyleSheet v3.2.3 from https://github.com/ColinDuquesnoy/QDarkStyleSheet
- **fontawesome-free-6.7.2-web/** - Font Awesome (not tracked by git)
