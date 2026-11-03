# Roadsters Race cockpit interface integration

This pack contains illustrated cockpit assets, blank instrument panels, separate controls, and a dependency-free browser overlay. The selected design is identified in `preview/pack-config.js`. The original game repository was not inspected or changed while making this pack.

## Open the preview

Unzip the complete folder, then open `preview/index.html` in a desktop browser. Keep the folder structure intact. No web server, build step, package installation, external font, or internet connection is required.

- Press **I** or click **SWITCH VIEW** to hide/show the cockpit overlay.
- Adjust speed, boost, steering, and attack state below the preview.
- The optional timer animation uses illustrative elapsed time.
- **Browse individual assets** opens a gallery of the original PNG files.

The colored skyline in the preview is a simple CSS illustration. It is not a game screenshot or a map asset. The external-view message marks where the game would render its chase camera. The preview does not implement racing, flight, damage, or a 3D camera.

## Files used by the overlay

| Path | Role |
| --- | --- |
| `artwork/cockpit_base.png` | Main cockpit shell with a transparent windshield and blank instrument surfaces. |
| `artwork/control_neutral.png` | Separate steering yoke or wheel for car01/car03. |
| `artwork/control_left.png`, `artwork/control_right.png` | Separate car02 side grips. |
| `panels/speed_panel.png` | Empty speed screen housing. |
| `panels/boost_panel.png` | Empty charge gauge housing. |
| `panels/attack_panel.png` | Empty attack-status housing. |
| `panels/race_panel.png` | Empty lap/timer housing. |
| `ui/icon_boost.png`, `ui/icon_attack.png` | Separate boost/attack pictograms. |
| `ui/icon_camera.png`, `ui/icon_warning.png` | Camera and unavailable-state pictograms. |
| `ui/reticle.png` | Small transparent aiming mark. |
| `ui/keycap_i.png` | Optional static I-key artwork; the sample uses an accessible HTML keycap. |
| `references/cockpit_preview.png` | Composition reference containing illustrative sample readings. |
| `references/interior_modeling_reference.png` | Interior design reference for later 3D construction. |

Every changing number, label, and meter in the interactive overlay is rendered by HTML/CSS. Do not use the preview reference image as the live HUD, since its example numbers are baked into that reference.

The supplied base already contains the dashboard's instrument housings. Calibrated layouts can set a panel's `path` to `null` and place live text directly in those blank surfaces. The separate `panels/` PNGs remain available for alternate HUD layouts, closer display details, or use during 3D cockpit construction. They do not need to be layered a second time over a housing already present in the base.

## Host integration

Include the stylesheet and plain script in the game page, adjusting paths to match the location where the pack is installed:

```html
<link rel="stylesheet" href="assets/car01_interface/preview/cockpit-ui.css">
<script src="assets/car01_interface/preview/cockpit-ui.js"></script>
```

Mount the overlay on a positioned element with a real, non-zero width and height. That element can contain the game's existing WebGL canvas; the overlay appends its own layer and does not replace the canvas.

```js
const cockpit = RoadstersCockpit.mount(gameViewportElement, {
  assetBaseUrl: 'assets/car01_interface/',
  theme: 'car01', // 'car02' or 'car03' for the other packs
  initialView: 'external',
  keyboard: true,
  layout: yourCalibratedLayout,
  onViewChange(view) {
    // Connect this callback to the project's EXISTING camera manager.
    // 'cockpit' should select the cockpit camera; 'external' the chase camera.
  }
});

// Feed these values from the actual race state.
cockpit.update({
  speedKmh: 338,
  lap: 1,
  totalLaps: 3,
  raceTimeMs: 24170,
  boost: 0.78,
  attackState: 'ready',
  attackCooldownMs: 0,
  steering: 0
});
```

`onViewChange` fires once when mounted to synchronize the initial view, then whenever the view changes. It does not choose camera positions or change rendering itself. Inspect the actual `src/car.js` and camera ownership before connecting it. A car may need a new cockpit camera anchor and exterior-mesh visibility adjustments; those coordinates cannot be inferred precisely from illustration alone.

### Public API

| Method | Behavior |
| --- | --- |
| `RoadstersCockpit.mount(element, options)` | Creates one overlay instance and returns its controller. |
| `controller.update(partialData)` | Updates only supplied values. |
| `controller.setView('cockpit' or 'external')` | Selects a view and notifies the host callback if it changes. |
| `controller.getState()` | Returns a copy of the current display state. |
| `controller.getMissingAssets()` | Lists PNG paths that failed to load. |
| `controller.destroy()` | Removes the overlay, keyboard/click listeners and resize observer. Safe to call twice. |
| `RoadstersCockpit.formatRaceTime(milliseconds)` | Formats elapsed time as minutes:seconds.hundredths. |

