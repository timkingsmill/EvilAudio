# CMake Build Process

## Overview

CMake is a meta-build system that generates platform-specific build files (like Visual Studio solutions, Makefiles, or Ninja build files) from a declarative configuration. The EvilAudio project uses CMake to manage its build process across different platforms and compilers.

## The CMake Build Workflow

The CMake build process consists of several distinct phases:

### 1. **Configuration Phase**

This is the first step when you run CMake. During this phase:

- **CMakeLists.txt Processing**: CMake reads and processes all `CMakeLists.txt` files in the project hierarchy
- **Variable Evaluation**: CMake variables are set and evaluated
- **Platform Detection**: The system type, compiler, and available tools are detected
- **Dependency Resolution**: External dependencies (like GoogleTest in this project) are found or downloaded
- **Cache Generation**: CMake creates a cache file (`CMakeCache.txt`) that stores configuration values
- **Build File Generation**: Platform-specific build files are generated in the `build/` directory

**Key Files Generated:**
- `CMakeCache.txt` - Stores all configuration settings
- `CMakeFiles/` - Contains intermediate CMake processing files
- Project-specific build files (`.vcxproj` for Visual Studio, `Makefile` for Unix, etc.)

### 2. **Build Phase**

Once configuration is complete, the actual compilation happens:

- **Source Compilation**: C++ source files are compiled into object files
- **Linking**: Object files are linked together to create executables or libraries
- **Test Executable Generation**: Test programs (from `testing/`) are compiled
- **Artifacts Output**: Final binaries are placed in the `build/` directory

**In EvilAudio:**
- Applications are built from `source/applications/`
- Libraries are built from `source/libs/`
- Plugins are built from `source/plugins/`
- Tests are built from `source/testing/`

### 3. **Install Phase** (Optional)

If you run the install command, CMake copies built artifacts to their final destination:

- Binaries go to system binary directories
- Libraries go to system library directories
- Headers go to include directories
- Configurations and resources are deployed

### 4. **Testing Phase** (Optional)

CMake can run tests using CTest:

- Tests defined in `source/testing/` are executed
- Results are collected and reported
- Failures can be analyzed and debugged

## Key CMake Concepts in EvilAudio

### **CMakeLists.txt Files**

The project contains multiple `CMakeLists.txt` files in a hierarchical structure. Each file defines what targets (executables, libraries) are built, their source files, dependencies between targets, compiler flags and options, and installation rules.

#### **Root Level**

##### **CMakeLists.txt** (Project Root)
- Sets the minimum CMake version (4.2.0)
- Defines project-wide options and settings
- Includes custom CMake modules from `extras/cmake/`
- Configures JUCE framework options
- Sets up compiler flags and multiprocessor compilation for MSVC
- Enables optional features like API documentation and PlantUML target graph generation
- Calls `add_subdirectory()` to process all major component directories

##### **external/CMakeLists.txt**
- Configures external dependencies for the project
- Sets up JUCE framework by disabling helper tools and enabling modules-only mode
- Adds JUCE as a subdirectory for inclusion in the build
- Does not build JUCE helper tools to keep the build lean

#### **Source Components**

##### **source/CMakeLists.txt**
- Orchestrates the build of all internal source components
- Includes guards to prevent multiple inclusions
- Adds subdirectories for libraries, applications, and testing
- Coordinates the order of builds to ensure proper dependency resolution
- Currently commented out: examples and plugins (can be uncommented when ready)

##### **source/libs/CMakeLists.txt**
- Manages all internal library targets
- Adds subdirectory for JUCE static libraries wrapper
- Adds subdirectory for EvilAudio's custom libraries (evil_audio_libs)
- Sets up GoogleTest framework for unit testing
- Adds subdirectory for the EvilDAW library (evil_daw_lib)
- Ensures all libraries are built before dependent applications and plugins

##### **source/libs/juce_static_libs/CMakeLists.txt**
- Wraps and configures JUCE as static libraries
- Simplifies JUCE integration by abstracting its complexity
- Makes JUCE modules available to other targets through a single interface

##### **source/libs/evil_audio_libs/CMakeLists.txt**
- Coordinates the build of EvilAudio's custom libraries
- Adds subdirectories for specialized libraries like evil_settings_lib and evil_plugins_lib
- Provides a common build interface for all EvilAudio-specific code

