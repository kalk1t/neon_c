# C09 — Route beacon pylon

## Individual item

PNG: ../../Assets/Course/C09_route_beacon.png

Type: **modeling_reference**. Reused unchanged from the approved Neon Circuit B course asset collection.

## Build specification

- **Method:** Blender: low-sided tapered column, cap, and narrow emissive insets.
- **Nominal dimensions:** 2.5 / 2.5 / 8 m (local Blender X / Y / Z)
- **Origin:** Base center at Z=0.
- **Collision:** One simple column proxy if near the route; otherwise none.
- **LOD0 / LOD1 / LOD2 triangle targets:** 1,400 / 480 / 96. These are future production targets, not measurements of delivered meshes.

Place in pairs beyond deck edges or beside aerial gaps. Use shared materials and instance the mesh; avoid a point light on each beacon.

All dimensions are authored design targets. The image is a visual reference and is not calibrated CAD, a mesh, a collision shape, or a PBR texture set. Use the route and clearance requirements to determine final geometry. Keep narrow cyan/magenta lights separate from neutral structural materials.

## Meshy use

Build this repeatable structural object in Blender. Meshy is optional only for decorative detail that does not determine connection geometry, clear openings, collision, or sign text. If a seed is useful, use this one isolated PNG only; clean it, set scale/pivot, create LODs and separate collision in Blender.

No paid Meshy generation was executed for this pack.

## Exact image-generation prompt

The following is the exact prompt for the included image. For reused B assets, references named in the original prompt describe the original generation context; they are not additional required runtime assets.

~~~text
Use case: stylized-concept. Create one individually usable PNG concept reference for the user's flying-car combat racing game.
Reference image 1 is a STYLE REFERENCE only. Match ONLY the CENTER COLUMN labeled B — NEON CIRCUIT. Ignore the orange/green A column and the desert C column completely. Preserve its polished stylized 3D hard-surface rendering, black anodized metal and chrome, cyan architectural edge lights, magenta luminous accents, clean modular geometry, elegant proportions and readable silhouette. This is a new isolated single-asset render, not a cropped board, not a comparison sheet, not a collage. No brands or watermarks.
Asset: ONE aerial route BEACON PYLON matching the center-column B magenta luminous pylon prop. A slim upright black/chrome tapered column with a brilliant vertical magenta light core behind protected slots, cyan edge indicators, a compact layered square hover base and a small clean magenta cap. The base integrates four small anti-gravity pads, allowing this marker to float along a free-flight course. Crisp toy-scale readable geometry, limited purposeful panel detail, no logos or numbers. Transparent PNG with genuine alpha outside the object, including any openings. One complete object, centered in a square frame, generous clear margin on every side, three-quarter product view, entire silhouette visible, no crop. Neutral soft studio highlights make black metal legible. Controlled emissive bloom with clean edges. No environment, floor, backdrop, checkerboard pattern, labels, captions, dimension lines, extra objects, or ground shadow.
~~~
