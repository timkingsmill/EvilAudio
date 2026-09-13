# PlantUML Implementation Guide

This document describes the current PlantUML and Doxygen documentation pipeline in EvilAudio. It covers how the toolchain is discovered, how CMake configures generation, how header-derived diagrams are named safely, and how the full-docs and PlantUML-only flows differ.

## 1. Scope

PlantUML support in EvilAudio serves two related but distinct purposes:

1. Render authored `.puml` diagrams through Doxygen.
2. Generate PlantUML source diagrams from C and C++ headers using `hpp2plantuml`.

The implementation is centered in:

1. `docs/tools/CMakeLists.txt`
2. `docs/tools/EvilAudioAPIDocumentation.cmake`
3. `docs/tools/doxygen/Doxyfile.in`
4. `docs/tools/plantUML/plantuml.cfg`

The top-level project enters this flow through the normal docs CMake path:

1. root `CMakeLists.txt`
2. `docs/CMakeLists.txt`
3. `docs/tools/CMakeLists.txt`

## 2. Toolchain Overview

The current implementation depends on three categories of tooling.

### 2.1 Doxygen

Doxygen is the final documentation renderer. It consumes the generated `Doxyfile`, scans the source tree, and renders HTML output. PlantUML integration is enabled through variables written into `docs/tools/doxygen/Doxyfile.in`.

### 2.2 PlantUML JAR

PlantUML rendering is performed by a bundled Java JAR discovered from `docs/tools/plantUML` or `docs/tools/plantuml`. The newest JAR is copied into the docs build directory using the stable filename `plantuml.jar` so Doxygen can reference a predictable path.

### 2.3 hpp2plantuml

`hpp2plantuml` is a Python package used to convert headers into `.puml` source files. These generated files are then available to Doxygen via `PLANTUMLFILE_DIRS`.

## 3. Python Environment and Package Installation

The repository initializes Python prerequisites during the top-level configure flow.

Current prerequisite behavior:

1. A virtual environment is created at `.venv` if it does not already exist.
2. If the environment already exists, it is reused.
3. Core packaging tools are upgraded when the venv is first created.
4. `hpp2plantuml` is installed into that environment.

Relevant functions:

1. `evilaudio_install_or_upgrade_python_venv(...)`
2. `evilaudio_ensure_hpp2plantuml_installed_in_venv(...)`
3. `evilaudio_resolve_venv_python(...)`

Important current behavior:

1. `evilaudio_ensure_hpp2plantuml_installed_in_venv(...)` now performs `pip install` directly instead of a separate `pip show` pre-check.
2. An optional version pin can be supplied through the helper's `VERSION` argument.
3. No repository patching of `CppHeaderParser` is part of the current flow.

## 4. Documentation Helper Module

The shared logic for the docs toolchain is implemented in `docs/tools/EvilAudioAPIDocumentation.cmake`.

Key responsibilities of this module:

1. Resolve docs output directories with `get_documentation_file_and_directories(...)`.
2. Generate a configured Doxygen file with `create_doxygen_config_file(...)`.
3. Discover and stage the PlantUML JAR and config with `create_plantuml_config(...)`.
4. Resolve a Python interpreter for docs tooling.
5. Ensure `hpp2plantuml` is available.
6. Generate per-header `.puml` files with `run_hpp2plantuml_for_doxygen(...)`.
7. Generate both `.h` and `.hpp` header groups through the shared wrapper `run_hpp2plantuml_header_globs_for_doxygen(...)`.

The addition of `run_hpp2plantuml_header_globs_for_doxygen(...)` is important because both configure-time docs generation and the `generate_plantuml_only` target now use the same canonical helper rather than duplicating two separate `run_hpp2plantuml_for_doxygen(...)` call blocks.

## 5. Configure-Time Flow in docs/tools/CMakeLists.txt

The current `docs/tools/CMakeLists.txt` performs the following steps during configure.

1. Includes `EvilAudioAPIDocumentation.cmake`.
2. Requires Doxygen with `find_package(Doxygen)`.
3. Resolves docs output directories.
4. Uses `${CMAKE_BINARY_DIR}/docs` as the docs root.
5. Uses `${CMAKE_BINARY_DIR}/docs/html` as the HTML root.
6. Uses `${CMAKE_BINARY_DIR}/docs/html/puml` as the generated PlantUML source root.
7. Defines Doxygen project metadata such as `EVIL_PROJECT_NAME` and `EVIL_PROJECT_BRIEF`.
8. Defines Doxygen inputs and exclusions.
9. Refreshes the bundled `C4-PlantUML` directory into the build tree.
10. Creates the PlantUML runtime configuration.
11. Converts the PlantUML search and include paths into Doxygen-compatible strings.
12. Generates header-derived `.puml` source diagrams under the PlantUML output directory.
13. Generates the final `Doxyfile` from `docs/tools/doxygen/Doxyfile.in`.
14. Invokes Doxygen and fails configuration if Doxygen exits non-zero.

## 6. Doxygen Inputs and Variables

