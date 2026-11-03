# C02 — Curved aerial deck

## Individual item

PNG: ../../Assets/Course/C02_curved_deck.png

Type: **modeling_reference**. Reused unchanged from the approved Neon Circuit B course asset collection.

## Build specification

- **Method:** Blender: generate from a curve with a shared cross-section.
- **Nominal dimensions:** See construction note; no fixed bounding-box scale.
- **Origin:** Centerline at the entrance on the deck surface; local -Y is the entrance tangent.
- **Collision:** Simplified curved deck strip; separate outer curb collision.
- **LOD0 / LOD1 / LOD2 triangle targets:** 3,600 / 1,200 / 320. These are future production targets, not measurements of delivered meshes.

18 m usable width; initial center radius 64 m and 30 degree turn as an authoring target. Match the actual route curvature instead of forcing this radius everywhere. Share seam vertices or snap matching boundary loops.

All dimensions are authored design targets. The image is a visual reference and is not calibrated CAD, a mesh, a collision shape, or a PBR texture set. Use the route and clearance requirements to determine final geometry. Keep narrow cyan/magenta lights separate from neutral structural materials.

## Meshy use

Build this repeatable structural object in Blender. Meshy is optional only for decorative detail that does not determine connection geometry, clear openings, collision, or sign text. If a seed is useful, use this one isolated PNG only; clean it, set scale/pivot, create LODs and separate collision in Blender.

No paid Meshy generation was executed for this pack.

## Exact image-generation prompt

The following is the exact prompt for the included image. For reused B assets, references named in the original prompt describe the original generation context; they are not additional required runtime assets.

~~~text
Use case: stylized-concept. Create ONE standalone environment asset PNG for B NEON CIRCUIT flying-car combat racing. Image1 is the authoritative B-only reference; image2 shows the same B art at higher detail, use ONLY its CENTER B column. Match obsidian/navy alloy, brushed chrome structural edges, cyan architecture lights and controlled magenta accents, high quality polished stylized 3D. The subject is environment only, NO CAR. Single isolated object, entire object completely inside the frame with generous margin, studio lighting shows black surfaces clearly, genuine transparent RGBA background. No ground plane, skyline, clouds, background shadows, captions, watermarks or contact sheet. Subject: ONE NINETY-DEGREE CURVED ELEVATED DECK SEGMENT visible on the right side of the reference course. Elevated three-quarter isometric view. Quarter-circle modular bridge with a constant-width segmented dark-metal deck, two visibly open square-cut connection ends at right angles, clear curved inner and outer edges, thin bright cyan perimeter strips, a few low chrome-and-black magenta-inset edge guards, substantial layered underdeck girders and compact floating supports. Match the construction style of the course's start platform and straight deck. Keep the central inside curve open. No gates, tall supports, buildings, car or accessories.
~~~
