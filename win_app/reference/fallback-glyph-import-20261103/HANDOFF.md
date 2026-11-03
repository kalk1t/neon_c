# Browser fallback glyph handoff: U+00B7 and U+2605

Source request: win_app/reference/ui-parity-20261103/ui-fallback-glyph-requirements.json. The current native hashes still match that file. All writes are in this separate task package; no source, native atlas or project folder was changed.

## Actual source-matched browser result

The current #submsg CSS is font70026px Orbitron, 'Arial Black', sans-serif, zero letter spacing, cyan color and a10px cyan text shadow. A GPU-disabled Windows Chromium fixture loads the exact preserved Orbitron weights, the current source CSS, and no game script. DevTools CSS.getPlatformFontsForNode after paint identifies the isolated U+00B7 as Arial Black (Arial-Black), and U+2605 as Segoe UI Symbol (SegoeUISymbol), at all three device scales. Mixed-label font runs are recorded too. This is a source-matched fixture oracle, not a live game/native screenshot comparison.

Font cmap/advance checks independently confirm the two glyphs are absent from Orbitron500/700/900. Arial Black contains the dot but no star; Segoe UI Symbol contains the star. Rajdhani's dot exists but has a different advance (5.382px at26px versus browser dot8.671875CSSpx), and it is not selected by the source submessage stack. Do not switch to Rajdhani or substitute ASCII period/asterisk.

## Portable asset option

native-glyphs/ contains12 isolated RGBA PNGs: two glyphs × device scales1/2/3 × white-core and source-cyan-with-glow variants. These are individual characters, not a baked full-screen label/skin. Each crop preserves source pixels losslessly and has two CSS pixels of transparent padding. Full120CSSpx capture cells are retained in glyphs/ as evidence; they are not intended draw bounds.

glyph-metrics.json records advance, crop, baseline and logical draw size. The metadata-only C header native/rr_fallback_glyphs.h supplies a lookup table with the same values; it is a handoff helper, not a compiled/accepted native integration.

For an intercepted Unicode codepoint, choose a glyph at the current device scale. If penX and baselineY are logical UI coordinates, draw its PNG at (penX+offset_x, baselineY+offset_y), with draw_width/draw_height, then advance penX by advance. If using physical pixels directly, multiply all logical values by device scale consistently. Do not center the crop rectangle on the text baseline: bearings/overhangs matter. Source advances: dot8.671875CSSpx, star21.671875CSSpx. The star's ink overhangs its advance; preserve the given bearing instead of substituting bitmap width as advance.

White-core PNGs support explicit tint/glow binding. Source-glow PNGs already include cyan core and CSS shadow: draw once with neutral tint, and do not add another core/glow. Source color/shadow strings are in glyph-oracle.json. Render primary letters with the existing primary font. Dispatch only codepoints183 and9733 to this small fallback table; continue UTF-8/codepoint iteration normally.

## Runtime font option

On Windows, resolve the installed local Arial Black face for U+00B7 and Segoe UI Symbol face for U+2605, verify cmap coverage, and explicitly load those Unicode codepoints into fallback atlases. An ASCII95 atlas excludes both characters even when the font contains them. Do not call a missing-primary glyph lookup and accept its default/notdef marker. This package records installed paths/hashes but does not copy or redistribute full system font files.

Raw native font-file rasterization is not automatically equal to browser font700 behavior: installed Arial Black reports weight900, Segoe UI Symbol400, and browser font selection/synthesis, hinting, alpha coverage and rounding can differ. The supplied browser PNGs are the fixed26CSSpx reference option; the runtime-font alternative needs its own native visual review. For other sizes/DPI, do not assume these twelve crops certify arbitrary resampling. Keep fractional pen advances; subpixel-phase AA and texture sampling also need review.

## Evidence, rights and acceptance

validation.json checks source/native hashes, cmap coverage, all platform-font resolutions, DPI-stable advances, non-clipped ink and exact PNG crop roundtrips. Original font/source paths and hashes are in provenance/. Browser fixtures and system font versions can differ across OS/browser installations. No complete native label pixel parity, device acceptance or hardware graphics behavior is claimed. No game, native process or WebGL renderer was launched.

Captured glyph PNGs are rendering output derived from installed system font faces. Whole proprietary fonts are not included. This handoff does not grant additional font rights or independently establish redistribution permission; review applicable font terms before external redistribution. Existing Orbitron/Rajdhani font notices remain in the prior asset packages. No paid generation or replacement font download was used.
