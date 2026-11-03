# C13 — Soft water mist sprite

## Individual item

PNG: ../../Assets/Effects/C13_soft_mist.png

Type: **vfx_sprite_source**. Generated for these map packs with the built-in image generator; transparent background enabled.

## Build specification

- **Method:** Runtime VFX: sparse translucent cards with distance fading and depth testing.
- **Nominal dimensions:** See construction note; no fixed bounding-box scale.
- **Origin:** Texture center; camera-facing particle or oriented mist card.
- **Collision:** None.
- **LOD0 / LOD1 / LOD2 triangle targets:** 2 / 2 / 0. These are future production targets, not measurements of delivered meshes.

Neutral RGBA mist source. Suggested derivative 256-512 px. Tint gently from scene lighting; do not place opaque fog walls across checkpoint sight lines. Alpha sorting and overdraw require engine QA.

All dimensions are authored design targets. The image is a visual reference and is not calibrated CAD, a mesh, a collision shape, or a PBR texture set. Use the route and clearance requirements to determine final geometry. Keep narrow cyan/magenta lights separate from neutral structural materials.

## Meshy use

Do not send this particle/look reference to Meshy. Follow the runtime construction method above.

No paid Meshy generation was executed for this pack.

## Exact image-generation prompt

The following is the exact prompt for the included image. For reused B assets, references named in the original prompt describe the original generation context; they are not additional required runtime assets.

~~~text
Use case: stylized-concept. Asset type: ONE isolated soft water-mist particle sprite for rainy futuristic racing maps. One irregular airy small puff of neutral white and very pale gray water spray, broad low oval with finely wispy broken edges, translucent center and gradually fading outer tendrils; natural water mist, not fire smoke. Front-on 2D VFX sprite centered in a square canvas occupying 70 percent width, with abundant completely transparent margin. White and gray only, no magenta, blue or other tint. Genuine transparent background, soft per-pixel alpha, no backdrop, no horizon, no ground, no shadow, no waterfall, no rain field, no glowing halo, no checkerboard painted in. No other objects, no text, no labels, no watermark. A single mist puff only, not a sprite sheet.
~~~
