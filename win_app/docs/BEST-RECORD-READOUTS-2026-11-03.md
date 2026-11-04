# Consistent per-road best-lap readouts

The native home menu now shows BEST LAP and the current road's formatted time, with the source-style magenta value. The road picker now uses BEST LAP mm:ss.xx rather than raw seconds. Empty values show --:--.-- on home and NO LAP YET in picker; locked rows retain WIN ROAD n IN1ST even when a synthetic fixture contains a best value. The existing HUD best sidebar now uses the same read-only adapter, so all three views read the same current per-road cache.

The menu reserves24px for the new line and recenters its existing stack. Settings and control/footer text retain their space at the supported native minimum window. This does not add native persistence: existing process-local records survive session restart/mode changes but still reset with a new process. No shared browser records, account data or unlocks are read/imported/written.

## Fresh evidence

Full native and WASM application builds each passed26/26 focused checks. New tests compare35 actual current browser strings from the home roadUI assignments, road-picker best template and HUD fmt, using isolated read-only storage values. They cover zero/positive finite times, rounding/minute boundaries and roads1/2/9 with locked/unlocked presentation. Synthetic locked records do not change production road availability.

Native core integration checks readout changes after improved/slower laps, restart and race/time-attack switching; choosing locked road2 still fails and leaves road1/open_count unchanged. Helper reads leave record/selection/progression fields unchanged. New footer containment and existing menu layout/hit-region checks run alongside all24 preceding focused regressions. The full61 declared tests were not rerun.

Local browser remains v0.2.11. Exact source identities/scope: reference/ui-parity-20261103/best-readouts-oracle.json. Changed-file hashes, fresh logs and executable/447 matching asset-copy identities: best-readouts-implementation.json. Six preceding reticle helper/fixture/tool files remain unchanged. Originals and previous launcher: reference/pre-best-readouts-20261103.

## Limits and launcher

The available native700 font is used for the source600 label; the value uses900. Actual typography, trailing-spacing alignment, native appearance and menu interaction remain visually unverified. This native footer adaptation does not establish complete browser home/paint/shop layout parity. Persistence/shared account records remain separate integration work; no record migration or grants occurred.

No new assets, font atlases, shaders, textures, meshes or render targets; existing caps and safety settings remain. After verification, Run-RoadstersRace.cmd selects build/best-readouts-native/bin/RoadstersRace.exe with that working directory. All prior clients/launcher backups are retained. Static target checks do not execute it. The earlier automatic-review graphics rejection under AGENTS.md remains pending explicit authorization; no retry or game launch occurred.
