# Results long-nickname wrapping measurements

This is a bounded source-layout reference corpus requested by RESULTS-QUALIFICATION-BOUNDARY-2026-11-04.md. It does not implement native wrapping, activate main, run a race, call account/services, deploy, or qualify native graphics. All names, numbers and times are scripted fixture values. Browser/source inputs and the request are hashed in provenance/inputs.json; source CSS/HTML/race/hud snapshots are preserved unchanged. Font binaries are referenced from the existing v4 package rather than duplicated.

## Outputs and coverage

- observations.json:60 actual browser observations,19 fixture profiles, device scales1/2/3; tall content has top and bottom observations at each scale. It records each list/li/float box, text-node Range line and individual-character rectangles, source styles, platform font runs, intrinsic heights and scroll state.
- native-layout-goldens.json: compact numeric/text/card-relative view for native CPU qualification.
- fixtures.json: all synthetic racer identities/names/values and frozen header profile.
- threshold-search.json: exact-fit, last observed fitting and first observed over cases for each column, with unbroken legal3–16character names; no ellipsis or discretionary breaks.
- summary.json, qualification-caveats.json, validation.json: concise geometry, explicit scope limits and15 checks.
- previews/: labeled static source-layout screenshots at1× only for selected profiles. These are not real race/account transactions. The availability of2×/3× is established by real browser devicePixelRatio and screenshot pixel dimensions for every observation, not fabricated review images.

The fixtures cover one/three/eleven racers,3- and16-character names,16W normal/player/crown-leader rows,16I, an existing hyphenated legal name, zero/large coins, finished/estimated values, stable ties, exact width boundaries, a late coin float relocation after all standings fit, narrow640×480 and tall1280×360 viewports. One-racer standings are EMPTY in source (multi=false); coin results still contain the one racer. No artificial one-row standings were inserted.

## Important observations

Nominal680px card tracks exactly match335.734375px and258.25px, with26px list padding. These are not invariant source widths: the unchanged CSS uses auto-minimum1.3fr/1fr grid tracks. The16W crown-leader profile produces308.96875px tracks in both columns while the card remains680px. No width/min-width/white-space override was applied to force the nominal numbers. Native code assuming permanently fixed tracks must reject these profiles or obtain an agreed behavior change; this package does not alter browser behavior.

Source computed line-height is21.3px, while a normal measured li is21.296875px. Floats follow the name text in DOM and can relocate below it WITHOUT increasing that li's own height, affecting later rows or crowding floated values side-by-side. Data and images preserve this behavior rather than making independent tidy rows. Source ranges, float boxes and natural column heights are the evidence for an implementation; do not infer line count/card height merely from the presence of a float relocation.

Both columns have a measured exact-equality case that fits. Near the observed boundary, a reported name+float sum exceeding li width by1/64px still fits, whereas2/64px relocates. Inspect the exact names/values/rectangles; do not generalize that into a universal tolerance without matching browser shaping/rounding. The late-coin fixture keeps every standings float on its first line, but relocates the last coin value.

Tall content really overflows: source card height874.828125px at1280×360, actual scroll range0…547px. The viewport rectangles translate by that scroll; card-relative coordinates remain invariant. No wheel, keyboard focus, hit testing, clipping engine, native cold-load lifetime or frame-time behavior was tested.

## Coordinate and intrinsic-height conventions

All measurements are logical CSS pixels. DOMRect viewport boxes and Range rects already include active scroll translation. Parent-relative/card-relative boxes subtract the named measured border-box origin. scrollContentRelative subtracts the scroll viewport origin and adds actual scroll offsets. Added Range LiRelative/CardRelative/SpanRelative fields are exact subtraction from measured viewport rects. DeviceScaleFactor is actually observed at1/2/3 and validated against8CSSpx screenshot probes; these are browser-emulated DPRs, not physical native-monitor DPI acceptance.

The active card's auto-height, grid height and stretched column boxes are measured unchanged. Natural column height is separately probed with an offscreen absolute auto-height/BFC clone of the same source subtree/width/ancestry, removed before active scroll measurement. All observed grid heights equal the larger probed natural column height. Do not confuse the stretched border height with its own natural content height.

::marker font/style, ordinal and26px padding are recorded. DOM Range/CSS does not expose an actual marker rectangle, so none is invented. Review pixels provide visual evidence; exact native marker raster placement remains a separate qualification item.

## Frozen-profile and implementation boundary

The fixture clones the current results DOM, loads current CSS and the existing static Orbitron/Rajdhani fonts, and copies the source escaping/formatting/ordinal and text-then-float row construction. It reproduces stable source sorting using synthetic finished/projected values. It does NOT import race.js, main.js, analytics, accounts or renderer modules. The account output slot is empty; unlock/next are hidden. No saved/offline/service banner is fabricated. Recorded card heights apply to this explicit frozen profile, not automatically to the existing491.1875px native profile or every account/unlock state.

Native code must implement/reject observed wrapping cases atomically, compose the full chosen profile, retain source binding/timer/callback rules and disabled production boundaries, then obtain the separate resource/input/graphics/visual acceptance described in the Windows request. Prior native/WebAssembly test counts are not rerun or promoted here.

## Reopen fixture

Keep this folder beside RoadstersSharedAssets-v4. Run node tools/serve-fixture.mjs and open its printed local URL. Choose a case with ?case=three-short-mixed (or another fixtures.json id). The source card itself contains no fixture controls; use the browser console fixtureSet(data) to change inputs. No button starts a race or writes data. The offline START_HERE.html lists summaries and selected review images without starting any browser process.
