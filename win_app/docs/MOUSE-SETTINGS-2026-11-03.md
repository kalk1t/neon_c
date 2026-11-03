# Mouse sensitivity and invert Y

The native menu now exposes MOUSE SENS (0.3 through2.5, step0.05) and INVERT Y, using the current browser's controls and updateAim behavior. Default remains1.0/normal Y. Dragging sensitivity continues outside the track with clamped endpoints until release, and focus loss cancels the drag. Settings survive session restarts/menu returns in the same process, without changing saved data or account progress.

The previous hardcoded mouse calculation moved into shared C. Sensitivity multiplies both steering aim and RMB orbit. Invert reverses vertical aim and vertical orbit, retaining the browser's different signs for each. Existing keyboard/gamepad aim reset, angle wrapping, pitch/orbit bounds and steering/climb fallback remain source-defined. No combat activation or platform capture changes.

## Fresh evidence

Native and WASM full application builds each passed12/12 focused checks. The new test covers384 sequential source updateAim states in12 settings/look sequences, min/default/max sensitivity, both inversion modes, large deltas, keyboard/gamepad-like overrides, angle wrapping and pitch clamps. It also checks all45 slider positions and clamped endpoints. The source function and mouse-rate constant were executed in an isolated Node VM; no account/storage/graphics code ran. The other11 focused UI/session/result/guidance/announcement tests passed again. This is not a rerun of all53 declared tests.

Local browser package v0.2.9; exact aim.js and markup identities are in reference/ui-parity-20261103/mouse-aim-oracle.json and mouse-settings-implementation.json. The latter records changed-file hashes, logs and executable/447 matching asset-copy hashes. Four preceding announcement helper/fixture/tool files remain unchanged. Originals and prior launcher are preserved under reference/pre-mouse-settings-20261103. Existing renderer caps and cockpit defaultOFF are unchanged; no additional asset resources.

## Remaining limits

Settings are process-local, unlike browser localStorage preferences. Native persistence is still a parity gap. Real slider/checkbox input, keyboard focus/accessibility, appearance and DPI behavior have not been observed. Browser mouse-event capture/spike suppression is not equivalent to filtering raylib's accumulated per-frame deltas and remains a separate host-input gap. Tests validate supplied deltas and aiming math, not actual pointer capture or controller hardware. Production combat remains inactive.

After verification, Run-RoadstersRace.cmd selects build/mouse-settings-native/bin/RoadstersRace.exe with that working directory. All prior clients are retained. Verification is static only; no launcher/game execution. Native visual qualification remains pending the explicit authorization required after the earlier automatic-review rejection under AGENTS.md. No rejected operation was retried.
