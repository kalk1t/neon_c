# C12 — Rain splash / ripple sprite

## Individual item

PNG: ../../Assets/Effects/C12_rain_splash.png

Type: **vfx_sprite_source**. Generated for these map packs with the built-in image generator; transparent background enabled.

## Build specification

- **Method:** Runtime VFX: pooled small quads or decals with short scale-and-fade animation.
- **Nominal dimensions:** See construction note; no fixed bounding-box scale.
- **Origin:** Texture center; place just above a receiving surface.
- **Collision:** None.
- **LOD0 / LOD1 / LOD2 triangle targets:** 2 / 2 / 0. These are future production targets, not measurements of delivered meshes.

Single top-down splash sprite, not an animation sheet. Suggested runtime derivative 128-256 px; keep alpha and fade it through the engine. Avoid obvious repeated splashes on every visible surface.

All dimensions are authored design targets. The image is a visual reference and is not calibrated CAD, a mesh, a collision shape, or a PBR texture set. Use the route and clearance requirements to determine final geometry. Keep narrow cyan/magenta lights separate from neutral structural materials.

## Meshy use

Do not send this particle/look reference to Meshy. Follow the runtime construction method above.

No paid Meshy generation was executed for this pack.

## Exact image-generation prompt

The following is the exact prompt for the included image. For reused B assets, references named in the original prompt describe the original generation context; they are not additional required runtime assets.

~~~text
Use case: stylized-concept. Asset type: isolated rain-impact splash and ripple sprite for a browser racing game. Generate ONE small round rain impact splash seen in strict top-down view, a very low broken circular crown of tiny water droplets around two soft concentric fading ripples. Neutral white and light gray highlights with translucent water, no color tint. Centered circular composition filling about 65 percent of a square canvas. A single cohesive water effect only, crisp small highlight details and smoothly feathered outer alpha. Genuine transparent background including transparent center gaps; no water plane, no reflective floor, no dark disk, no environment, no shadows, no checkerboard painted in, no text or watermark. 2D VFX sprite suitable as a starting reference for a small animated particle; no grid and no animation sheet.
~~~
