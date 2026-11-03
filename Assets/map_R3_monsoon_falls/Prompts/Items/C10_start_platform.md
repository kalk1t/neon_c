# C10 — Start platform

## Individual item

PNG: ../../Assets/Course/C10_start_platform.png

Type: **modeling_reference**. Reused unchanged from the approved Neon Circuit B course asset collection.

## Build specification

- **Method:** Blender: broad deck with spawn markings, thin light strips, and separate underbody.
- **Nominal dimensions:** 24 / 48 / 3 m (local Blender X / Y / Z)
- **Origin:** Center of upper deck; travel local -Y.
- **Collision:** Deck slab only, plus explicit edge barriers. Spawn volumes stay clear.
- **LOD0 / LOD1 / LOD2 triangle targets:** 5,000 / 1,800 / 480. These are future production targets, not measurements of delivered meshes.

24 m width by 48 m length as a target for a starting grid. Adapt to the real number of racers and spawn spacing. Keep start geometry and start/finish trigger independent.

All dimensions are authored design targets. The image is a visual reference and is not calibrated CAD, a mesh, a collision shape, or a PBR texture set. Use the route and clearance requirements to determine final geometry. Keep narrow cyan/magenta lights separate from neutral structural materials.

## Meshy use

Build this repeatable structural object in Blender. Meshy is optional only for decorative detail that does not determine connection geometry, clear openings, collision, or sign text. If a seed is useful, use this one isolated PNG only; clean it, set scale/pivot, create LODs and separate collision in Blender.

No paid Meshy generation was executed for this pack.

## Exact image-generation prompt

The following is the exact prompt for the included image. For reused B assets, references named in the original prompt describe the original generation context; they are not additional required runtime assets.

~~~text
Use case: stylized-concept. Create ONE standalone individual environment asset PNG for B NEON CIRCUIT flying-car combat racing. Image1 is the authoritative selected B-only reference. Image2 provides the same B column at higher detail; ignore its A and C columns. The subject must be a COURSE/ENVIRONMENT OBJECT, no car, no vehicle, no whole scene. Match the central aerial-course image: dark obsidian/navy alloy plates, chrome structural trim, bright thin cyan edge lights, restrained magenta accents, polished yet readable stylized 3D game art. Single isolated object, elevated three-quarter isometric perspective showing top and front edge/underside, complete geometry in frame with generous 12 percent margin, genuine transparent RGBA background. No ground plane, skyline, cloud, shadows on background, captions, watermark or contact sheet. Studio lighting should expose the black metal details without drowning them in neon bloom. Subject: the LARGE RECTANGULAR START/LAUNCH PLATFORM beneath the START gate in the reference, shown BY ITSELF WITH NO GATE so that the existing gate can be attached later. Broad flat deck with segmented dark plates and two clear cyan longitudinal guidance strips, subtle starting-grid stripe near one end. Chamfered corners, cyan perimeter lights, magenta small underdeck lights, thick layered structural underside, visible mounting sockets for a separate start arch at the two sides. Small low edge guards along sides only, entry and exit ends unobstructed. A substantial floating racing platform with integrated hover mechanisms underneath; no tall support pillar, no building, no vehicle.
~~~