##### **source/libs/evil_audio_libs/evil_settings_lib/CMakeLists.txt**
- Defines the settings management library
- Contains configuration and settings handling code
- Typically provides utilities for loading, saving, and managing application settings

##### **source/libs/evil_audio_libs/evil_plugins_lib/CMakeLists.txt**
- Defines the plugin management library
- Contains code for plugin discovery, loading, and communication
- Provides the framework for integrating VST/AU plugins

##### **source/libs/EvilDAWLib/evil_daw_lib/CMakeLists.txt**
- Defines the core DAW (Digital Audio Workstation) library
- Contains the main audio engine and DAW functionality
- Provides core abstractions used by applications and plugins

#### **Applications**

##### **source/applications/EvilDAW/CMakeLists.txt**
- Orchestrates all EvilDAW-related applications and components
- Includes guards to prevent multiple inclusions
- Adds subdirectories for individual EvilDAW applications

##### **source/applications/EvilEQ/CMakeLists.txt**
- Defines the EvilEQ GUI application target
- Creates an executable using the `evil_audio_add_gui_app()` custom function
- Sets up JUCE integration with product name, version, and bundle ID
- Configures compile definitions (JUCE options like browser and curl support)
- Links against JUCE recommended warning, config, and link-time optimization flags
- Includes directories for application-specific headers

##### **source/applications/EvilEQ/source/CMakeLists.txt**
- Defines source files for the EvilEQ application
- Lists the actual `.cpp` and `.h` source files that make up EvilEQ

##### **source/applications/EvilEQ/include/CMakeLists.txt**
- Defines header files and public interface of the EvilEQ application
- Specifies which headers are part of the public API

##### **source/applications/EvilLookAndFeel/CMakeLists.txt**
- Defines a reusable look-and-feel library for GUI applications
- Provides consistent styling across EvilAudio applications

##### **source/applications/EvilLookAndFeel/source/CMakeLists.txt**
- Defines source files for the look-and-feel implementation

##### **source/applications/EvilLookAndFeel/include/CMakeLists.txt**
- Defines the public header interface for the look-and-feel library

#### **Plugins**

##### **source/plugins/EvilEQPlugin/CMakeLists.txt**
- Defines the EvilEQ plugin as a JUCE plugin project
- Uses `juce_add_plugin()` to create a plugin target with specific format support (Standalone, VST3, AU, AAX)
- Sets up plugin metadata (manufacturer code: OAUD, plugin code: Eq01)
- Configures plugin properties like MIDI input/output and keyboard focus
- Generates the required JuceHeader.h
- Links against the evil_audio_libs for plugin logic

#### **Testing**

##### **source/testing/CMakeLists.txt**
- Sets up the test driver application using GoogleTest framework
- Calls `evil_audio_setup_google_test()` to configure the testing environment
- Uses `evil_audio_find_test_sources()` to discover all test files in the project
- Creates an executable test driver (EvilAudioTestDriver) that runs all discovered tests
- Links against GoogleTest libraries (gtest_main and gtest)
- Includes test mock files for testing library internals

#### **Documentation**

##### **docs/CMakeLists.txt**
- Configures documentation generation targets
- Sets up Doxygen for API documentation generation
- Organizes documentation sources and output

##### **docs/tools/CMakeLists.txt**
- Defines utility targets for documentation tooling
- May include PlantUML diagram generation for CMake target graphs
- Provides scripts for documentation maintenance and generation

#### **CMakeLists.txt Hierarchy Diagram**

```
CMakeLists.txt (root)
├── external/CMakeLists.txt
│   └── JUCE framework setup
├── source/CMakeLists.txt
│   ├── libs/CMakeLists.txt
│   │   ├── juce_static_libs/CMakeLists.txt
│   │   ├── evil_audio_libs/CMakeLists.txt
│   │   │   ├── evil_settings_lib/CMakeLists.txt
│   │   │   └── evil_plugins_lib/CMakeLists.txt
│   │   └── EvilDAWLib/evil_daw_lib/CMakeLists.txt
│   ├── applications/EvilDAW/CMakeLists.txt
│   │   ├── applications/EvilEQ/CMakeLists.txt
│   │   │   ├── source/CMakeLists.txt
│   │   │   └── include/CMakeLists.txt
│   │   └── applications/EvilLookAndFeel/CMakeLists.txt
│   │       ├── source/CMakeLists.txt
│   │       └── include/CMakeLists.txt
│   ├── plugins/EvilEQPlugin/CMakeLists.txt
│   └── testing/CMakeLists.txt
└── docs/CMakeLists.txt
    └── tools/CMakeLists.txt
```

