# Fallback glyphs and race submessages

The Windows client now uses the supplied browser-derived middle dot and star sprites for checkpoint and lap submessages. It retains fractional advances and baseline-relative crop bearings, tints white cores cyan, and uses the existing native glow approximation. Orbitron text remains dynamic; no complete system fonts were copied. This is a local development import with the package's original provenance and rights conditions retained, not external redistribution approval.

Checkpoint captions show the count and elapsed lap time. Lap captions use the star for a new volatile per-road best. The sidebar now reads that same per-road record across restarts. Caption replacement, timer decay and expiry follow the source. Accounts, persistent records and progression are unchanged. The independent large lap/finish announcement channel still has parity gaps.

## Evidence

- Full native and WASM application builds passed. Each passed 9/9 focused tests, including 92 captured source caption/timer states, CPU font/glyph decoding and layout, session, picker and results regressions.
- Independently verified 40 manifest entries in both supplied folder and archive, including lengths and SHA256; imported two unchanged 3x white-core PNGs.
- Two glyph textures add 27,060 decoded RGBA8 bytes. Existing CPU font atlas plus glyph bound is 6,842,804 bytes, within its 16 MiB test bound. No mesh/resource cap increase.
- All 447 source assets match their copies beside the latest native executable. Five preceding camera-FX implementation/evidence files remain byte-identical, excluding deliberately edited main.c and CMakeLists.txt.
- Fixtures were captured against local v0.2.8. Package version now reports v0.2.9; race.js and hud.js still match the fixture hashes. This does not establish whole-v0.2.9 parity.
- Initial WASM preload-option link failure was corrected using grouped CMake SHELL options; the failed log and intermediate evidence remain preserved. Latest logs are fallback-subtext-native-build.log and fallback-subtext-web-build.log under reference/ui-parity-20261103.

Machine-readable evidence: reference/ui-parity-20261103/fallback-subtext-implementation.json. Package audit/provenance: reference/fallback-glyph-import-20261103 and assets/ui-fallback/PROVENANCE.txt. Original main.c/CMakeLists.txt: reference/pre-subtext-20261103.

## Limits

No native graphics execution occurred. GPU texture upload, mixed-font baseline, resampling at real DPI, glow and complete line appearance remain unverified. The portable fallback covers only these two symbols, not arbitrary Unicode. The complete 50-test suite was not rerun; these fresh nine checks and prior complete/focused results have separate scopes. No rendering stability or human race acceptance is claimed. The prior rejected graphics launch remains pending explicit authorization, with no retry.