The Doxygen template at `docs/tools/doxygen/Doxyfile.in` consumes the following variables from `docs/tools/CMakeLists.txt`.

### 6.1 Core metadata

1. `EVIL_PROJECT_NAME`
2. `EVIL_PROJECT_BRIEF`
3. `EVIL_OUTPUT_DIRECTORY`

### 6.2 Input scanning

1. `EVIL_INPUT`
2. `EVIL_EXCLUDE`
3. `EVIL_EXCLUDE_PATTERNS`

### 6.3 PlantUML integration

1. `EVIL_PLANTUML_JAR_PATH`
2. `EVIL_PLANTUML_CFG_FILE`
3. `EVIL_PLANTUML_INCLUDE_PATH`
4. `EVIL_PLANTUML_FILE_DIRS`

Those values map directly into these Doxygen settings:

1. `PLANTUML_JAR_PATH`
2. `PLANTUML_CFG_FILE`
3. `PLANTUML_INCLUDE_PATH`
4. `PLANTUMLFILE_DIRS`

Current important detail:

1. `PLANTUMLFILE_DIRS` points at `${CMAKE_BINARY_DIR}/docs/html/puml`, which is where generated header-based `.puml` files are staged.

## 7. C4-PlantUML Asset Handling

The current build no longer performs a one-time copy and skip. Instead it refreshes the build copy every configure run.

Current behavior:

1. Source assets are expected at `docs/tools/C4-PlantUML`.
2. Destination root is `${CMAKE_BINARY_DIR}/docs/tools`.
3. If `${CMAKE_BINARY_DIR}/docs/tools/C4-PlantUML` already exists, it is removed.
4. The source directory is copied into the destination root.
5. The copy is validated by checking for `${_C4_PLANTUML_DESTINATION_DIR}/C4_Context.puml`.
6. Configure fails if the refreshed assets are not present.

This prevents stale C4 includes from lingering across incremental configure runs.

## 8. Header-to-PlantUML Generation

### 8.1 Canonical wrapper

The high-level entry point is:

1. `run_hpp2plantuml_header_globs_for_doxygen(...)`

It exists specifically to generate both header groups consistently:

1. `source/**/*.h`
2. `source/**/*.hpp`

Its inputs are:

1. `OUTPUT_FILE_H`
2. `OUTPUT_FILE_HPP`
3. `WORKING_DIRECTORY`
4. `PYTHON_EXECUTABLE`
5. `VENV_PATH`
6. `QUIET`
7. `SUPPRESS_PARSE_WARNINGS`

The `.hpp` pass always adds `ALLOW_EMPTY_INPUT` so the flow remains stable even if a subtree contains no `.hpp` matches.

### 8.2 Low-level conversion helper

The wrapper delegates to:

1. `run_hpp2plantuml_for_doxygen(...)`

This helper performs the following work:

1. Validates required arguments.
2. Resolves the Python interpreter.
3. Ensures `hpp2plantuml` is installed.
4. Expands the input glob with `file(GLOB_RECURSE ...)`.
5. Derives the output directory and optional filename prefix from `OUTPUT_FILE`.
6. Creates the destination directory if needed.
7. Invokes `hpp2plantuml` once per input file.

### 8.3 UTF-8 execution hardening

Each invocation is wrapped with:

1. `PYTHONUTF8=1`
2. `PYTHONIOENCODING=UTF-8`

This reduces parser failures caused by encoding issues in complex headers.

### 8.4 Error and warning behavior

Supported behavior flags are:

1. `FAIL_ON_ERROR`
2. `QUIET`
3. `SUPPRESS_PARSE_WARNINGS`
4. `ALLOW_EMPTY_INPUT`

Current semantics:

1. `FAIL_ON_ERROR` converts a tool failure into `FATAL_ERROR`.
2. `QUIET` suppresses normal status logging for tool output.
3. `SUPPRESS_PARSE_WARNINGS` skips non-fatal parser/runtime failures instead of warning noisily.
4. `ALLOW_EMPTY_INPUT` suppresses the empty-glob warning.

## 9. Collision-Safe Output Naming

Generated file naming is intentionally collision-resistant.

The current naming algorithm:

1. Computes the input file path relative to the configured working directory.
2. If the file is not under that working directory, it retries relative to `CMAKE_SOURCE_DIR`.
3. If the file is still outside the source tree, it falls back to basename plus a short MD5 hash.
4. Path separators are converted to underscores.
5. The file extension is removed.
6. The optional output prefix is prepended.

Practical result:

1. Headers with the same basename in different folders no longer overwrite each other.
2. Generated names remain readable when the file lives inside the source tree.
3. Files outside the source tree remain unique through hash suffixing.

Typical patterns now look like:

1. `headers_source_modules_evilaudio_eq_eq_Analyser.puml`
2. `headers_hpp_source_libs_some_lib_include_widget.puml`
3. `plantuml_all_headers_h_source_modules_... .puml`

## 10. Generation Modes

### 10.1 Full documentation mode

This happens during:

