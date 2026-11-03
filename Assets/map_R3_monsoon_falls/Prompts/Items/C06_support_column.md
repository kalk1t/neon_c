# C06 — Deck support column

## Individual item

PNG: ../../Assets/Course/C06_support_column.png

Type: **modeling_reference**. Reused unchanged from the approved Neon Circuit B course asset collection.

## Build specification

- **Method:** Blender: reusable shaft with independently adjustable height and a fixed top cap.
- **Nominal dimensions:** 8 / 8 / 40 m (local Blender X / Y / Z)
- **Origin:** Center of the base at local Z=0.
- **Collision:** One shaft box plus cap box, placed below/outside the flight corridor.
- **LOD0 / LOD1 / LOD2 triangle targets:** 2,200 / 800 / 160. These are future production targets, not measurements of delivered meshes.

Default target height 40 m; set individual support heights to the deck underside. Extend shaft geometry rather than stretching bolts and trim. Floating sections may omit supports.

All dimensions are authored design targets. The image is a visual reference and is not calibrated CAD, a mesh, a collision shape, or a PBR texture set. Use the route and clearance requirements to determine final geometry. Keep narrow cyan/magenta lights separate from neutral structural materials.

## Meshy use

Build this repeatable structural object in Blender. Meshy is optional only for decorative detail that does not determine connection geometry, clear openings, collision, or sign text. If a seed is useful, use this one isolated PNG only; clean it, set scale/pivot, create LODs and separate collision in Blender.

No paid Meshy generation was executed for this pack.

## Exact image-generation prompt

The following is the exact prompt for the included image. For reused B assets, references named in the original prompt describe the original generation context; they are not additional required runtime assets.

~~~text
Use case: stylized-concept. Generate ONE isolated individual PNG asset for NEON CIRCUIT. Image 1 is the authoritative selected style B reference. Image 2 shows the same art direction in its CENTER B COLUMN only; ignore A and C. Match the sleek black metal/chrome, crisp cyan and magenta neon accents, polished stylized 3D-game aesthetic. One complete asset, centered with generous margins, no collage, no text, no label, no watermark, no scenery. Genuine transparent RGBA background, including soft glow edges, no checkerboard painted into image, no solid background. Subject: a single narrow TALL STRUCTURAL SUPPORT COLUMN for the elevated platforms in the aerial-course image. A dark graphite rectangular mechanical pillar with brushed chrome edges, a clearly visible square platform attachment collar at the top and narrower structural connector at bottom, cyan vertical inset light strips and small magenta status lights. One continuous support with a few compact braces, believable load-bearing proportions. No occupied building, no windows, no skyscraper, no full deck, no large base. Three-quarter front view with the full tall silhouette fitting in a portrait composition. Studio illumination reveals all black panel shapes, clean engineered module.
~~~
