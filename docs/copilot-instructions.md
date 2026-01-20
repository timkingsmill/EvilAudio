# EvilAudio Copilot Instructions

## Overview
- C++23 JUCE-based DAW framework; JUCE modules are wrapped as static libs for reuse across apps/plugins.
- Only active target today is the sample app EvilStaticLibraryClient; other apps/plugins are commented out in [source/CMakeLists.txt](source/CMakeLists.txt).
- JUCE install is expected at [libs/JUCE/install](libs/JUCE/install) and is injected into `CMAKE_PREFIX_PATH` in the root [CMakeLists.txt](CMakeLists.txt).

## Layout you need to know
- Build helpers: [cmake/EvilAudioUtils.cmake](cmake/EvilAudioUtils.cmake) defines `add_juce_module_static_library`, source filtering, and include/definition export.
- Libraries: [libs/EvilAudio/CMakeLists.txt](libs/EvilAudio/CMakeLists.txt) wraps JUCE modules (currently only `juce_core`) into `evil_<module>` static libs.
- Modules: [source/modules/CMakeLists.txt](source/modules/CMakeLists.txt) registers custom modules `evilaudio_core`, `evilaudio_eq`, `evilaudio_lookandfeel` with alias namespace `evilaudio`.
- App example: [source/applications/EvilStaticLibraryClient/CMakeLists.txt](source/applications/EvilStaticLibraryClient/CMakeLists.txt) links against `evil_juce_core`.

## Build workflow (CMake >=4.2)
1) Clone with submodules.
2) Install JUCE: `cmake -B external/JUCE/juce-install -DCMAKE_INSTALL_PREFIX=/source/EvilAudio/libs/JUCE/install` then `cmake --build external/JUCE/juce-install --target install`.
3) Configure: `cmake -B build -DCMAKE_PREFIX_PATH=/source/EvilAudio/libs/JUCE/install` (in-source builds are blocked).
4) Build: `cmake --build build`; to smoke-test, `cmake --build build --target EvilStaticLibraryClient`.
5) ASIO (Windows): if [libs/asiosdk/common](libs/asiosdk/common) exists, `JUCE_ASIO_SUPPORT` is enabled automatically; otherwise a warning is emitted.

## CMake patterns to follow
- Use `add_juce_module_static_library(juce::<module>)` (see [libs/EvilAudio/CMakeLists.txt](libs/EvilAudio/CMakeLists.txt)) to wrap modules; it sets `JUCE_*` compile defs, exports include dirs, and links JUCE-recommended flags.
- Module source selection is platform-filtered via `_should_build_source_file` in [cmake/EvilAudioUtils.cmake](cmake/EvilAudioUtils.cmake) (suffix `_windows/_mac/_linux/_android/_ios` are excluded when host does not match).
- `JUCE_ENABLE_MODULE_SOURCE_GROUPS` can be toggled; default ON at root, OFF for modules list.

## Conventions
- Module naming: `<namespace>_<name>`; static lib target becomes `evil_<name>` (e.g., `juce_core` → `evil_juce_core`).
- Required compile defs on wrapped modules: `JUCE_GLOBAL_MODULE_SETTINGS_INCLUDED=0`, `JUCE_WEB_BROWSER=0`, `JUCE_USE_CURL=0`, `JUCE_INCLUDE_ZLIB_CODE=1`.
- C++23 enforced globally with extensions OFF.

## How to add things
- New JUCE-style module: place code under `source/modules/<mod>/<mod>/`, register in [source/modules/CMakeLists.txt](source/modules/CMakeLists.txt), then wrap in [libs/EvilAudio/CMakeLists.txt](libs/EvilAudio/CMakeLists.txt) via `add_juce_module_static_library(juce::<mod>)`, and link `evil_<mod>` into your app.
- New app/plugin: create `source/applications/<App>/CMakeLists.txt` or `source/plugins/<Plugin>/CMakeLists.txt`, link against the `evil_*` libs, and add the subdirectory to [source/CMakeLists.txt](source/CMakeLists.txt).

## Tips for agents
- Do not edit in-source build paths; CMake guards against it.
- When touching module wrapping, keep the export properties (`INTERFACE_JUCE_MODULE_*`) intact so include paths propagate.
- If build issues arise, verify `CMAKE_PREFIX_PATH` points to [libs/JUCE/install](libs/JUCE/install) and submodules are updated.

Last updated: Jan 2026
