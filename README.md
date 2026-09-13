# Evil Audio Documentation

![EvilAudio logo: A red glowing skull with demonic horns wearing headphones, centered within concentric circular rings, set against a dark background. The text EVIL AUDIO appears below the skull in bold red letters. The overall tone is intense and edgy.](assets/EvilAudioLogo.png)

## EvilAudio

EvilAudio is a powerful audio processing library for Python that provides a wide range of tools and functionalities for working with audio data. It aims to make audio manipulation and analysis easy and efficient for developers and researchers alike.

## PlantUML Theme Include

Use the shared PlantUML theme include for all repository diagrams to keep styling consistent.

```plantuml
@startuml
!include docs/puml/themes/EvilTheme.puml
...
@enduml
```

If a `.puml` file lives in a nested directory, use a relative path to `docs/puml/themes/EvilTheme.puml` from that file's location.

## CMake Target Dependencies

```plantuml
!include docs/tools/plantUML/cmake-targets.puml
```

### Regenerate Graph

```powershell
cmake -S . -B build
powershell -ExecutionPolicy Bypass -File .\docs\tools\plantUML\generate-cmake-targets.ps1
```

This runs `docs/tools/plantUML/generate-cmake-targets.ps1` and refreshes `docs/tools/plantUML/cmake-targets.puml`.

## Best-Practice MVC Example

The repository includes a runnable console sample that executes the end-to-end
best-practice MVC flow:

- Target: `ExampleMvcApp`
- Source: `source/examples/StaticLibraryClients/ExampleMvcApp/example_mvc_app.cpp`

Observer registration guidance:

- Preferred: `AppModel::subscribe(IModelObserver&)` and keep the returned
  `ObserverSubscription` token alive.
- Compatibility/manual control: `addObserver()` and `removeObserver()`.

Example observer lifecycle:

```cpp
evil::mvc::AppModel model;
evil::mvc::MainView view;

// Keep this token alive while callbacks are needed.
auto subscription = model.subscribe(view);

// ... run application logic ...

// Optional early unsubscribe before scope end.
subscription.reset();
```

### Configure

```powershell
cmake -S . -B build
```

### Build only the example target

```powershell
cmake --build build --target ExampleMvcApp --config Debug
```

### Run

```powershell
& "build/source/examples/StaticLibraryClients/ExampleMvcApp/Debug/ExampleMvcApp.exe"
```

Expected behavior:

- Prints model/view example status values.
- Asserts that the happy path succeeds.
- Asserts that the invalid path emits at least one validation error.

## MVC Library Docs

Detailed documentation for `evil_mvc_lib`:

- Architecture: `source/libs/evil_audio_libs/evil_mvc_lib/ARCHITECTURE.md`
- Use cases: `source/libs/evil_audio_libs/evil_mvc_lib/USE_CASES.md`

## Doxygen Output

After running the `Generate Doxygen Docs` task, open:

- Docs home: `docs/evil_audio_api/html/index.html`
- `evil_mvc_lib` page (contains the interaction UML graph): `docs/evil_audio_api/html/group__evil__mvc.html`

## Doxygen Comment Lint Checklist

Use this quick checklist before opening a PR that changes C++ headers or source files.

### Required per file

- Add one file block near the top of each .h/.cpp file:
  - `@file <filename>`
  - `@brief <single-sentence summary>`
- Add `@details` when intent or behavior is non-obvious.

### Required per public API symbol

- Include `@brief` for each public class, struct, enum, and function.
- Add `@param <name>` for every function parameter.
- Add `@return` for every non-void function.
- Keep terminology consistent:
  - Prefer: `@return true if ...; otherwise false.` for boolean returns.
  - Prefer: `@return <object/value meaning>.` for non-boolean returns.

### Consistency rules

- Keep wording imperative and concise.
- Use the same parameter names in docs as in function signatures.
- Keep punctuation consistent across tags.
- Do not document internal/private helpers unless they are intentionally part of generated API docs.

### Fast pre-PR check

- [ ] Every edited .h/.cpp has `@file` and `@brief`.
- [ ] New/changed public APIs include complete `@param` and `@return` coverage.
- [ ] Booleans use normalized return wording.
- [ ] Doxygen generation succeeds with no new warnings for changed files.

### Validation step

- Run the VS Code task: `Generate Doxygen Docs`.
- Verify updated pages render under `docs/evil_audio_api/html/`.