### **Targets**

A target is a unit of output that CMake builds. Common types include:

- **Executable Targets**: Programs that can be run (applications)
- **Library Targets**: Code libraries (.dll, .a, .so) used by other targets
- **Custom Targets**: Non-compiled outputs (documentation, code generation, etc.)

### **Dependencies**

CMake tracks dependencies between targets:

- **Target Dependencies**: If Target A depends on Target B, B is built first
- **External Dependencies**: External libraries (JUCE, GoogleTest) are linked
- **Header Dependencies**: Changes to header files trigger recompilation of dependent source files

### **Build Configuration**

CMake supports multiple build configurations:

- **Debug**: Includes debug symbols, no optimization (used for development)
- **Release**: Optimized, no debug symbols (used for production)
- **RelWithDebInfo**: Optimized with debug symbols (used for performance profiling)
- **MinSizeRel**: Minimized size (used for embedded or resource-constrained environments)

## The Build Directory Structure

When CMake configures the project, it creates this structure in `build/`:

```
build/
├── CMakeCache.txt           # Configuration cache
├── CMakeFiles/              # Temporary CMake files
├── CMakeDoxyfile.in         # Doxygen configuration template
├── _deps/                   # External dependencies (GoogleTest, etc.)
├── source/                  # Build outputs for source targets
├── docs/                    # Documentation build output
├── external/                # External library builds
├── INSTALL.dir/             # Install target directory
├── ZERO_CHECK.dir/          # Reconfiguration check directory
├── ALL_BUILD.vcxproj        # Visual Studio meta-project
├── cmake_install.cmake      # Installation script
└── CTestTestfile.cmake      # Test configuration
```

## Typical CMake Commands

### **Configure the Project**
```bash
cmake -S . -B build -G "Visual Studio 16 2019"
```
- `-S .` specifies the source directory (where CMakeLists.txt is)
- `-B build` specifies the build directory
- `-G` specifies the generator (build system to use)

### **Build the Project**
```bash
cmake --build build --config Release
```
- Compiles all targets in the specified configuration

### **Run Tests**
```bash
ctest --build-dir build -C Release
```
- Runs all tests defined in the project

### **Install**
```bash
cmake --install build --config Release
```
- Installs built artifacts to their final locations

### **Clean**
```bash
cmake --build build --target clean
```
- Removes all build artifacts

## CMake Variables

CMake uses variables to control behavior. Common ones include:

- `CMAKE_BUILD_TYPE` - Debug, Release, etc.
- `CMAKE_CXX_COMPILER` - Which C++ compiler to use
- `CMAKE_CXX_FLAGS` - Compiler flags
- `CMAKE_INSTALL_PREFIX` - Where to install files
- `BUILD_TESTING` - Whether to build tests (on/off)

## Project-Specific Configuration

In EvilAudio, the root `CMakeLists.txt` typically:

1. Sets the minimum CMake version required
2. Declares the project name and version
3. Defines common compile options and warnings
4. Configures external dependencies (JUCE, GoogleTest)
5. Adds subdirectories for each component (apps, libs, plugins, tests)
6. Defines installation rules

## Incremental Builds

After the initial configuration, CMake detects changes and only rebuilds what's necessary:

- **Source file changes**: Only that file and files that depend on it are recompiled
- **CMakeLists.txt changes**: CMake automatically reconfigures and rebuilds affected targets
- **Header file changes**: All files that include that header are recompiled
- **Dependency changes**: The build system knows which targets depend on which and rebuilds in the correct order

## Common Issues and Solutions

### **Out-of-Sync Build**
If CMakeLists.txt changes aren't reflected:
```bash
cmake --build build --target clean
cmake -S . -B build
cmake --build build
```

### **Compiler Not Found**
Specify the compiler explicitly:
```bash
cmake -S . -B build -DCMAKE_CXX_COMPILER=clang++
```

### **Different Generators**
To use a different build system, regenerate with a different generator:
```bash
cmake -S . -B build -G Ninja
```

## Conclusion

CMake abstracts away the complexity of cross-platform builds by providing a single configuration language that generates appropriate build files for your target platform. For EvilAudio, this means developers can work on Windows with Visual Studio, macOS with Xcode, or Linux with Make/Ninja using the same CMakeLists.txt files.
