# CMake Files in `extras/cmake`

This directory contains reusable CMake modules, build helpers, and short
reference documents used by the EvilAudio build. Most `.cmake` files use
`include_guard(GLOBAL)` so a module is evaluated only once during a CMake
configuration.

The root [CMakeLists.txt](../../CMakeLists.txt) loads these modules and then
uses their functions while configuring dependencies, libraries, tests, and
documentation.

## File Overview

| File | Purpose |
| --- | --- |
| `EvilAudioApplicationUtils.cmake` | Creates the initial target for a GUI application. |
| `EvilAudioASIO.cmake` | Detects the optional ASIO SDK and enables JUCE ASIO support on Windows. |
| `EvilAudioCMakeUtils.cmake` | Organizes GoogleTest targets in IDE solution folders. |
| `EvilAudioCommandHandlers.cmake` | Handles optional configure-time commands. |
| `EvilAudioDocumentUtils.cmake` | Provides PlantUML, Python, and VS Code documentation helpers. |
| `EvilAudioLogging.cmake` | Provides indented and centralized CMake status logging. |
| `EvilAudioPrerequisiteUtils.cmake` | Creates, removes, and manages the project Python virtual environment. |
| `EvilAudioStaticLibUtils.cmake` | Creates static JUCE module libraries and resolves their sources and dependencies. |
| `EvilAudioTestSettings.cmake` | Configures GoogleTest, discovers test sources, and adds `.runsettings` files to Visual Studio. |
| `EvilAudioUtils.cmake` | Provides target-linking, target-inspection, and MSVC build helpers. |
| `CMakeCheatSheet.md` | Quick reference for common `target_link_libraries` patterns. |
| `cmakefiles.md` | This directory guide. |

## Module Details

### `EvilAudioApplicationUtils.cmake`

Defines `evil_audio_add_gui_app(target)`. The function creates a shared
library for Android and an executable on other platforms. It is intended to be
the starting point for a JUCE GUI application target.

The file currently contains the target-creation step only. The JUCE
initialization, output naming, bundle configuration, compile definitions, and
resource-file setup are retained as commented calls for future integration.

### `EvilAudioASIO.cmake`

Defines `evil_detect_asio_sdk()`. It checks for the optional SDK at
`source/libs/asiosdk/common`.

When the SDK exists on Windows, the function sets the `ASIO_SDK_PATH` cache
variable and enables `JUCE_ASIO_SUPPORT`. When the SDK is absent, it warns and
sets `ASIO_SDK_PATH` to an empty cache path. Non-Windows platforms do not
enable ASIO through this helper.

### `EvilAudioCMakeUtils.cmake`

Defines `apply_google_test_static_lib_folder(solution_folder)`. It assigns the
`gmock`, `gmock_main`, `gtest`, and `gtest_main` targets to the requested IDE
solution folder.

Missing GoogleTest targets produce warnings rather than fatal errors. This
allows the helper to be called safely around optional or conditionally created
GoogleTest targets.

### `EvilAudioCommandHandlers.cmake`

Defines `evilaudio_handle_configure_time_command(requested_command)`. It
normalizes the requested command and accepts:

- `none`, which performs no action.
- `configure`, which logs that the configure command was received.

An empty value is treated as `none`. Any other value causes a fatal
configuration error. The root `CMakeLists.txt` reads
`EVIL_AUDIO_COMMAND` from the cache and removes it after reading it, so a
one-shot command is not unintentionally repeated on later reconfigure runs.

### `EvilAudioDocumentUtils.cmake`

Provides shared documentation and PlantUML helpers:

- `evil_audio_run_hpp2plantuml(...)` runs `hpp2plantuml` for a supplied input
	glob and output file.
- `_evil_audio_resolve_python_interpreter(...)` chooses a Python interpreter
	from an explicit path, a supplied virtual environment, the active
	environment, the project `.venv`, or `find_package(Python3)`.
- `evil_audio_ensure_hpp2plantuml_installed(...)` checks for and installs the
	Python package, optionally delegating to the virtual-environment helper.
- `_evil_audio_ensure_hpp2plantuml_installed(...)` is a compatibility alias.
- `evil_audio_get_all_puml_files_in_directory(...)` recursively collects
	`.puml` files from one or more directories.
- `evil_audio_add_vscode_plantuml_include(...)` updates `.vscode/settings.json`
	so the PlantUML extension can resolve repository include paths.

The module also normalizes paths for Windows and uses a small embedded Python
script to update the VS Code JSON settings while preserving existing values.

### `EvilAudioLogging.cmake`

