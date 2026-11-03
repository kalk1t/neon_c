# Roadsters Race: shared C game

Windows is the main target. The future browser client will compile the same C game to WebAssembly. C + raylib, browser gameplay and visual parity first, shared accounts/data/rooms, then improvements.

This directory contains a first-flight C prototype, native and WebAssembly build targets, browser reference fixtures, and the migration plan. It is not full browser parity. After a reported whole-PC freeze, graphics execution is suspended pending explicit user authorization; current changes are verified by compilation and headless tests only. See [recovery](docs/RECOVERY-2026-11-02.md) and [renderer changes and validation limits](docs/RENDERING.md).

- [Confirmed direction](PROJECT_DIRECTION.md)
- [Architecture](docs/ARCHITECTURE.md)
- [Milestones and acceptance gates](docs/ROADMAP.md)
- [Browser feature parity](docs/PARITY.md)
- [Captured browser reference](reference/browser-baseline.json)
- [raylib dependency setup](third_party/README.md)

All writes stay inside `win_app/`. The browser working tree and live deployment remain owned by Claude. Build and integration changes needed there are described as handoffs, not made here.

## Dependency setup

The Windows bootstrap pins the official raylib 6.2 x64 MSVC archive and verifies its published SHA-256 before extraction. From this directory:

```powershell
.\scripts\Install-Raylib.ps1
```

An already downloaded archive can be supplied without network access:

```powershell
.\scripts\Install-Raylib.ps1 -ArchivePath .\downloads\raylib-6.2_win64_msvc16.zip
```

If local PowerShell policy blocks script files, invoke the reviewed script with a process-only policy override; this does not change system policy:

```powershell
powershell.exe -NoProfile -ExecutionPolicy Bypass -File .\scripts\Install-Raylib.ps1
```

Capture a new browser reference only when intentionally updating the porting baseline:

```powershell
powershell.exe -NoProfile -ExecutionPolicy Bypass -File .\scripts\Capture-BrowserBaseline.ps1
```

The installer only prepares the library; it does not build the game or install a global SDK. Windows source builds and Emscripten builds will use the same pinned raylib source revision when the build targets are implemented.

raylib 6.2 is installed locally and its official archive hash is verified. The C17 dependency check compiles, links the static raylib library, and passes its runtime test. It does not open a window or validate GPU rendering. See [the validation record](docs/VALIDATION.md) for commands, toolchain details, and remaining work.

