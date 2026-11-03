# Roadsters Race: Windows direction

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

Updated after session recovery on 2026-11-02: a first-flight C prototype, native package, WebAssembly target, and headless browser-reference tests exist. Full parity is incomplete. The user reports a whole-PC freeze after launching the earlier native package. Graphics execution is suspended without new explicit authorization. Revised rendering adds deferred scene loading, a 2D menu/pause, visibility filtering, cached building meshes, bounded uploads/resolution, conservative defaults, and persistent native diagnostics. Compilation and headless checks do not establish graphics stability or performance. See `docs/RECOVERY-2026-11-02.md` and `docs/RENDERING.md`.