```powershell
cmake -S . -B build
```

In this mode:

1. Header-based `.puml` sources are generated first.
2. Doxygen is then invoked using the configured `Doxyfile`.
3. Doxygen can resolve generated PlantUML sources through `PLANTUMLFILE_DIRS`.

### 10.2 PlantUML-only mode

The repository also defines a focused custom target:

1. `generate_plantuml_only`

This target is implemented by generating a script at configure time:

1. `${CMAKE_CURRENT_BINARY_DIR}/GeneratePlantUMLOnly.cmake`

That generated script:

1. Re-establishes `CMAKE_SOURCE_DIR` and `CMAKE_BINARY_DIR`.
2. Includes `EvilAudioAPIDocumentation.cmake`.
3. Calls `run_hpp2plantuml_header_globs_for_doxygen(...)` with `QUIET` and `SUPPRESS_PARSE_WARNINGS`.

This mode is useful when you want fresh header-derived `.puml` files without rerunning the full Doxygen pass.

Command:

```powershell
cmake --build build --target generate_plantuml_only
```

## 11. Output Layout

Current output layout is:

1. Docs root: `${CMAKE_BINARY_DIR}/docs`
2. HTML root: `${CMAKE_BINARY_DIR}/docs/html`
3. Generated PlantUML source root: `${CMAKE_BINARY_DIR}/docs/html/puml`
4. Staged C4 assets: `${CMAKE_BINARY_DIR}/docs/tools/C4-PlantUML`
5. Generated Doxygen file: `${CMAKE_CURRENT_BINARY_DIR}/Doxyfile`
6. Generated PlantUML-only script: `${CMAKE_CURRENT_BINARY_DIR}/GeneratePlantUMLOnly.cmake`
7. Staged PlantUML runtime JAR: `${CMAKE_CURRENT_BINARY_DIR}/plantuml.jar`
8. Staged PlantUML config file: `${CMAKE_CURRENT_BINARY_DIR}/plantuml.cfg`

## 12. Doxygen Failure Handling

The current implementation checks the result of the Doxygen process explicitly.

Current behavior:

1. `execute_process(...)` captures `RESULT_VARIABLE _doxygen_result`.
2. Configure fails with `FATAL_ERROR` if Doxygen exits with a non-zero code.

This prevents silent documentation failures during configure.

## 13. Operational Commands

Configure the full docs pipeline:

```powershell
cmake -S . -B build
```

Re-run just the PlantUML generation target:

```powershell
cmake --build build --target generate_plantuml_only
```

If you want a clean rebuild of generated docs artifacts:

```powershell
Remove-Item -Recurse -Force build\docs
cmake -S . -B build
```

## 14. Workflow: Add a New PlantUML Diagram

Use this workflow when you want to add a hand-authored diagram and render it through Doxygen.

1. Create the diagram file under the diagram tree used by current source comments, for example:
   `source/docs/puml/diagrams/<area>/<diagram_name>.puml`
2. Write the PlantUML content and include C4 definitions if needed, for example:
   `!include C4_Context.puml`
   `!include C4_Container.puml`
3. Reference the diagram from the relevant header or source documentation block using `@plantumlfile`, for example:
   `@plantumlfile docs/puml/diagrams/application/EvilDAWApplicationSequence.puml "EvilDAW Startup Sequence"`
4. Reconfigure docs so generated configuration is refreshed:

```powershell
cmake -S . -B build
```

1. Verify the diagram renders in generated docs.
2. If Doxygen cannot resolve the diagram path, update PlantUML search paths in `docs/tools/CMakeLists.txt` by extending `EVIL_PLANTUML_FILE_DIRS`, then rerun configure.

Notes:

1. Use repository-relative paths in `@plantumlfile` consistently.
2. Keep diagram files in a module- or feature-specific folder to avoid naming collisions.
3. Use the `generate_plantuml_only` target only for header-derived diagrams; it does not render full Doxygen HTML.

## 15. Known Constraints and Current Tradeoffs

1. `hpp2plantuml` can still fail on especially complex or non-standard headers.
2. Current configured calls favor resilience over strict failure by using `SUPPRESS_PARSE_WARNINGS`.
3. Doxygen still depends on Java being usable for PlantUML rendering.
4. The generated source diagrams are one file per matched header, which improves isolation and collision resistance at the cost of a larger number of output files.
5. `generate_plantuml_only` generates source diagrams only; it does not run Doxygen HTML generation.

## 16. Source of Truth Files

For future maintenance, the most relevant files are:

1. `CMakeLists.txt`
2. `docs/CMakeLists.txt`
3. `docs/tools/CMakeLists.txt`
4. `docs/tools/EvilAudioAPIDocumentation.cmake`
5. `docs/tools/doxygen/Doxyfile.in`
6. `docs/tools/plantUML/plantuml.cfg`
7. `docs/tools/plantUML/plantuml.md`
8. `cmake/EvilAudioPrerequisiteUtils.cmake`
9. `cmake/EvilAudioDocumentUtils.cmake`
