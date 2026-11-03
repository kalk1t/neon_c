# Unified race announcement channel

Windows previously displayed countdown and warnings through separate timers and omitted the browser's large lap/finish messages. A fixed shared-C message state now replaces text, warning style and duration atomically, matching HUD.message. Countdown/GO, FINAL LAP, ordinary LAP n in time attack, solo FINISH, MISSED GATE, WRONG WAY and SIGNAL LOST all use this channel. The independent checkpoint/best-lap submessage remains separate, as in the browser.

Messages expire on the prepaint HUD tick, including pause; menu/start retain existing HUD content and timer. A new non-warning clears warning styling. Lap messages last1.6s, solo finish3s, and existing source warning/countdown durations are retained. Wrong-way recovery now receives this same message timer, so it waits while a lap/countdown/finish message is active rather than consulting only the old warning timer. Existing two-second results transition, session/restart cancellation, progress and locks are unchanged.

## Evidence

Full native and WASM application builds each passed11/11 focused checks. The new fixture has76 states from the actual HUD.message/update timer and source lap/solo-finish announcement statements, covering overwrite, expiry, reset/menu retention, warning-style replacement and finite/infinite lap naming. A separate native coupling assertion checks wrong-way suppression during a lap message and eligibility after expiry. These tests do not execute completeLap's storage/reward branch. Ten preceding focused guidance/UI/session/result checks passed again; all52 declared tests were not rerun.

The local browser package is v0.2.9; exact race.js/hud.js hashes and scope are in reference/ui-parity-20261103/hud-message-oracle.json. Machine-readable changed-file hashes, fresh logs and client/447 asset-copy identities are in race-announcements-implementation.json. Four preceding guidance helper/fixture/tool files remain unchanged. Original main.c/CMakeLists.txt and launcher are preserved under reference/pre-race-announcements-20261103.

## Limits and launcher

Production remains solo/offline. Ranked place announcements, combat/wreck/takedown messages, accounts, rewards and live rivals are unconnected. Native warning typography uses the source84px/pink style, but CSS fade, glow, pixel alignment and actual presentation remain visually unverified. No new texture/shader/mesh/render target or cap increase.

After successful verification, Run-RoadstersRace.cmd selects build/race-announcements-native/bin/RoadstersRace.exe with that working directory. Previous clients/launcher are preserved. Static identity verification does not execute the launcher. The prior automatic-review rejection of graphics execution under AGENTS.md remains pending explicit authorization; no retry or graphics launch occurred.
