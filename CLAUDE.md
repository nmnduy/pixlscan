# CLAUDE.md

## Common Development Commands

### Build
```bash
./build.sh
```
Or using CMake directly:
```bash
mkdir -p build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build . -j$(nproc)
```

### Run
```bash
./run.sh   # if provided, otherwise run the executable directly
./build/pixlscan
```

### Lint / Format
If clang-tidy and clang-format are set up in the project, you can run:
```bash
clang-tidy src/**/*.cpp -- -Iinclude -std=c++20
clang-format -i src/**/*.cpp include/**/*.hpp
```

### Tests
The repository currently does not contain a test suite. When tests are added under a `tests/` directory (e.g., using Catch2 or GoogleTest), they can be built and executed via CMake. To run a single test binary:
```bash
./build/tests/my_test --filter TestCaseName
```

## High‑Level Architecture

- **Entry point**: `src/main.cpp` creates a `QApplication`, loads the QDarkStyleSheet, and shows the `MainWindow`.
- **MainWindow** (`src/mainwindow.cpp/.h`): Qt UI handling image upload, displaying original and processed images, and invoking the document‑snapping algorithm.
- **Document snapping** (`src/doc_snapper.cpp/.h`): Provides `snapDocument` which processes an OpenCV `cv::Mat` and returns an optional result image.
- **Dependencies**: Qt5 (Widgets, Core, Gui) for the UI and OpenCV for image processing. The build is driven by `CMakeLists.txt`, which pulls in these packages and generates a compilation database.
- **Build output**: The executable `pixlscan` is placed in the `build/` directory. The QDarkStyleSheet assets are copied into the build directory by CMake and loaded at runtime.

## Additional Notes

- The repository includes the QDarkStyleSheet under `QDarkStyleSheet/`; it is copied to the build directory (`darkstyle.qss`) and applied in `main.cpp`.
- No unit tests are present yet; consider adding a `tests/` directory and using CMake's `add_test` to integrate with `ctest`.
- Follow the coding guidelines already present below.

# Coding guidelines

* Prefer composition over inheritance.
* Use templates or concepts for polymorphism when possible.
* Use `std::variant` or `std::visit` instead of class hierarchies for type alternatives.
* Avoid smart pointer cycles (especially with `std::shared_ptr`).
* Prefer smart pointers (`std::unique_ptr`, `std::shared_ptr`) over raw pointers.
* Avoid manual `new` and `delete` unless absolutely necessary.
* Use RAII (Resource Acquisition Is Initialization) for resource management.
* Avoid naked resource handles; wrap them in classes.
* Use `std::vector` or `std::array` instead of dynamic arrays.
* Use references (`&`) instead of pointers when ownership is not needed.
* Avoid returning raw pointers to internal data.
* Do not use dangling pointers or references to destroyed objects.
* Initialize all variables upon declaration.
* Use `std::optional` instead of nullable pointers when possible.
* Avoid `reinterpret_cast` unless absolutely necessary.
* Use `std::move` only when you understand ownership semantics.
* Never access memory after freeing or moving.
* Use `std::span` for safe view over contiguous data.
* Enable compiler sanitizers (`-fsanitize=address,undefined`).
* Avoid global mutable state.
* Make destructors virtual in polymorphic base classes.
* Prefer `std::make_unique` / `std::make_shared` for allocation.
* Use `const` and `constexpr` aggressively to prevent unintended modifications.
* Review ownership semantics in every API (who frees what).
