# C01 — Straight aerial deck

## Individual item

PNG: ../../Assets/Course/C01_straight_deck.png

Type: **modeling_reference**. Reused unchanged from the approved Neon Circuit B course asset collection.

## Build specification

- **Method:** Blender: build a dimensioned repeatable mesh or sweep a cross-section along the route.
- **Nominal dimensions:** 18 / 24 / 2 m (local Blender X / Y / Z)
- **Origin:** Center of the upper driving deck, Z=0; travel along local -Y.
- **Collision:** Simple deck slab; keep bolts and decorative underbody non-colliding.
- **LOD0 / LOD1 / LOD2 triangle targets:** 2,400 / 900 / 240. These are future production targets, not measurements of delivered meshes.

18 m usable width, 24 m nominal length, 2 m structure below the deck. The source image gives panel and lighting style; its apparent proportions do not define meter scale. Join positions and tangent continuity come from Layout/route.json.

All dimensions are authored design targets. The image is a visual reference and is not calibrated CAD, a mesh, a collision shape, or a PBR texture set. Use the route and clearance requirements to determine final geometry. Keep narrow cyan/magenta lights separate from neutral structural materials.

## Meshy use

Build this repeatable structural object in Blender. Meshy is optional only for decorative detail that does not determine connection geometry, clear openings, collision, or sign text. If a seed is useful, use this one isolated PNG only; clean it, set scale/pivot, create LODs and separate collision in Blender.

No paid Meshy generation was executed for this pack.

## Exact image-generation prompt

The following is the exact prompt for the included image. For reused B assets, references named in the original prompt describe the original generation context; they are not additional required runtime assets.

~~~text
Use case: stylized-concept. Create ONE standalone individual environment asset PNG for B NEON CIRCUIT flying-car combat racing. Image1 is the authoritative selected B-only reference. Image2 provides the same B column at higher detail; ignore its A and C columns. The subject must be a COURSE/ENVIRONMENT OBJECT, no car, no vehicle, no whole scene. Match the central aerial-course image: dark obsidian/navy alloy plates, chrome structural trim, bright thin cyan edge lights, restrained magenta accents, polished yet readable stylized 3D game art. Single isolated object, elevated three-quarter isometric perspective showing top and front edge/underside, complete geometry in frame with generous 12 percent margin, genuine transparent RGBA background. No ground plane, skyline, cloud, shadows on background, captions, watermark or contact sheet. Studio lighting should expose the black metal details without drowning them in neon bloom. Subject: ONE REUSABLE STRAIGHT ELEVATED DECK/BRIDGE SEGMENT from the aerial circuit, separate from the city and gates. Long narrow rectangular dark segmented deck, about four times as long as wide, flush modular connection plates at both ends, cyan edge lighting on both long sides, short angular black/chrome low guard sections with a few magenta light inserts, layered support girders and compact antigravity units underneath. Both open ends clearly visible. It is a platform/course-construction module over which cars can fly; no ground, gate, towers, vehicle or separate accessories.
~~~
