# car03 — Layered armored flying grand-tourer references

Extract `car03.zip` into the game project root. The archive contains the folder `Assets/01_Vehicle/car03/`.

This pack develops the **layered armored grand-tourer** shown in the user's exact selected attachment. That attached image is the design authority and is preserved as `concept.png`. The pack contains PNG modeling references and documentation. A GLB, Blender model, rig, or game integration is not included.

## Files

| File | Purpose |
| --- | --- |
| `concept.png` | Exact selected attached image, preserved unchanged; primary authority for the body design, armor, and materials. |
| `front.png` | Straight front reference. |
| `side.png` | Left-side reference, with the nose pointing right. |
| `rear.png` | Straight rear reference establishing the larger central nozzle and two smaller side nozzles. |
| `top.png` | Top reference, with the nose pointing up; shows all four pod positions. |
| `prompt.md` | Exact generation prompts and refinement history. |
| `README.md` | Package contents, design requirements, and modeling handoff notes. |

## Design and integration requirements

These requirements come from the supplied car03 brief:

- Preserve a wide, low, heavy armored coupe: blunt front, stacked hood armor, rectangular magenta headlights, compact tinted canopy, short fastback, and overlapping angular flank plates.
- Keep the body predominantly neutral obsidian black or graphite with neutral chrome trim. Confine saturated color to thin **magenta-dominant** light strips, restrained turbine details, and small cyan accents. This supports the intended repaint and emissive classification.
- Build **exactly four separate metallic cylindrical lift pods** at conventional wheel positions. Each sits outside the body on a short mechanical arm, with a clear air gap around the connection. Their axes run transversely, left to right like wheel axles; circular turbine faces point outward to the sides. Use no rubber or tires.
- Provide **one larger central rear nozzle and two smaller symmetric side nozzles**, built into the tail and distinct from the lift pods.
- Target approximately **5.6 m overall length**. Total width, including pods, must remain at or below **70% of length**—about **3.92 m** at that length.
- Keep the low rear spoiler within body width, clear of the rear pods. The front weapon aperture stays flush and recessed, without a projecting barrel or firing effect.

## Using the views

Use `concept.png` as the design authority, `rear.png` for the authored nozzle layout, and `top.png` with `side.png` for pod placement.

These are generated references intended for orthographic interpretation, not calibrated CAD drawings or projections from a shared validated mesh. Image framing does not establish equal pixels per meter. Reconcile small differences in Blender, set physical dimensions explicitly, and measure pod pivots and nozzle, flame, trail, and turbine-ring attachment coordinates on the finished model.

The actual `src/car.js`, `meshy/build_game_car.py`, and current runtime model were not inspected for this package. Confirm their naming, axes, material handling, attachment expectations, and in-game clearance before integration.
