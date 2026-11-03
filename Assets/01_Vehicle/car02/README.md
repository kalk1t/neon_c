# car02 — Flying interceptor references

Extract `car02.zip` into the game project root. The archive contains the folder `Assets/01_Vehicle/car02/`.

This pack contains PNG references for building the approved car design. A GLB, Blender model, rig, or game integration is not included.

## Files

| File | Purpose |
| --- | --- |
| `concept.png` | The approved hero image, preserved unchanged; primary authority for the design and materials. |
| `front.png` | Straight front reference. |
| `side.png` | Left-side reference, with the nose pointing right. |
| `rear.png` | Straight rear reference establishing the central nozzle and two side nozzles. |
| `top.png` | Top reference, with the nose pointing up; shows the four pod positions. |
| `prompt.md` | Exact concept-generation history and prompts used for the additional views. |

## Design and integration requirements

These requirements come from the supplied car02 brief:

- Keep the body predominantly neutral black or graphite, with neutral chrome trim. Confine saturated cyan and magenta to thin lighting strips and small accents so the body can support the existing repaint and emissive treatment.
- Build four separate metallic thruster pods at conventional wheel positions, suitable for steering tilt. Use no rubber or tires.
- Provide one central rear turbine nozzle and a symmetric pair of rear side nozzles. Measure their attachment positions on the completed model.
- Target approximately **5.6 m overall length**, with a compact, low body compatible with the existing car envelope.
- Keep the front weapon aperture flush and recessed. Include no protruding barrel or firing effect in the model reference.

## Using the views

Use `concept.png` to preserve the long wedge nose, canopy, faceted body, materials, and lighting accents. Use `rear.png` for the authored three-nozzle arrangement, which the hero angle does not fully reveal, and `top.png` to establish the four corner pod positions.

The additional views are generated modeling references, not projections from a shared, validated 3D mesh or calibrated CAD drawings. Framing is intended for visual readability; equal image occupancy does not establish equal pixels per meter. Reconcile any small differences in Blender, set the final model scale explicitly, and measure nozzle, trail, flame, and turbine-ring attachment coordinates on the finished geometry.

The actual `src/car.js`, `meshy/build_game_car.py`, and current car model were not inspected for this package. Confirm their naming, axes, pivots, material handling, and attachment expectations before exporting and integrating the new model.
