# Source references and provenance

Prepared 2026-10-31. All geometry dimensions, layout points, material starting values and performance budgets in this pack are newly authored design choices unless explicitly identified as user requirements.

## User-approved visual sources

- References/selected_map.png is the exact selected scene for this map, copied unchanged.
- The C-series shared structural images reuse the approved Neon Circuit B individual course assets, except C03/C11/C12/C13 which were generated for this handoff.
- The N-series/F-series items were generated individually using the selected map as an art-direction reference.
- Every item's exact image prompt and modeling notes are in Prompts/Items. The selected scene's original prompt is in Prompts/selected_map_prompt.md.
- AI illustrations are visual references. They are not calibrated CAD views, meshes, PBR map sets or evidence that the resulting game will meet performance targets.

## Primary technical references

1. Official Meshy CLI and examples: https://github.com/meshy-dev/meshy-cli
2. Official CLI usage: https://github.com/meshy-dev/meshy-cli/blob/main/docs/usage.md
3. Official Image-to-3D API/task and output formats: https://docs.meshy.ai/en/api/image-to-3d
4. Official multi-image input semantics: https://docs.meshy.ai/en/api/multi-image-to-3d
5. Official remesh workflow: https://docs.meshy.ai/en/api/remesh
6. Meshy task lifecycle/quick start: https://docs.meshy.ai/en/api/quick-start
7. Maintained Blender glTF exporter material/export documentation: https://github.com/KhronosGroup/glTF-Blender-IO/blob/main/docs/blender_docs/scene_gltf2.rst
8. Blender glTF exporter operator parameters (4.4 reference; inspect the installed version): https://docs.blender.org/api/4.4/bpy.ops.export_scene.html
9. Blender 4.5 LTS glTF manual: https://docs.blender.org/manual/en/4.5/addons/import_export/scene_gltf2.html
10. Khronos glTF specification: https://github.com/KhronosGroup/glTF/blob/main/specification/2.0/Specification.adoc

The docs were consulted for workflow/export compatibility; model names, CLI flags and supported features should still be checked against the version installed at implementation time. No external asset marketplace models, textures or code were downloaded for this handoff.
