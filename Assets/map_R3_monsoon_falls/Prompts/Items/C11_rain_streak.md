# C11 — Rain streak sprite

## Individual item

PNG: ../../Assets/Effects/C11_rain_streak.png

Type: **vfx_sprite_source**. Generated for these map packs with the built-in image generator; transparent background enabled.

## Build specification

- **Method:** Runtime VFX: textured instanced quads or the engine's existing particle system.
- **Nominal dimensions:** See construction note; no fixed bounding-box scale.
- **Origin:** Texture center; animate in a camera-centered particle volume.
- **Collision:** None.
- **LOD0 / LOD1 / LOD2 triangle targets:** 2 / 2 / 0. These are future production targets, not measurements of delivered meshes.

One neutral RGBA source sprite. Make a 64-128 px runtime derivative in the production pipeline, preserve alpha, and choose particle width/length in meters at runtime. It is neither a 3D model nor a seamless texture.

All dimensions are authored design targets. The image is a visual reference and is not calibrated CAD, a mesh, a collision shape, or a PBR texture set. Use the route and clearance requirements to determine final geometry. Keep narrow cyan/magenta lights separate from neutral structural materials.

## Meshy use

Do not send this particle/look reference to Meshy. Follow the runtime construction method above.

No paid Meshy generation was executed for this pack.

## Exact image-generation prompt

The following is the exact prompt for the included image. For reused B assets, references named in the original prompt describe the original generation context; they are not additional required runtime assets.

~~~text
Use case: stylized-concept. Asset type: isolated rain-streak particle sprite for a browser racing game. Generate ONE vertical thin white rain streak only, a soft tapered elongated droplet motion blur with a narrow bright neutral-white core and transparent feathered edges, fading smoothly to full transparency at both tips. Shape roughly 4 percent of image width and 75 percent of image height, perfectly centered in a square canvas. Neutral white and gray only, no blue, no magenta. Straight-on 2D VFX sprite, no perspective, no lighting environment. Actual transparent background, no checkerboard painted in, no solid background, no floor or shadow, no bloom field. No other droplets, no splashes, no rain scene, no clouds, no text, no border, no watermark. Keep the useful streak isolated with abundant empty transparent pixels on all sides.
~~~