Options also include `onAssetError(path)` for optional diagnostics. The script exposes `RoadstersCockpit.themes` and `RoadstersCockpit.layouts` for inspection. Defaults are copied for each instance.

### Data contract

| Field | Units / accepted values |
| --- | --- |
| `speedKmh` | Nonnegative number, displayed rounded to an integer. |
| `lap`, `totalLaps` | Positive integers. The host owns race completion rules. |
| `raceTimeMs` | Nonnegative elapsed milliseconds. |
| `boost` | Fraction from `0` to `1`; displayed as a percentage and bar. |
| `attackState` | `ready`, `cooldown`, or `disabled`. |
| `attackCooldownMs` | Nonnegative remaining milliseconds; shown only during cooldown. |
| `steering` | Fraction from `-1` to `1`, used for illustrative control rotation. |

The host must explicitly change `attackState` when an ability becomes ready. A zero cooldown value does not invent that gameplay transition. The overlay does not consume input for attack, accelerate, boost, steer the car, or change race state.

## Keyboard ownership and lifecycle

The built-in listener reacts to **I**, ignores held-key repeat and modified shortcuts, and ignores editable fields and focused buttons. It honors an event already marked `defaultPrevented`. The on-screen switch remains available.

If the game already has an input manager, set `keyboard: false` and call `setView` from that manager. This avoids having two owners for the same key. Use only one active cockpit instance per player viewport. Call `destroy()` on vehicle changes or when unloading the game scene, then mount the newly selected car's interface. Destroy does not switch the host camera; the host still owns its final camera state.

## Layout calibration

The artwork is fitted to a logical canvas with the configured `aspectRatio`. Coordinates are fractions of that fitted canvas, so resizing preserves placement. Any bars around the fitted canvas belong to the host viewport.

```js
const yourCalibratedLayout = {
  aspectRatio: [1536, 1024], // Use actual base PNG dimensions.
  base: {
    box: [0, 0, 1, 1]
  },
  panels: {
    speed: {
      box: [0.40, 0.60, 0.20, 0.14],
      // Optional alpha-content bounding rectangle in the source PNG:
      contentBox: [0.10, 0.18, 0.80, 0.64],
      // Optional readable region within the placed panel:
      textBox: [0.18, 0.20, 0.64, 0.60]
    }
  },
  controls: [
    {
      path: 'artwork/control_neutral.png',
      box: [0.30, 0.72, 0.40, 0.28],
      contentBox: [0.10, 0.20, 0.80, 0.60],
      rotation: 16,
      origin: '50% 65%'
    }
  ]
};
```

`box`, `contentBox`, and `textBox` are `[x, y, width, height]`, all normalized. These example numbers are illustrative. For a pixel alpha bounding box `[left, top, right, bottom]`, divide its position and size by the PNG width/height to obtain `contentBox`. That option trims transparent margins in CSS without modifying the source image. Keep the placed box's aspect ratio consistent with the content to avoid stretching. Omit `contentBox` to use regular `object-fit: contain` on the complete PNG.

A panel may also specify `fontScale` (for example, `1.2`) to adjust the size of its live text relative to the fitted canvas. Use `path: null` for text-only placement on a display already drawn into the cockpit base.

`controls` replaces the whole controls array when overridden. `panels`, `base`, and `reticle` accept partial overrides. Neutral control rotation is zero; rotation amounts are artwork animation suggestions and must be tuned to the host game's steering feedback.

The provided positions were measured against the delivered base PNGs and control alpha bounds; they still need to be checked at the actual game resolution and field of view. Car03's wheel is deliberately foreshortened vertically to suit the illustrated driver's viewpoint, with its lower edge clipped by the viewport. Its otherwise blank left round instrument contains a small decorative reticle, not additional gameplay telemetry. Illustrated components are not renders of a shared calibrated 3D model. If a true 3D interior is required, use the reference images and prompt files to build the dashboard, controls, glass, materials and camera anchor, then render live instruments onto the intended display surfaces.

## Validation boundaries

The kit's JavaScript syntax and display-controller behavior can be tested independently of the game. Successful preview operation does not validate the game's camera collision, cockpit clipping, frame rate, steering pivots, attack mechanics, mobile input, or integration with `src/car.js`. Those checks belong to the host implementation after its actual code is available.
