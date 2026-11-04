# Supplemental fallback glyphs: ≈ and ♛

This additive package contains only the new missing characters; the earlier middle-dot/star package is unchanged. All project folders are read-only. The current browser has results logic in src/race.js: estimated standings times are prefixed with U+2248 and the first coin-race name with U+265B. The menu ranking button also uses U+265B.

## Observed source rendering

A Windows Chromium text-only fixture uses the current CSS and the preserved static Orbitron500/700/900 faces, without any game scripts. After paint, CSS.getPlatformFontsForNode identifies U+2248 as Arial Black / Arial-Black, and U+265B as Segoe UI Symbol / SegoeUISymbol, at all tested sizes/device scales. The bundled Orbitron cmap lacks both. Installed fallback font paths/hashes/glyph IDs are recorded in provenance/font-cmap-audit.json; whole system fonts are not copied.

The actual result styles are weight700 at15px. Approximate standings times normally use the span color, with a different player-row color/shadow. The coin prefix inherits its list-row style. Ranking buttons use20px normally,18px under the max-height820px compact CSS rule, and3px letter spacing. Source paint values, row variants, line heights and selectors are recorded rather than baked into a new full label. Advertising uses a larger queen already baked in the existing slide asset; this supplement does not duplicate that advertisement.

## Minimal portable assets

Twelve white-core RGBA PNGs cover approximate15px plus queen15/18/20px at device scales1/2/3. They contain only the individual glyphs, with two logical pixels of transparent padding. No full-screen fixtures or replacement skins are provided. Full capture cells remain as evidence; native-glyphs/ contains the intended small draw assets.

Use glyph-metrics.json or the metadata-only C header native/rr_results_fallback_glyphs.h. Lookup by codepoint, font size and device scale. Draw at logical (penX+offset_x, baselineY+offset_y), using draw_width/draw_height. Advance by advance, then apply the native layout's tracking once. advance excludes tracking; the source menu's3px is separately stored as source_tracking. Results tracking is0. Do not use bitmap width as advance: preserve bearings and transparent padding. Keep UTF-8/codepoint iteration and primary Orbitron text unchanged, dispatching only codepoints8776/9819 to these fallback assets.

Tint the white glyph to the current source context color, and handle any row shadow separately. The assets contain no baked shadow, tracking or dynamic text; do not multiply opacity twice or apply world bloom to UI glyphs. Menu hover/focus changes paint, not the glyph shape. Touch zoom, arbitrary font sizes, non-integer subpixel phase and native sampling/hinting need separate review; these captures certify only their recorded source sizes/scales on the inspected Windows font installation.

## Runtime alternative and limits

Native code may instead resolve installed Arial Black for U+2248 and Segoe UI Symbol for U+265B, verify cmap and load those codepoints explicitly into fallback atlases. ASCII95 loading omits them even in a face that has them. Browser font700 synthesis/hinting and glyph placement can differ from raw native loading of the installed faces; the PNGs are the source-raster reference option, not proof of native rendering parity. Do not replace ≈ with ~ or ♛ with another crown/asterisk.

Only CPU font/PNG inspection was performed. No game, native graphics test, WebGL test or deployment was launched. Source and old-package hashes are verified. The C header is data/lookup scaffolding, not a compiled production change. Browser-specific fallback and system font versions can differ on other machines. Rendering output and provenance do not grant additional system-font redistribution rights; applicable font terms remain to be reviewed for external distribution. No paid service or font download was used.
