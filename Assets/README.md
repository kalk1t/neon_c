# Neon Circuit — Style B individual PNG assets

This pack contains **27 individually generated PNG assets** for the selected Neon Circuit art direction: black and chrome flying cars, cyan navigation lighting, magenta attack energy, and a futuristic night city.

## Contents

| Folder | PNGs | Contents |
| --- | ---: | --- |
| 01_Vehicle | 3 | Flying car, removable thruster, pulse weapon |
| 02_Course | 7 | Aerial course reference, start/finish gate, checkpoint ring, beacon, barrier, arrow sign, floating platform |
| 03_Scenery | 5 | City tower, control tower, two neon trees, skyline |
| 04_Interface | 4 | Racing HUD, menu, minimap, attack icon |
| 05_Materials | 4 | Black metal, chrome, magenta emissive, blue glass appearance references |
| 06_Effects | 4 | Thruster trail, projectile, impact, smoke |

Each asset has a separate matching generation prompt in **Prompts**. **ASSET_INDEX.md** gives filenames, dimensions, transparency, and intended uses. **assets.json** provides the same inventory with file checksums for programmatic use. **Reference** contains the original three-style comparison sheet; use only its middle B column for this project.

## How to use the artwork

- Use the vehicle, course pieces, and scenery images as visual references when making the 3D models in Blender or an image-to-3D workflow.
- Keep the car body, four flight pods, and forward pulse weapon as separately controllable parts when modeling. The image of the complete car is a single view, so hidden geometry still needs to be designed and reviewed.
- The course image establishes the visual route and altitude changes. Build the actual course geometry, checkpoints, reset positions, and collision separately.
- Use the HUD and menu as interface design references. Speed, time, lap count, cooldowns, and the minimap must be driven by game code; numbers shown in these PNGs are illustrative.
- The material spheres show the desired appearance. Create actual shader parameters and texture maps for the 3D models; these sphere images are not seamless or calibrated PBR texture maps.
- The effect images are individual static sprite artwork. Size, animate, orient, blend, and test them in the browser renderer. Thruster trail is cyan; the attack projectile and impact use magenta.
- The skyline is a panoramic visual reference, not a tested seamless or equirectangular skybox.

## Image format and transparency

The pack preserves the original generated PNGs without cropping or resampling. **23 of 27 asset images contain real transparent pixels.** The aerial course, skyline, HUD, and menu are intentionally opaque scene/interface references. Exact dimensions and alpha information are in the index. Neon glows use soft transparency and should be checked against the intended game background.

## Art direction to preserve

- A recognizable sports-car cabin, hood, windshield, doors, and rear body.
- Four mechanical flight pods, with propulsion visually separate from the forward attack aperture.
- Obsidian body panels and chrome trim; cyan running lights and navigation; magenta weapon energy and selected neon accents.
- A true aerial race route through gates at different heights, with clear passages and readable course markers.
- Matching modular forms and materials across gates, buildings, barriers, and vehicle components.

## Further assets and implementation

This is the requested PNG reference/artwork package. The next production step is to make the 3D models, materials, and playable systems from these references.

Audio still needs actual sound files: a thruster/engine loop, weapon discharge, impact, countdown, race-start, and finish cues. Gameplay needs simplified collision geometry, ordered checkpoint volumes, safe reset transforms, flight controls, attack rules, and dynamic UI. Those are functional game components rather than PNG artwork.

## Prompt reuse

The prompt files preserve the text used for each generation. References in those prompts mean the center B column of **Reference/Original_Three_Style_Comparison.png**. Vehicle-module prompts also use **01_Vehicle/01_flying_car.png** as a visual reference. If a prompt contains a generation-time workspace location, substitute the matching file in this extracted pack.

## Validation

All 27 asset PNGs were decoded successfully, every asset has its own prompt, image files are distinct, and the ZIP was checked for archive integrity. Visual reviews checked the selected palette, object readability, flight-course direction, UI text, and separation of effects. This validates the delivered image pack; browser rendering, 3D geometry, animation, and gameplay have not been built or tested in this task.
