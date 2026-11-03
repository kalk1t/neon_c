# Rendering revision after the reported freeze

2026-11-02. The incident's root cause remains unknown. Neither native nor browser graphics was launched to validate this revision. No sustained performance or driver-stability claim is made.

## Browser comparison

Read-only inspection of the current browser source shows bloom (`game/src/main.js`), merged city geometry with bounding spheres (`city.js`), merged/instanced prefabs (`geo.js`), instanced route dots (`course.js`), and an automatic switch to low graphics after a slow menu start (`main.js`). The source inspected is the current working tree, not a newly captured/frozen release baseline.

The previous native post shader performed 26 scene texture reads per pixel. This is an operation count, not a diagnosis: it does not establish execution time, GPU utilization, a driver hang, or why Windows stopped responding. Browser bloom also performs substantial GPU work. Browser/native performance must eventually be compared on identified hardware with actual measurements.

The old native city submitted primitive geometry each frame. raylib batches many primitive submissions internally; counting DrawCube calls as separate GPU draw calls would be incorrect. Cached geometry still avoids repeated CPU geometry construction/submission.

## Implemented behavior

- Startup is a 960x720 window with a 2D menu. The model, world shaders, city GPU meshes, and 3D render target are deferred until an explicit Start Flight action. Pause/menu never render the world.
- The internal 3D target preserves aspect ratio and never exceeds 960x600, including after fullscreen/resizing. UI stays at window resolution. Target allocation failure returns to a 2D error menu; invalid dimensions are rejected.
- MSAA is disabled; glow is off by default. Explicit `--glow` enables the existing effect at bounded internal resolution.
- Camera frustum sphere tests reject geometry outside the view, including behind the camera. A nearest-first selection caps city rendering at 64 buildings within 1,000 world units. Selection updates every rendered flight frame.
- Each building's body, roof, and three window bands per side are combined into one static vertex-coloured mesh. Geometry is built only on first visibility and reused. At most two new building meshes are uploaded per frame; pending buildings use a single cheap cube silhouette. Previously seen meshes remain cached for this finite city and are released with the scene. This is not an unbounded world streaming implementation.
- Gates and guidance markers are culled by view/distance and use less geometry. Decorative traffic count is reduced. Flight/checkpoint simulation code and recorded browser fixtures are unchanged.
- Normal rendering is capped at 30 FPS, unfocused rendering at 5 FPS; loss of focus pauses flight. Minimized native windows poll events and sleep without rendering. Web builds use browser scheduling instead of sleeping on the main thread. Higher-quality choices and measured tuning remain future work.
- Ten consecutive measured frame durations over 150 ms pause flight and display a message. This only acts when control returns to the game; it cannot interrupt a blocked driver call. Frame durations include presentation/frame pacing and are not isolated GPU timings.
- Native raylib logs, including reported OpenGL vendor/renderer/version, scene-loading stages, periodic frame/target information, and clean shutdown, append to `logs/roadsters.log` beside the executable. If opening the log fails, output goes to stderr. Logs are flushed after each message. Startup still needs an OpenGL context even though its screen is 2D.

## Non-graphics verification

Native MSVC C17 compilation uses `/W4 /WX`. WebAssembly compilation uses the pinned Emscripten/raylib toolchain. Both targets run headless tests for the existing 2,940 browser flight steps, curve/gate fixtures, bounded render sizes (including extreme aspect ratios/integers), camera side/near/far visibility and edge intersection, rotated cameras, and overload pause thresholds.

These checks do not execute shaders, validate uploaded meshes on a GPU, check final screenshots, measure FPS, or prove the freeze fixed. Graphics execution requires a new explicit user instruction. W0/W1 and later milestone acceptance gates remain incomplete.
