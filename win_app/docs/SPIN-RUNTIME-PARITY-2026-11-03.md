# Active native spin-dodge rules

The native host previously applied spin before its movement step, always rolled right when steering was neutral, and lacked the source camera kick/submessage. Spin now runs after movement/race rules/recovery and Nitro/impact feedback, matching the source's playerWeapons placement. A frame that finishes the race skips the spin request and cooldown update. Pause/countdown/finished/results do not advance active spin cooldown.

An isolated bridge reuses the existing source-tested rr_arsenal spin rule with all other weapon inputs zero. It does not enable production weapons/rivals/coins. Ready spins follow steering sign; only accepted neutral spins consume one random choice. The production host uses a native raylib fair-bit draw. Denied/cooling/steered/no-press cases consume no random value. This matches side-selection semantics, not the browser's entire seeded/global RNG stream.

Accepted spins retain the existing source-equivalent Car.dodge motion: barrel roll, surge, speed floor/multiplier and lateral jink. Camera kick becomes max(existing,0.6). The dynamic caption is SPIN · 1.5× SPEED for0.9s, reusing the imported middle-dot glyph. The multiplication sign already exists in the supplied licensed Orbitron-700.ttf; its codepoint215 is now included in the existing bold atlas. Source warning/start/lap captions remain independent through their existing channels. No extra physics step, texture count, shader, render target, mesh, per-frame allocation or resource-cap increase.

## Fresh evidence

Full native/WASM application builds each passed19/19 focused checks. The new fixture executes the actual playerWeapons spin block, Car.dodge/barrelRoll and constants across192 cooldown/dt/steering/RNG-boundary/active/kick/press cases. It checks side, random demand, denial, cooldown, car velocity/speed/roll/surge, camera kick and UTF8 caption. An actual native finish-crossing fixture checks that simultaneous spin is ignored without RNG/cooldown changes. Existing arsenal, fire-dispatch RNG and full offline player-tick tests passed again alongside the15 previous focused tests. These are CPU fixtures, not live combat activation or a rerun of all55 declared tests.

Exact current local v0.2.9 aim.js/car.js identities: reference/ui-parity-20261103/spin-control-oracle.json. Changed-file hashes, build logs and executable/447 matching asset-copy identities: spin-runtime-implementation.json. Six preceding impact/session/camera helper/fixture/tool files remain byte-identical. Originals, including previous font loader/tests, submessage header and launcher, are preserved under reference/pre-spin-runtime-20261103. Initial native/WASM tests caught31 caption mismatches from ASCIIx versus source multiplication sign; the corrected UTF8 caption and actual font-glyph lookup/atlas are tested. Failed pre-multiplyfix logs are retained separately.

Intermediate font tests assumed96 requested glyphs meant96 returned; raylib returns95 available glyphs for this request. The corrected assertions inspect the actual U+00D7 raster and lookup rather than requiring unsupported existing ASCII coverage. Pre-fontcountfix logs are also retained. Final CPU font atlases remain1024x512 for each Orbitron weight and512x256 for Rajdhani:6,815,744 RGBA8 bytes total,6,842,804 including both fallback crops. Dimensions and tested bound are unchanged.

## Remaining limits

Boost/deny sounds, cyan HUD flash, live spin broadcasts, exact browser-global RNG ordering and actual native control/roll appearance remain open. Full-pack integration must replace this isolated bridge and avoid stepping spin/cooldown/RNG twice. Accounts/progression and inactive production combat remain unchanged.

Run-RoadstersRace.cmd selects build/spin-runtime-native/bin/RoadstersRace.exe only after verification, with prior builds preserved. Launcher target/working directory are statically checked; no game/launcher execution. The prior automatic-review rejection under AGENTS.md remains pending explicit graphics authorization, with no retry. No native visual/stability/performance or human race acceptance is claimed.
