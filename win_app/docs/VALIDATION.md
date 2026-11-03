# Initial validation: 2026-11-02

This initial report below is historical. Work later reached a first-flight
prototype. After a reported whole-PC freeze, see `RECOVERY-2026-11-02.md`
and `RENDERING.md` for current state, checks, and execution constraints.
The revised renderer is compiled and headless-tested only; historical
captures do not validate it.

## Passed

- Both PowerShell setup/reference scripts passed parser validation.
- Browser reference capture recorded 94 source/content hashes, checked twice for consistency during capture. Observed package version: 0.2.2; Git commit: `71c030227d4148d2853a0f7d93dec18bd03bf3d0`. The working tree was dirty and its status is recorded in the manifest. No browser file was changed by this work.
- Official raylib 6.2 Windows x64 MSVC archive downloaded and installed inside `win_app/third_party/`.
- Archive size: 2,598,891 bytes. SHA-256: `8467b5dd6db4d7ab942f3148a3d30d93be6ed9b2fab88069d177e59604268ebd`, matching official release metadata.
- Installer `-ValidateOnly` rechecked the cached archive successfully.
- `tools/dependency-check/` compiled as C17, linked the downloaded static raylib library, and passed `raylib_link_and_runtime` (1/1 CTest tests). Compiler warnings are enabled as errors for the check source.

Toolchain: CMake 4.4.0, MSVC 19.51.36260 targeting x64, Windows SDK 10.0.28000.0, NMake generator. The prebuilt raylib package requires the dynamic C runtime; the check explicitly uses `/MD`.

## Reproduce the dependency build

From a Visual Studio **x64 Native Tools Command Prompt**, change to `win_app/`, then run:

```text
cmake -S tools/dependency-check -B build/dependency-check-nmake -G "NMake Makefiles" -DCMAKE_BUILD_TYPE=Release
cmake --build build/dependency-check-nmake
ctest --test-dir build/dependency-check-nmake --output-on-failure
```

The default dependency path points to the installer output. This is a separate dependency-check project, not the future game's top-level build.

## Environment finding

The Visual Studio CMake generator's compiler-identification build failed in MSBuild's file-tracking initialization with `E_ACCESSDENIED`. The NMake generator with the normal Visual Studio developer environment successfully compiled and linked with the same compiler. Use the working NMake route in this environment; no system configuration was changed.

## Not yet validated

Window creation, GPU/OpenGL support, asset loading, rendering/audio, game simulation, a WebAssembly toolchain/build, desktop identity/App Check, real backend calls, multiplayer, frame time, packaging, and browser replacement. W0 remains partly complete; no later milestone is complete.
