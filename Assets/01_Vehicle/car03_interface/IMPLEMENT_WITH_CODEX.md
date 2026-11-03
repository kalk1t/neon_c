# Implement CAR03 3A — Armored Sport

Use this folder as the approved cockpit-interface art pack for Roadsters Race.

1. Inspect the actual game's entry point, input manager, current cameras, renderer, race state and **src/car.js** before changing anything. Do not assume names or camera anchors from these illustrations.
2. Preserve the selected design: **3A Armored Sport**, magenta dominant with small cyan accents, heavy flat-bottom steering wheel. Use **references/cockpit_preview.png** as the composition guide.
3. Start with the included raster foreground overlay and live HTML instruments unless the owner requests a modeled interior. Keep **artwork/cockpit_base.png** transparent over the actual game view. Use the separate control artwork for restrained steering feedback.
4. Integrate **preview/cockpit-ui.js** and **preview/cockpit-ui.css** using the public API in **INTEGRATION.md**. Use theme **car03**. Load this pack's assets only while this car is selected; dispose of the previous interface on car changes.
5. Bind **I** once through the existing input owner. Toggle chase/external camera and cockpit camera together with the visible cockpit overlay. Ignore repeated held-key events, typing fields, modal menus and already-consumed events as appropriate to the existing game. If the game owns the listener, mount with **keyboard:false**.
6. Connect speed, lap, timer, boost and attack status to the real race/ability state. Do not use the reference image's example numbers as data. The host controls cooldown-to-ready transitions.
7. Place and verify a cockpit camera anchor for the actual car model. Check windshield visibility, hood placement, clipping, field of view, camera motion and whether exterior body geometry should be hidden from that camera.
8. Keep boost and attack behavior unchanged while integrating their displays. Update the game loop via **controller.update(...)**; use **onViewChange** to choose the real camera. Use one overlay instance per local player.
9. Check the installed view at the supported desktop aspect ratios and the game's target resolution. Adjust normalized layout and font sizes using the actual display. The supplied artwork remains 2D; large viewpoint changes require a real 3D cockpit.
10. Verify view switching during racing, after respawn/restart, while changing cars, and when opening menus. Check zero/full boost, attack ready/cooldown/unavailable, long race times, no duplicate key handlers, missing-asset handling and teardown. Report actual game validation separately from the provided asset-pack checks.

No deployment, account action, game-economy change or Meshy generation is required to use this pack. If a fully 3D interior is later requested, use **references/interior_modeling_reference.png**, the blank panel skins and item prompts to construct it, then measure pivots and camera placement in the actual mesh.
