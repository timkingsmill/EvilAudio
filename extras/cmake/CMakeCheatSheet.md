# CMake `target_link_libraries` Cheat Sheet
> Quick reference for common CMake linking patterns used in **EvilAudio** (and compatible with JUCE-based projects).

## What this covers

- Linking against local library targets
- Choosing `PRIVATE` / `PUBLIC` / `INTERFACE`
- Platform-conditional links
- Debug vs release link selection
- Common pitfalls and fixes

## Basic syntax

```cmake
target_link_libraries(<target>
   <PRIVATE|PUBLIC|INTERFACE> <item>...
   [<PRIVATE|PUBLIC|INTERFACE> <item>...]
)
```

- `<target>` must be created first (for example by `add_library()` or `add_executable()`).
- `<item>` can be:
  - another CMake target (preferred),
  - a plain library name (`ws2_32`),
  - a full path to a library file,
  - a generator expression.

## Scope keywords (`PRIVATE`, `PUBLIC`, `INTERFACE`)

Think of link dependencies as usage requirements:

- `PRIVATE`: used to build this target only.
- `PUBLIC`: used to build this target and propagated to targets that link this target.
- `INTERFACE`: not used to build this target; only propagated to dependents.

### Mental model

If `App -> MyLib -> Dependency`:

- `MyLib PRIVATE Dependency`: `App` does **not** automatically link `Dependency`.
- `MyLib PUBLIC Dependency`: `App` gets `Dependency` transitively.
- `MyHeaderOnly INTERFACE Dependency`: dependents get `Dependency` even though `MyHeaderOnly` has no compiled objects.

## Preferred style: link CMake targets, not raw paths

```cmake
# Good: target-to-target link
target_link_libraries(EvilStaticLibraryClient
 PRIVATE
  evil_juce_core
)
```

Why this is preferred:

- Carries include directories, compile definitions, and transitive links automatically.
- More portable across toolchains and platforms.
- Easier to refactor than hard-coded `.lib`/`.a` paths.

## Common patterns

### 1) Executable links a project library

```cmake
add_executable(MyApp main.cpp)
target_link_libraries(MyApp PRIVATE evil_juce_core)
```

### 2) Library exposes a dependency to consumers

```cmake
add_library(AudioEngine STATIC engine.cpp)
target_link_libraries(AudioEngine
   PUBLIC
      evil_juce_core
)
```

### 3) Header-only library

```cmake
add_library(EvilHeaders INTERFACE)
target_include_directories(EvilHeaders INTERFACE include)
target_link_libraries(EvilHeaders INTERFACE evil_juce_core)
```

### 4) Platform-conditional system libraries

```cmake
if(WIN32)
   target_link_libraries(MyApp PRIVATE ws2_32 winmm)
elseif(APPLE)
   target_link_libraries(MyApp PRIVATE "-framework CoreAudio")
elseif(UNIX)
   target_link_libraries(MyApp PRIVATE pthread)
endif()
```

### 5) Generator expressions for fine-grained conditions

```cmake
target_link_libraries(MyApp
   PRIVATE
      $<$<PLATFORM_ID:Windows>:ws2_32>
      $<$<CONFIG:Debug>:MyDebugHelper>
)
```

## Debug vs release libraries

Modern CMake usually prefers imported targets over `debug/optimized` keywords. If needed, this still works:

```cmake
target_link_libraries(MyApp
   PRIVATE
      debug MyLib_d
      optimized MyLib
)
```

Equivalent generator-expression style:

```cmake
target_link_libraries(MyApp
   PRIVATE
      $<$<CONFIG:Debug>:MyLib_d>
      $<$<NOT:$<CONFIG:Debug>>:MyLib>
)
```

## Order, repetition, and calls

- You may call `target_link_libraries()` multiple times for the same target.
- Link items are appended in call order.
- Repetition is generally harmless, but avoid unnecessary duplicates.

## Typical mistakes (and fixes)

### Mistake: linking before target exists

```cmake
# Wrong
target_link_libraries(MyApp PRIVATE evil_juce_core)
add_executable(MyApp main.cpp)
```

Fix: define the target first.

### Mistake: using `INTERFACE` on compiled implementation deps

If code in a `.cpp` directly uses a dependency, it should usually be `PRIVATE` or `PUBLIC`, not only `INTERFACE`.

### Mistake: hard-coding platform-specific library file paths

Prefer package/imported targets from `find_package()` and project targets.

## JUCE / EvilAudio notes