Centralizes formatted CMake logging. It stores indentation in the global
property `GLOBAL_MESSAGE_INDENT` and mirrors it into `CMAKE_MESSAGE_INDENT`.

Functions:

- `message(level ...)` wraps the original CMake message implementation.
- `increment_log_indent()` adds one four-space indentation level.
- `decrement_log_indent()` removes one indentation level.
- `reset_log_indent()` clears all indentation.

Other modules use these helpers to make nested configure-time operations easier
to read in the CMake output.

### `EvilAudioPrerequisiteUtils.cmake`

Manages the project-local Python environment and its documentation dependency.

- `evilaudio_install_or_upgrade_python_venv(venv_directory ...)` creates a
	virtual environment when needed and upgrades `pip`, `setuptools`, and
	`wheel`.
- `evilaudio_ensure_hpp2plantuml_installed_in_venv(venv_directory ...)`
	installs `hpp2plantuml`, with optional `VERSION` and `QUIET` arguments.
- `evilaudio_uninstall_python_venv(venv_directory)` removes the environment
	recursively when it exists.
- `evilaudio_resolve_venv_python(venv_directory python_executable)` returns
	the platform-specific interpreter path through the parent scope.

Windows uses `Scripts/python.exe`; POSIX systems use `bin/python`. Creation,
installation, and interpreter-resolution failures stop configuration with a
fatal error.

### `EvilAudioStaticLibUtils.cmake`

Builds static library targets for JUCE modules. The main entry point is
`evil_add_static_lib(source_path)`, which:

1. Validates the module directory and expected module header.
2. Derives a target name and skips blacklisted modules.
3. Collects source files and filters platform-specific files.
4. Adds sources, headers, include directories, and JUCE compile definitions.
5. Applies Windows, MSVC, zlib, and optional ASIO settings.
6. Reads JUCE module metadata and links declared dependencies.

Supporting functions validate module paths, collect source files, decide
whether platform-suffixed files should build, extract metadata blocks, and
retrieve individual metadata values. The blacklist currently excludes
`juce_audio_plugin_client`.

### `EvilAudioTestSettings.cmake`

Provides test and Visual Studio integration helpers:

- `evil_audio_add_runsettings_to_solution_items(runsettings_file)` adds an
	existing `.runsettings` file to Visual Studio solution items on Visual
	Studio generators.
- `evil_audio_setup_google_test()` downloads GoogleTest with `FetchContent`,
	configures the Windows runtime setting, and organizes its static library
	targets with `apply_google_test_static_lib_folder(...)`.
- `evil_audio_find_test_sources(test_sources)` recursively finds test `.cpp`
	files below the current directory's `tests` folder and returns them through
	the parent scope.

### `EvilAudioUtils.cmake`

Contains general target and compiler utilities. Its helpers include:

- `target_link_static_libraries(target ...)`, which validates that each named
	target exists and is a static library before linking it privately.
- `print_target_properties(target)`, which prints target type, sources,
	include directories, compile features, and link libraries for debugging.
- `_print_list_properties(...)`, an internal formatting helper used by target
	inspection.
- `evil_audio_set_msvc_multiprocessor_options()`, which validates
	`EVILAUDIO_MSVC_MAX_PARALLEL_COMPILE` and adds `/MP` or `/MP<n>` for MSVC
	builds when `EVILAUDIO_MSVC_ENABLE_MULTIPROCESSOR` is enabled.

The module also includes `EvilAudioApplicationUtils.cmake`. Its older static
library include is intentionally disabled because JUCE modules can now be
linked directly to application targets.

### `CMakeCheatSheet.md`

Provides a quick reference for CMake linking patterns used by EvilAudio. It
covers local target linking, `PRIVATE`/`PUBLIC`/`INTERFACE` visibility,
platform-specific links, debug/release selection, and common pitfalls.

### `cmakefiles.md`

This file documents the purpose and relationships of every file in the
`extras/cmake` directory. It is documentation only and is not included by
CMake.

## Configuration Flow

The normal root-level flow is:

1. The root `CMakeLists.txt` includes logging, utility, ASIO, test, document,
	 and prerequisite modules.
2. Configure-time commands are dispatched by
	 `evilaudio_handle_configure_time_command(...)`.
3. VS Code PlantUML settings and the project Python environment are prepared.
4. Optional ASIO support is detected.
5. External dependencies and project subdirectories are added.
6. Library, application, test, and documentation modules call the shared
	 helpers described above.

Most helpers operate during configuration rather than build execution. A
failure in dependency discovery, required tooling, or documentation setup can
therefore stop the CMake configure step before compilation begins.
