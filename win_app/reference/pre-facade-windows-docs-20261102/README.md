# Roadsters Race: shared C game

Latest bounded city evidence: [canonical core and source crowns](docs/CANONICAL-CITY-CORE-2026-11-02.md) - native/WASM 15/15; 96 prototype buildings replaced by 295 canonical buildings, 86 crown anchors, two native trials and verified shutdown. Fixed hero-camera occlusion remains unresolved; full city/material/lighting parity is open.


Latest hero evidence: [existing control-tower asset](docs/CONTROL-HERO-2026-11-02.md) — native/WASM 14/14; source fit/material subset and two scoped native views with verified shutdown. Canonical city/crowns and whole-scene PBR/lighting remain open; locks/progress unchanged.


Latest shared-scenery evidence: [platforms, trees and source material inputs](docs/SHARED-SCENERY-2026-11-02.md) — native/WASM 13/13; existing cyan-tree model, normal native render/pause and clean shutdown. Full city/PBR/lighting and manual race acceptance remain open; locks/progress unchanged.


Latest course evidence: [selected-road furniture](docs/COURSE-FURNITURE-2026-11-02.md) — native/WASM 12/12; existing prefab geometry, normal Skyline render/start/pause and clean shutdown. Shared platforms/trees, city/materials and manual race acceptance remain open; locks/progress unchanged.


Latest collision evidence: [scoped solid-world collision](docs/COLLISION-2026-11-02.md) — native/WASM 11/11; scripted post push-out/rebound/pause and clean shutdown verified. Full furniture/city and human-driven race acceptance remain open; no progress changes.


Latest finish evidence: [solo finish/results](docs/FINISH-RESULTS-2026-11-02.md) — native/WASM 10/10; scripted finish/result/restart/menu visual checks and clean shutdown. Full human-driven race, rivals/collision/art remain open; no progression changes.


Latest city evidence: [all-route city clearance](docs/CITY-CLEARANCE-2026-11-02.md) — native/WASM 9/9, bounded Skyline rendering and clean shutdown; complete city art/collision and race acceptance remain open. Locks and progress are unchanged.

Windows is the main target. The future browser client will compile the same C game to WebAssembly. C + raylib, browser gameplay and visual parity first, shared accounts/data/rooms, then improvements.

This directory contains an offline Skyline trial/time attack, a connected nine-road picker with preserved locks, and shared C geometry for all nine browser roads. Both latest full builds pass 15/15 headless tests, including source-fitted control hero/resource guards, shared scenery, road furniture, collision and solo race lifecycle. Native Enter/navigation, locked refusal, Skyline countdown/start, pause/resume/reset and clean shutdown have fresh functional evidence. Only road 1 is selectable until native progress/profile integration exists; no unlocks or user-data writes were added. Complete race/results, broader art/input/performance and Web graphics qualification remain open. See [current road-picker implementation and evidence](docs/ROAD-PICKER-2026-11-02.md), [nine-road geometry](docs/ALL-ROADS-2026-11-02.md), and [route recovery](docs/ROUTE-RECOVERY-2026-11-02.md).

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

