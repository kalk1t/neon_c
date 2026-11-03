# C08 — Turn chevron sign

## Individual item

PNG: ../../Assets/Course/C08_turn_chevron.png

Type: **modeling_reference**. Reused unchanged from the approved Neon Circuit B course asset collection.

## Build specification

- **Method:** Blender: shallow sign housing and emissive chevron geometry.
- **Nominal dimensions:** 6 / 0.5 / 3 m (local Blender X / Y / Z)
- **Origin:** Center of the sign face; face normal local -Y.
- **Collision:** None unless the housing is reachable; then one simple box outside the corridor.
- **LOD0 / LOD1 / LOD2 triangle targets:** 900 / 320 / 64. These are future production targets, not measurements of delivered meshes.

Place on the outside of bends facing the approaching driver. Use the correct arrow direction for each bend; do not mirror the entire track.

All dimensions are authored design targets. The image is a visual reference and is not calibrated CAD, a mesh, a collision shape, or a PBR texture set. Use the route and clearance requirements to determine final geometry. Keep narrow cyan/magenta lights separate from neutral structural materials.

## Meshy use

Build this repeatable structural object in Blender. Meshy is optional only for decorative detail that does not determine connection geometry, clear openings, collision, or sign text. If a seed is useful, use this one isolated PNG only; clean it, set scale/pivot, create LODs and separate collision in Blender.

No paid Meshy generation was executed for this pack.

## Exact image-generation prompt

The following is the exact prompt for the included image. For reused B assets, references named in the original prompt describe the original generation context; they are not additional required runtime assets.

~~~text
Use case: stylized-concept. Create one individually usable PNG concept reference for the user's flying-car combat racing game.
Reference image 1 is a STYLE REFERENCE only. Match ONLY the CENTER COLUMN labeled B — NEON CIRCUIT. Ignore the orange/green A column and the desert C column completely. Preserve its polished stylized 3D hard-surface rendering, black anodized metal and chrome, cyan architectural edge lights, magenta luminous accents, clean modular geometry, elegant proportions and readable silhouette. This is a new isolated single-asset render, not a cropped board, not a comparison sheet, not a collage. No brands or watermarks.
Asset: ONE standalone TURN DIRECTION ARROW SIGN for the neon aerial race course. A compact beveled rectangular black/chrome panel with three large luminous MAGENTA CHEVRONS pointing RIGHT, thin cyan frame lighting, restrained industrial edge bolts, and one small underside mounting socket. The arrow chevrons are the entire message, no letters or numbers. Front-facing three-quarter product view keeps the three chevrons highly readable. No pole, environment, road, or other props. Transparent PNG with genuine alpha outside the object, including any openings. One complete object, centered in a square frame, generous clear margin on every side, three-quarter product view, entire silhouette visible, no crop. Neutral soft studio highlights make black metal legible. Controlled emissive bloom with clean edges. No environment, floor, backdrop, checkerboard pattern, labels, captions, dimension lines, extra objects, or ground shadow.
~~~
