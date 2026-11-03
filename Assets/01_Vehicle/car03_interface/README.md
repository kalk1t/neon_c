# car03_interface — 3A Armored Sport

Selected design: **3A — Armored Sport**  
Car: **CAR03**  
Palette: **Magenta dominant with small cyan accents**  
Controls: **Heavy flat-bottom steering wheel**

Layered black/graphite armor, brushed chrome fasteners, round instruments, and a substantial sports wheel.

## Open first

Unzip the entire folder and open **START_HERE.html**, or open **preview/index.html** directly in a desktop browser. Keep its subfolders together. The preview is self-contained and requires no server, install, account, or network connection.

Press **I** or click **SWITCH VIEW** to show/hide the cockpit. The controls below the preview adjust speed, boost, steering and attack state. An optional demo timer changes the displayed race time. The preview's skyline and numbers are illustrative; the game must provide its real scene, camera and state.

## Included individual PNGs

| Folder | Files | Use |
| --- | ---: | --- |
| artwork/ | 2 | Transparent cockpit shell and independently movable control artwork. |
| panels/ | 4 | Separate blank speed, boost, attack and lap/timer instrument housings. |
| ui/ | 6 | Boost, attack, camera, warning, aiming reticle and I-keycap graphics. |
| references/ | 4 | Selected single cockpit preview, interior modeling cutaway, original car exterior and original options board. |

**16 PNGs total.** The comparison board is retained only to document the approved selection **3A**; the other two concepts are not alternate runtime designs in this pack.

Each generated component has its own exact prompt in **prompts/**. Refinement prompts remain with the corresponding item. **asset_records.json** contains dimensions, alpha bounds, SHA256 hashes, sources and prompt links. **FILE_INDEX.md** lists every PNG and its intended role.

### Cockpit and instruments

- Main shell: **artwork/cockpit_base.png**, 1672 × 941 pixels, with a genuinely transparent windshield. The outside race scene can render behind it.
- The control assembly is separate so it can move without redrawing the dashboard.
- Reusable instrument surfaces are blank. Live speed, lap, timer, boost and attack state are drawn by the included HTML/CSS/JavaScript module.
- The base already contains its instrument bezels. The default preview places live readouts into those blank surfaces; it does not duplicate the separate panel PNGs over them.
- Standalone **panels/** images are available for alternate screen layouts, detail views, or building display surfaces in a later 3D interior.
- Light rings and trim in the artwork are decorative. The changing boost meter and cooldown readout belong to runtime UI.

### Interface states

The included module supports speed, lap count, elapsed time, boost from 0–100%, attack ready/cooldown/unavailable, a small reticle and the I-key view hint. It exposes a host callback for the actual camera switch. It does not trigger attacks, consume boost, move the car or invent game mechanics.

## Integrate

Read **INTEGRATION.md** for the module API, data fields, keyboard ownership, layout coordinates, transparent margins and lifecycle. **IMPLEMENT_WITH_CODEX.md** is a concrete handoff for connecting this pack to the real game.

The preview uses system fonts, so no font download or external font license is required. Text stays editable and can be localized. Shared icons are included in every ZIP so each folder works independently.

## Reference and generation notes

The reusable assets and the two new cockpit references were made using the built-in image generator, then copied without modifying their PNG bytes. The original exterior and comparison board are preserved references. Generated cutaways illustrate the design; they are not calibrated CAD or renders of one shared validated 3D mesh. A true 3D interior, pivots and exact cockpit camera anchor still need construction in Blender if that is the chosen implementation.

The source paths in prompt/provenance records identify this generation session. Portable reference copies are in **references/**. Failed draft images are not included.

## Validation

All PNGs decode, every reusable PNG has real transparency, windshield center samples are fully transparent, required files and prompt links exist, and archive CRC checks are performed during packaging. The preview JavaScript passed syntax and DOM behavior checks covering data updates, view toggling, key-repeat suppression, edit-field suppression, state handling, host callbacks and cleanup.

**Browser-rendered layout testing was unavailable in this environment.** Layouts were measured against and visually compared with the generated source images; they should be checked in the actual game viewport. See **docs/preview_validation.json** and **docs/asset_validation.json** for the recorded checks.

These are usable raster layers, an example UI module, and modeling references. The live Roadsters Race project was not opened or changed, and no GLB/Blender model, rig or PBR texture set is included.
