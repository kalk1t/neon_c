# Car03 interface — 3A Armored Sport

The user selected **3A**, the top row of the Car03 comparison board. This pack follows the layered armored GT, with a heavy flat-bottom wheel, two round chrome instrument housings, a central rectangular speed display, a small right-hand attack display, black/graphite armor and magenta lighting.

## Layer use

- `artwork/cockpit_base.png`: stationary foreground overlay. Its windshield and exterior viewing area are truly transparent. Gauge faces are opaque blank dark glass. The steering wheel and all dynamic readouts have been removed.
- `artwork/control_neutral.png`: separate complete wheel for gentle steering rotation about its hub. The openings through its spokes are transparent. Its natural framing differs from the full-screen base; position and size it with layout coordinates, rather than stretching it across the screen.
- `panels/speed_panel.png`, `boost_panel.png`, `attack_panel.png`, `race_panel.png`: separate front-facing blank instrument skins for alternative UI placement, close-ups, or model texturing guidance. They do not need to be stacked over the already illustrated dashboard bezels. Use live HTML/canvas text and meter graphics over them.
- `references/cockpit_preview.png`: a single illustrative first-person composition with example race values and city. It is a reference, not the transparent runtime layer.
- `references/interior_modeling_reference.png`: a perspective cutaway illustrating material, structure, seats and dashboard for a future modeled interior.

## Placement hints

The base is 1672 × 941 pixels. Its central display interior is approximately x=0.422–0.576, y=0.484–0.638 in normalized coordinates. The left and right round gauge centers are (0.344, 0.582) and (0.657, 0.582). Each circle's usable radius is approximately 0.080 of the frame height. The right-hand attack screen occupies approximately x=0.752–0.893, y=0.601–0.738. Keep critical text above the steering rim. Calibrate to the final game's viewport and camera.

## Scope and limitations

These are generated raster art components and illustrative modeling references. They are not a calibrated 3D mesh, CAD drawings, rig, PBR material set, or a completed change to the running game. The cutaway gives a material/design direction; it is not a dimensionally exact reconstruction of the first-person artwork. Reusable layers contain no baked speed, timer, boost value, attack state or I-key label. The runtime must supply those values and actual camera switching.

All eight images were generated with the built-in image generation tool. Exact prompts and reference paths are recorded individually under `prompts/`. PNG files are copied unchanged from generator output. `asset_records.json` records native sizes, alpha bounds, source paths and hashes.
