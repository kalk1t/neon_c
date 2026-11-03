# Roadsters process and asset reassessment, 2026-11-03

## Current gate

The latest supported desktop inventory contained no Roadsters app/window. This establishes window absence only, not process absence. The existing Task Manager window 265208 was minimized; restoring that same application through its inventory identity succeeded. Its accessibility response still reports higher Windows integrity than the Computer Use helper and exposes only window/title/raise. Its fresh screenshot was occluded by an unrelated foreground window and was not retained as project evidence.

No process-list typing, process termination, game launch, security change, alternative process lookup or retry of previously denied CIM/Git inspection occurred. No graphics test passed or failed this turn. The process check remains **blocked by an access limitation**, with the Roadsters process state **unverified**. There was no fresh auto-review rejection this turn.

Exact owner step: in Task Manager, open Details, search for `RoadstersRace.exe`, close only that application if present, and confirm that no matching process remains. Do not change administrator/security settings. Fresh confirmation of process absence permits the next single-instance test; a missing window alone does not. This small manual observation is needed because the supported helper cannot inspect the higher-integrity process UI reliably.

## Fresh independent work

Read the Windows asset index handoff and v7 integration/addendum. Verified byte lengths and SHA-256 of every manifest-listed file in v5 (149), v6 (248), v7 (47) and the metadata index (12): **456 checks passed, zero failures**. Verified all six index-to-package manifest hashes and five v7 external original-art references. Compared all 36 indexed browser source files with current read-only source: **36 unchanged, zero drift** from the index snapshot. These are directory checks; archives and image pixel/render appearance were not revalidated.

The metadata index itself covers v1–v6. v7 must be consulted alongside it; its addendum supersedes the coin-prefab and primitive-organization gaps without editing old packages. The indexed selected-local-release reference remains `28a4e302e63d2e797db65f62d042025b97a7de84`; source freshness does not assert that the live deployment is still that release or resolve the known working-source versus selected-release backend differences.

Useful integration cautions from the new supplement:

- Coin glTF uses an external v1 PNG; preserve that link or explicitly relocate it. UV crop and image-Y convention are already baked, so do not crop/flip again. Collection radius is 6.5; visual radius 2.6 is separate.
- The v1 captured halo has equal alpha but unequal decoded RGB versus the source gradient. Do not claim byte/pixel equality; v7 provides the gradient component rather than a replacement bitmap.
- Four neutral geometry carriers do not reproduce additive HDR materials. Explicit native material/shader binding remains required, including emissive-strength support where the importer ignores its glTF extension.
- Nitro flames use time-dependent source GLSL, random seed, pooled effects and dynamic buffers. They cannot be replaced faithfully by a static PNG. Native adaptation and compilation remain unqualified.

No runtime assets or application sources were changed or imported. Existing native/WASM **41/41** results are prior evidence, not fresh tests this turn. No potentially running executable or its assets were replaced. Locks, accounts, progress, current resource caps and Claude-owned sources remain untouched.

## Shortest useful continuation

1. Resolve process absence with the owner step above.
2. Launch one existing default-cockpit-OFF `build/v3-ui-corrections-native/bin/RoadstersRace.exe` candidate under established conservative settings. First check only the menu and road picker: typography, clipping, scaling, locked-road refusal, navigation and clean exit. Do not automatically load the world or activate combat/cockpit.
3. Fix observed UI defects, then qualify HUD/pause/results and the opt-in cockpit separately before extending garage presentation. Use verified package chrome only where its geometry fits; retain native dynamic labels and interaction rather than importing fixture screenshots as live screens.
4. Keep city GPU feasibility, full lighting/material parity, live gameplay integration and shared account services as separate acceptance gates.

Further offline UI prototypes would duplicate already-tested hidden work without resolving appearance. The new package audit/source-freshness check is complete; meaningful next UI implementation now depends on native visual evidence. No new feature was invented merely to avoid that boundary.

Machine-readable evidence: `reference/ui-parity-20261103/process-assets-reassessment.json`.
