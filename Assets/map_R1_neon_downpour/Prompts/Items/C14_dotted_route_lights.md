# C14 — Dotted route light sequence

## Individual item

PNG: ../../Assets/Course/C14_dotted_route_lights.png

Type: **placement_look_reference**. Reused unchanged from the approved Neon Circuit B course asset collection.

## Build specification

- **Method:** Blender/runtime: repeated small emissive discs or sprites along a curve.
- **Nominal dimensions:** See construction note; no fixed bounding-box scale.
- **Origin:** Each light instance uses its own center; positions are sampled from the route.
- **Collision:** None.
- **LOD0 / LOD1 / LOD2 triangle targets:** 16 / 8 / 2. These are future production targets, not measurements of delivered meshes.

This PNG is a perspective look reference for an arrangement, not a calibrated spline or a texture to stretch over the race. Build individual dots at a proposed 6-10 m spacing along aerial gaps. Triangle targets apply per dot.

All dimensions are authored design targets. The image is a visual reference and is not calibrated CAD, a mesh, a collision shape, or a PBR texture set. Use the route and clearance requirements to determine final geometry. Keep narrow cyan/magenta lights separate from neutral structural materials.

## Meshy use

Do not send this particle/look reference to Meshy. Follow the runtime construction method above.

No paid Meshy generation was executed for this pack.

## Exact image-generation prompt

The following is the exact prompt for the included image. For reused B assets, references named in the original prompt describe the original generation context; they are not additional required runtime assets.

~~~text
Use case: stylized-concept. Generate ONE isolated individual PNG asset for NEON CIRCUIT. Image 1 is the authoritative selected style B reference. Image 2 shows the same art direction in its CENTER B COLUMN only; ignore A and C. Match the sleek black metal/chrome, crisp cyan and magenta neon accents, polished stylized 3D-game aesthetic. One complete asset, centered with generous margins, no collage, no text, no label, no watermark, no scenery. Genuine transparent RGBA background, including soft glow edges, no checkerboard painted into image, no solid background. Subject: ONE curved string of small glowing airborne route-marker dots exactly like the dotted flight path joining the rings in the aerial-course reference. A smooth shallow S-shaped perspective curve composed of separate round luminous MAGENTA points, with occasional small CYAN guide points. The route sweeps from lower left to upper right and is fully contained in a wide composition with ample edge room. Soft halos around individual dots, clearly separated points, translucent glow fading into genuine transparent alpha. No physical connecting tube, no road, no aircraft, no rings, no arrows, no environment. This is a single route-marker sprite/reference segment.
~~~