- In this repo, wrapped JUCE module libs like `evil_juce_core` are target names and should be linked directly.
- Use `PRIVATE` for app-only dependencies (typical for executables).
- Use `PUBLIC` only when your library API/ABI requires consumers to also link that dependency.

## Quick checklist

- Target exists before linking.
- Prefer target names over raw library file paths.
- Choose `PRIVATE` / `PUBLIC` / `INTERFACE` intentionally.
- Keep platform-specific links guarded (`if(WIN32)` or generator expressions).
- Verify transitive behavior when creating reusable libraries.

---

# CMake `target_include_directories` Cheat Sheet
> Quick reference for include-path usage requirements in **EvilAudio** and other modern CMake projects.

## What this covers

- Adding include paths to targets
- Choosing `PRIVATE` / `PUBLIC` / `INTERFACE`
- Build vs install include paths
- System include directories
- Common pitfalls and fixes

## Basic syntax

```cmake
target_include_directories(<target>
   [SYSTEM] [AFTER|BEFORE]
   <PRIVATE|PUBLIC|INTERFACE> <dir>...
   [<PRIVATE|PUBLIC|INTERFACE> <dir>...]
)
```

- `<target>` must already exist.
- `<dir>` can be absolute, relative (to current source dir), or a generator expression.
- Prefer target-based include propagation instead of directory-global `include_directories()`.

## Scope keywords (`PRIVATE`, `PUBLIC`, `INTERFACE`)

- `PRIVATE`: used only when compiling this target.
- `PUBLIC`: used for this target and propagated to dependents.
- `INTERFACE`: not used to compile this target, only propagated to dependents.

### Mental model

If `App -> MyLib`:

- `target_include_directories(MyLib PRIVATE include)` means `App` does **not** inherit that include path.
- `target_include_directories(MyLib PUBLIC include)` means `App` does inherit it.
- `target_include_directories(MyHeaders INTERFACE include)` is ideal for header-only libraries.

## Common patterns

### 1) Private implementation includes

```cmake
add_library(AudioEngine STATIC src/engine.cpp)
target_include_directories(AudioEngine
   PRIVATE
      src
)
```

### 2) Public API headers

```cmake
add_library(EvilAudioCore STATIC src/core.cpp)
target_include_directories(EvilAudioCore
   PUBLIC
      include
)
```

### 3) Header-only library

```cmake
add_library(EvilHeaders INTERFACE)
target_include_directories(EvilHeaders
   INTERFACE
      include
)
```

### 4) Build-tree vs install-tree paths (recommended for reusable libs)

```cmake
target_include_directories(EvilAudioCore
   PUBLIC
      $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
      $<INSTALL_INTERFACE:include>
)
```

### 5) Mark third-party headers as system headers

```cmake
target_include_directories(MyApp
   SYSTEM PRIVATE
      ${CMAKE_CURRENT_SOURCE_DIR}/external/somelib/include
)
```

`SYSTEM` often reduces warning noise from external headers.

## `BEFORE` and include order

- CMake appends include dirs by default.
- Use `BEFORE` to prepend paths when order matters:

```cmake
target_include_directories(MyApp BEFORE PRIVATE include)
```

Use include-order overrides sparingly; it can hide design issues.

## Typical mistakes (and fixes)

### Mistake: using global `include_directories()` everywhere

Fix: move includes to per-target `target_include_directories()` for predictable propagation.

### Mistake: exposing private implementation paths as `PUBLIC`

Fix: keep internal headers in `PRIVATE`; expose only stable API include roots as `PUBLIC`/`INTERFACE`.

### Mistake: missing install interface for exported libraries

Fix: use `$<BUILD_INTERFACE:...>` and `$<INSTALL_INTERFACE:...>` for relocatable packages.

### Mistake: relative paths that depend on caller location

Fix: prefer `${CMAKE_CURRENT_SOURCE_DIR}`-based absolute paths for clarity.

## JUCE / EvilAudio notes

- Prefer linking wrapped module targets (for example `evil_juce_core`) so include dirs propagate automatically.
- Only add manual include directories when a dependency is not represented as a proper CMake target.
- For project modules under `source/modules`, expose consumer-facing headers via `PUBLIC`/`INTERFACE`, keep implementation paths `PRIVATE`.

## Quick checklist

- Target exists before setting include dirs.
- `PRIVATE` / `PUBLIC` / `INTERFACE` selected intentionally.
- Prefer target-based propagation over global include settings.
- Use `BUILD_INTERFACE`/`INSTALL_INTERFACE` for installable libs.
- Use `SYSTEM` for external headers when warning suppression is desired.

