# Latest verified client launcher

Run-RoadstersRace.cmd is inside win_app. It previously opened dist/first-flight/RoadstersRace.exe. The existing scripts/Build-Windows.ps1 builds build/windows-Configuration and, with -Package, installs over dist/first-flight. Neither packaging nor that old binary was changed for this update.

The launcher now explicitly selects build/spin-readout-native/bin/RoadstersRace.exe and sets its working directory to that bin folder. CMake's native post-build step already copied assets there. This isolated client contains the latest completed UI/input/camera changes and new race submessages, with cockpit preview still OFF and existing graphics safeguards unchanged. Old builds and dist/first-flight remain intact. No save/profile data was copied, removed or reset.

The target passed a full application compile and 51/51 focused native tests; corresponding WASM checks also passed 51/51. All 449 native asset copies match source hashes. Camera visibility tests explicitly record a pre-existing native/JavaScript numerical disagreement at the exact free-look threshold; see docs/VIEW-CONTROLS-PARITY-2026-11-03.md for the qualified test scope. See reference/ui-parity-20261103/spin-readout-implementation.json for exact executable and asset identities.

Verification of this launcher is static only: resolve its explicit relative path, check executable identity and copied assets, confirm quoted command and working directory. The launcher was not executed. It remains an intentional user-operated game launch; no native visual/stability acceptance is implied by pointing it at a headlessly verified build.

For each future verified progress update, promote the matching isolated native output by updating the explicit clientDir after successful build and applicable checks, and record its executable/asset hashes and test scope before reporting. Never select the newest file by timestamp, silently fall back to an old build, replace an in-use executable, or run the client as part of promotion. If the target is missing this launcher reports the expected path instead of opening an older client.

Original launcher preserved at reference/pre-launcher-latest-20261103/Run-RoadstersRace.cmd. Build-Windows.ps1 -Package still updates the old dist target and does not promote this launcher; keeping those operations distinct avoids silently overwriting an older package.

Native graphical validation remains blocked by the prior automatic-review rejection under AGENTS.md pending explicit execution authorization. This launcher update neither retries that action nor bypasses its gate.

