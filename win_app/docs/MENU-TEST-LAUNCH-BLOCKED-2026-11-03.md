# Menu test launch blocked, 2026-11-03

The owner checked Task Manager Details and confirmed: `no RoadstersRace.exe is present`. This resolves the previous unknown process state through the requested owner observation. A fresh supported desktop inventory showed no native Roadsters window. Browser tabs with Roadsters titles were distinguished from the native application and left untouched.

Preflight read `AGENTS.md` and the existing executable metadata. Candidate: `build/v3-ui-corrections-native/bin/RoadstersRace.exe`, 941568 bytes, modified 2026-11-03 07:27:36 UTC. Its CMake cache records Release and `RR_COCKPIT_PREVIEW=OFF`. Source retains 960x720 resizable/vsync startup without MSAA. Planned test was menu/track-selection only; no race/world launch, smoke/fallback mode, account writes or new feature activation.

Exactly one supported launch was attempted with Computer Use `cua.computer.launch_app`, targeting that absolute executable path. Automatic approval review rejected it **before execution**:

> Launching the native graphics build is explicitly prohibited by AGENTS.md after a prior whole-PC freeze unless the user separately authorizes execution; confirming no process remains does not authorize the launch.

No workaround, retry, second instance, termination or security change followed. The tool call stopped at the rejected launch, so its subsequent window-list statement did not run. The menu/track-selection test is **not run**, not failed. No rendering, responsiveness, clean-shutdown, GPU or visual-parity result was established. No shutdown was required from this rejected attempt because the application was not launched.

The larger conversation contains prior authorizations, but this action was still rejected. Another attempt should wait for materially new explicit owner authorization naming the native graphics test, rather than resubmitting the same evidence or changing launch mechanisms.

Smallest owner approval needed: `I explicitly authorize launching the Windows Roadsters build for the bounded menu and track-selection graphics test despite the previously reported freeze. Do not start a race.` The risk is that opening the native OpenGL window could again cause severe lag or a whole-PC freeze even though this test does not load the 3D world. No request to disable or change security protections is needed.

Independent asset/source audit completed in the previous reassessment remains valid: 456 fresh directory-file checks, six manifest links, five external references and 36 unchanged browser source files. Prior 41/41 native/WASM tests remain historical. No application source, binary, asset, progression data or Claude-owned file was changed this turn. Continuing more hidden UI prototypes would not resolve the native visual acceptance gate.

After a newly authorized supported launch succeeds, the next action remains the existing default-cockpit-OFF menu/track selector test, followed by observed defect correction and separately scoped later UI/world validation.
