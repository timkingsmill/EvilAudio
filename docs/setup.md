# EvilAudio Setup

## Configure the source code

Clone the EvilAudio GitHub Repository and initialize the JUCE library submodules.

```shell
    git clone --recurse-submodules https://github.com/timkingsmill/EvilAudio.git
```

If you've already cloned the EvilAudio repository, initialize and update submodules.

```shell
    git submodule update --init --recursive
```

To update submodules to the latest commit on their tracked branch:

```shell
    git submodule update --remote
```

For more information, see the [Git Submodules Documentation](https://git-scm.com/book/en/v2/Git-Tools-Submodules).

## Prerequisites

### 1. Installing The JUCE Library

To install JUCE globally on your system, you'll need to tell CMake where to place the installed files.

```shell
    # Go to JUCE directory
    cd /path/to/clone/JUCE

    # Configure JUCE build with library components only and installation path.
	cmake -B juce-install -DCMAKE_INSTALL_PREFIX=/path/to/JUCE/install

    # Install the JUCE library to the path specified above.
    cmake --build juce-install --target install
```cd

For example:

```shell
    cmake -B juce-install -DCMAKE_INSTALL_PREFIX=/source/EvilAudio/libs/JUCE/install
    cmake --build juce-install --target install
```

### 2. Consuming the JUCE library

In your project which consumes JUCE, make sure the project CMakeLists.txt contains the line: `find_package(JUCE CONFIG REQUIRED)`.

This will make the JUCE modules and CMake helper functions
available for use in the rest of your build. Then, run the build like so:

```shell
    # Go to project directory
    cd /path/to/my/project
    # Configure build, passing the JUCE install path you used earlier
    cmake -B cmake-build -DCMAKE_PREFIX_PATH=/path/to/JUCE/install
```

cmake -B cmake-build -DCMAKE_INSTALL_PREFIX=/source/EvilAudio/libs/JUCE/install

```shell
    # Build the project
    cmake --build cmake-build

```
# EvilAudio Documentation

## Project Overview

EvilAudio is a **C++23 JUCE-based DAW framework** that wraps JUCE modules as static libraries for reuse across applications and plugins. Currently, the active target is the sample app **EvilStaticLibraryClient**.

## Architecture

### Directory Structure

```
EvilAudio/
├── cmake/
│   └── EvilAudioUtils.cmake          # Build helpers and utilities
├── libs/
│   ├── JUCE/
│   │   └── install/                  # JUCE installation directory
│   ├── asiosdk/                      # ASIO SDK (Windows audio support)
│   └── EvilAudio/
│       └── CMakeLists.txt            # JUCE module wrapping
├── source/
│   ├── modules/                      # Custom JUCE-style modules
│   ├── applications/
│   │   └── EvilStaticLibraryClient/  # Sample application
│   └── plugins/                      # Plugin targets (commented out)
├── CMakeLists.txt                    # Root CMake configuration
└── external/
    └── JUCE/                         # JUCE submodule
```

## Build System

### Key CMake Helpers

**EvilAudioUtils.cmake** provides:
- `add_juce_module_static_library()` - Wraps JUCE modules into static libraries
- `_get_juce_module_sources()` - Collects and filters module source files
- `_should_build_source_file()` - Platform-specific source filtering (Windows/Mac/Linux/Android/iOS)
- `target_add_module_source_groups()` - Organizes module files in IDE source trees

### Build Workflow

#### 1. **Install JUCE**
```bash
cmake -B external/JUCE/juce-install -DCMAKE_INSTALL_PREFIX=libs/JUCE/install
cmake --build external/JUCE/juce-install --target install
```

#### 2. **Configure Build**
```bash
cmake -B build -DCMAKE_PREFIX_PATH=libs/JUCE/install
```

#### 3. **Build Project**
```bash
cmake --build build
cmake --build build --target EvilStaticLibraryClient  # Smoke test
```

### Requirements
- CMake ≥ 4.2
- C++23 (enforced globally, extensions OFF)
- JUCE installed at `libs/JUCE/install`

## Module System

### Module Naming Convention

- **Format**: `<namespace>_<name>`
- **Static Library Target**: `evil_<name>`
- **Example**: `juce_core` → `evil_juce_core`

### Module Registration Flow

1. **Create Module**: Place code in `source/modules/<mod>/<mod>/`
2. **Register**: Add entry to `source/modules/CMakeLists.txt`
3. **Wrap**: Add `add_juce_module_static_library()` call in `libs/EvilAudio/CMakeLists.txt`
4. **Link**: Reference `evil_<mod>` in your application CMakeLists.txt

### Module Structure Example

```
source/modules/evilaudio_core/
├── evilaudio_core/
│   ├── evilaudio_core.h       # Module header
│   ├── evilaudio_core.cpp     # Implementation
│   └── ...other files...
└── CMakeLists.txt             # Module metadata
```

## Compilation Requirements

### Global Compile Definitions

All wrapped modules must define:
```cmake
JUCE_GLOBAL_MODULE_SETTINGS_INCLUDED=0
JUCE_WEB_BROWSER=0
JUCE_USE_CURL=0
JUCE_INCLUDE_ZLIB_CODE=1
```

### Special Handling for juce_core

The `juce_core` module receives additional configuration:
- Links `juce::juce_atomic_wrapper`
- **BSD**: Links `execinfo`
- **Android**: Includes CPU features, links `android` and `log`
- Exports standard debug/release definitions

## Platform Support

### Source File Filtering

Platform-specific files are automatically included/excluded based on file suffix:
- `_windows` - Windows only
- `_mac`, `_osx` - macOS only
- `_linux` - Linux/BSD
- `_android` - Android only
- `_ios` - iOS only

**Example**: `MyClass_windows.cpp` is only compiled on Windows.

### ASIO Support (Windows)

If `libs/asiosdk/common` exists, `JUCE_ASIO_SUPPORT` is automatically enabled. Otherwise, a build warning is emitted.

## Adding Components

### New JUCE-Style Module

1. Create directory: `source/modules/<mod>/<mod>/`
2. Add `<mod>.h` header file
3. Register in `source/modules/CMakeLists.txt`:
   ```cmake
   add_subdirectory(<mod>)
   ```
4. Wrap in `libs/EvilAudio/CMakeLists.txt`:
   ```cmake
   add_juce_module_static_library(juce::<mod>)
   ```
5. Link in your app `CMakeLists.txt`:
   ```cmake
   target_link_libraries(MyApp evil_<mod>)
   ```

### New Application

1. Create: `source/applications/<App>/CMakeLists.txt`
2. Define target with JUCE setup
3. Link against `evil_*` libraries
4. Register in `source/CMakeLists.txt`:
   ```cmake
   add_subdirectory(applications/<App>)
   ```

### New Plugin

1. Create: `source/plugins/<Plugin>/CMakeLists.txt`
2. Follow application pattern
3. Register in `source/CMakeLists.txt`:
   ```cmake
   add_subdirectory(plugins/<Plugin>)
   ```

## Configuration Flags

### CMake Options

- `CMAKE_PREFIX_PATH` - Must point to `libs/JUCE/install`
- `JUCE_ENABLE_MODULE_SOURCE_GROUPS` - Toggle IDE source organization (default: ON at root, OFF for modules)
- `CMAKE_BUILD_TYPE` - Debug or Release

## Troubleshooting

| Issue | Solution |
|-------|----------|
| JUCE modules not found | Verify `CMAKE_PREFIX_PATH` points to `libs/JUCE/install` |
| Submodules not available | Run `git clone --recurse-submodules` |
| In-source build error | Use out-of-source build with `-B build` |
| Platform-specific errors | Check source file naming suffixes match your platform |

## Development Guidelines

✅ **Do**:
- Use `add_juce_module_static_library()` for JUCE module wrapping
- Keep `INTERFACE_JUCE_MODULE_*` properties intact on targets
- Follow C++23 standard with no extensions

❌ **Don't**:
- Edit in-source build paths (CMake guards against this)
- Modify `CMAKE_PREFIX_PATH` after submodule updates
- Forget to register new modules in CMakeLists.txt files

---

**Last Updated**: January 2026
