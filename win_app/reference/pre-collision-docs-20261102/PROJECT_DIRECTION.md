# Roadsters Race: Windows direction


Latest finish evidence: [solo finish/results](docs/FINISH-RESULTS-2026-11-02.md) — native/WASM 10/10; scripted finish/result/restart/menu visual checks and clean shutdown. Full human-driven race, rivals/collision/art remain open; no progression changes.


Latest city evidence: [all-route city clearance](docs/CITY-CLEARANCE-2026-11-02.md) — native/WASM 9/9, bounded Skyline rendering and clean shutdown; complete city art/collision and race acceptance remain open. Locks and progress are unchanged.

Decision record started 2026-11-02. This records the discussion, not a completed implementation.

## Confirmed

- Windows game code: C.
- Game library: raylib.
- First reproduce the browser game's flight handling, combat behavior, and rules. Improve those systems after parity rather than redesign them during the port.
- Match the browser's existing visuals and art direction first. Enhanced Windows graphics are a later phase; keep the renderer structured to allow quality presets without changing gameplay.
- Share as much game code as practical in C between native Windows and a future WebAssembly browser build.
- Share accounts, persistent game data (including progression and purchases), and multiplayer rooms across Windows and browser players. Cross-play is a required destination of the port.
- Keep platform-specific account sign-in, storage, networking transport, and browser integration behind explicit interfaces. Thin JavaScript integration is acceptable where browser APIs or existing services require it; game rules and simulation belong in C.
- Shared C primarily reduces duplicated development and behavior drift. Browser runtime performance must be measured; WebAssembly does not automatically improve GPU-bound rendering.
- Windows is intended to become the main game application.
- All work by this agent stays inside `win_app/`; the surrounding project is available for reading.
- Claude continues handling the browser version.
- User's stated baseline is v0.2.1. The inspected working `game/package.json` declares 0.2.2; the exact reference revision remains to be selected rather than assuming either is a frozen source snapshot.
- Browser development may proceed slowly while the Windows port catches up. There is no agreed calendar deadline or fixed v0.3 release gate.
- The migration should go deep enough to become the main implementation, rather than stop at a desktop wrapper or small demo.

## Migration direction

Bring Windows up to browser feature parity, then lead development in shared C while keeping the browser game current. Transition the browser client to the shared C game via WebAssembly rather than permanently maintain two separate gameplay implementations. Claude continues owning any changes outside `win_app/`.

## Pending decisions

1. Supported Windows versions, minimum hardware, resolution, and frame-rate targets.
2. Final release reference if a specific v0.2.1 release must be used instead of the inspected working source; the initial source reference is captured in `reference/browser-baseline.json`.
3. Installer/signing/update policy and desktop identity/App Check integration details.

## Architecture and milestone plan

The user authorized preparation of the architecture and staged migration plan. See `docs/ARCHITECTURE.md`, `docs/ROADMAP.md`, and `docs/PARITY.md`. The plan captures a source baseline, tracks browser changes at milestone boundaries, proves a C browser build early, and treats shared identity/cross-play as release gates. Technical defaults remain subject to build and integration evidence.

The user also authorized downloading raylib. Release 6.2 and its immutable source commit are pinned in `third_party/raylib.lock.json`, with the official Windows archive SHA-256 and license. Metadata/license retrieval succeeded through the GitHub connector; the installer subsequently downloaded, verified, and extracted the official Windows x64 MSVC package. A C17 dependency check compiles, links, and passes its non-graphics runtime test. See `docs/VALIDATION.md`.

## Examination findings

- The browser client is a Three.js/Vite game; its JavaScript game systems require C implementations.
- There is one Skyline City environment and nine aerial road layouts, with two-lap races, time attack, and online free roam.
- Existing features include ten AI opponents, blaster/seeker/EMP combat, boost/spin dodge, checkpoints, three car models, cockpit views, shop/progression, and keyboard/mouse/gamepad input.
- Three GLB car models are available under `../game/public/models/`. Road coordinates and tuning are in `../game/src/roads.js`, `car.js`, and `ai.js`.
- Much of the city, course geometry, atmosphere, and effects is procedural JavaScript, not a complete ready-to-load map model.
- Firebase handles identity and persistent online data; a WebSocket relay handles free roam. A C client needs explicit integrations and must respect current server contracts.
- Visual Studio Community's MSVC C compiler, CMake, and Windows SDK were used successfully for the C17 raylib dependency check. No game renderer/window has been tested yet.
- No root or Windows-folder AGENTS.md was found during the initial examination; this file's companion AGENTS.md records the user's scope and decisions for future work.

## Current work status

Updated 2026-11-02: both full native and WebAssembly builds compile and pass 8/8 headless tests. The menu now connects the nine-road picker to parameterized track/session initialization, with browser-equivalent lock decisions, session-local lap bests and bounds-based minimaps. The live offline app supplies clean progress (road 1 only); no user storage/profile import or unlock grant occurs. Queued discrete keys fix a concrete within-frame missed-tap case. Native functional observations verify picker navigation/locked refusal, Skyline countdown/start, pause/resume, reset and clean shutdown. This is not full graphics/performance, complete-race/results, controller or Web graphics acceptance. Shared accounts/progression and all-route city clearance remain open. See `docs/ROAD-PICKER-2026-11-02.md`.
