# C03 — Climbing transition ramp

## Individual item

PNG: ../../Assets/Course/C03_climbing_ramp.png

Type: **modeling_reference**. Generated for these map packs with the built-in image generator; transparent background enabled.

## Build specification

- **Method:** Blender: control both end heights and tangents; generate an eased climb profile.
- **Nominal dimensions:** 18 / 24 / 8 m (local Blender X / Y / Z)
- **Origin:** Center of lower entrance on the upper deck surface, Z=0; travel local -Y.
- **Collision:** Low-poly sloped deck with eased transitions, never a full bounding box across the air space.
- **LOD0 / LOD1 / LOD2 triangle targets:** 3,000 / 1,000 / 280. These are future production targets, not measurements of delivered meshes.

Nominal 24 m horizontal length, 18 m width, 6 m rise and about 2 m structural depth. Use longer ramps where car control needs a gentler climb. The PNG is a style reference; rebuild exact slopes from the validated centerline.

All dimensions are authored design targets. The image is a visual reference and is not calibrated CAD, a mesh, a collision shape, or a PBR texture set. Use the route and clearance requirements to determine final geometry. Keep narrow cyan/magenta lights separate from neutral structural materials.

## Meshy use

Build this repeatable structural object in Blender. Meshy is optional only for decorative detail that does not determine connection geometry, clear openings, collision, or sign text. If a seed is useful, use this one isolated PNG only; clean it, set scale/pivot, create LODs and separate collision in Blender.

No paid Meshy generation was executed for this pack.

## Exact image-generation prompt

The following is the exact prompt for the included image. For reused B assets, references named in the original prompt describe the original generation context; they are not additional required runtime assets.

~~~text
Use case: stylized-concept. Asset type: ONE isolated climbing-ramp module for the NEON CIRCUIT flying-car racing course. Image 1 supplies ONLY the black/chrome material and thin cyan-magenta light-strip styling; change its straight geometry completely. The requested geometry is an UNMISTAKABLE SLOPING RAMP WITH TWO BENDS: short flat low entrance at the LEFT, a steep diagonal rising middle, then a short flat high exit at the RIGHT. Viewed nearly from the SIDE at a low 15-degree elevated three-quarter angle, so the SIDE SILHOUETTE clearly shows both horizontal end levels separated by a large vertical rise. The structure is NOT a straight flat plank and NOT just tilted by the camera. The center slope physically connects two horizontal parallel end decks. Left entry length 5 m, center climb length 14 m, right upper exit length 5 m, rise 6 m, full deck width 18 m. Maintain smooth eased knees at the two transitions; two distinct raised corners visible in profile. Thick engineered deck, underslung segmented beams, narrow cyan track edges following the entire rising profile, graphite alloy panels, small brushed chrome trims, tiny magenta accents, low curb sections. One assembled ramp only, no pedestals or ground. Entire item fits central 75 percent of a wide landscape frame with generous empty margin. Neutral bright studio lighting reading dark panels. Genuine transparent background. No floor, shadows, sky, scenery, glow cloud, vehicles, text, diagrams, dimension annotations, watermark, or extra views.
~~~
