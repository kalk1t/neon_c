# Device-local units persistence and cockpit binding, 2026-11-04

Reference: local browser v0.2.16 `src/main.js` setUnits / neon.units / dash.setUnit and `src/hud.js` SPEED_UNITS; source hashes verified unchanged from the preceding speed-units slice. No deployed version was checked.

Active main loads the unit at startup and saves changes using the existing guarded blob I/O: 0.25s coalescing, forced save on menu exit, blur, minimize and shutdown, atomic native replacement, conflict detection and 2s write retry. Native raw `kmh`/`mph` is stored separately in `%LOCALAPPDATA%/RoadstersRace/speed-units-v1.txt`. WebAssembly uses the browser-compatible `neon.units` key. Mouse settings, accounts, rewards and progression are separate. No file is created or rewritten at startup; malformed/oversized values fall back to KM/H read-only, preserving data rather than applying the browser's startup normalization. Device-settings save failures are exposed in the existing menu warning.

The disabled cockpit candidate now has a unit-aware readout helper and renderer label. Its source main binding uses the same total-speed conversion as the HUD. The preview remains OFF; no graphical or enabled-preview-main acceptance is claimed.

Native and WASM each passed all 140 CPU tests after the final incremental rebuild. Expanded isolated preference tests cover save/reload, delay/force, startup non-write, conflicts, malformed/oversized preservation, canceled changes and platform write failure/retry. Native tests use only build-fixture files; Web tests use a fresh in-memory localStorage mock, including the neon.units compatibility check. No real profile/store or graphics/audio device was used. Existing cockpit fit/readout fixtures plus new unit checks passed.

Originals and initial full-suite logs remain separate from final rebuild/full-suite logs. Static review corrected a default cockpit-label initialization, raw-read terminator space and native environment length guard before final verification. The preceding EXE and 455 asset hashes were preserved/verified before launcher promotion.

Remaining: native pixels, hardware input, DPI and a real application restart are gated. The native menu uses a toggle rather than the browser select popup. Enabled cockpit visuals, full-pack production activation, reward/account integration and missing 12px caption glyph evidence remain unqualified.
