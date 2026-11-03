# Meshy workflow for Claude

## Choose where generation helps

Use Meshy for a rock/boulder seed, vegetation shape, an ornamental prop or a distinctive detail whose silhouette can tolerate cleanup. Use Blender directly for track dimensions, gates, modular seams, structural dam panels, clear portals, collision and sign text. Water, mist, rain and route-light arrays should be built as runtime effects or simple procedural geometry.

Each PNG in Assets/Environment is one item. Feed **one isolated item** to an Image-to-3D task. The selected map scene is an art-direction reference, not a suitable input for generating the entire level as one mesh. These packs do not contain calibrated multi-view sets. Meshy's multi-image route expects views of the same object; do not submit a cliff, tree and dam together as if they were different views.

## Establish the installed workflow

Use the project's installed Meshy integration or the official meshy-cli. Read its current help and account status before adapting the example below:

~~~bash
meshy --version
meshy doctor
meshy auth status
meshy balance
meshy image-to-3d create --help
meshy image-to-3d wait --help
~~~

These packs contain no executed Meshy tasks, credentials, credit allowance or purchased models. Honor any budget and authorization already provided in the implementation session. Do not import an unrelated project's credit allowance. If no generation allowance exists, complete the Blender blockout and a concrete list of proposed Meshy items/costs before requesting one.

Current models and supported flags can change. Choose a supported model and inspect the selected model's current texturing/topology options. A target face count is a request, not proof of the actual output count.

## One-item example, after the applicable budget is established

Replace ITEM.png with the desired isolated PNG and confirm these flags in local help. This uses the official CLI workflow; it is documentation, not a job already submitted:

~~~bash
meshy image-to-3d create --image-url "./Assets/Environment/ITEM.png" --model-type standard --should-texture true --enable-pbr true --target-formats glb --async --output-schema v1 --format json --no-update-check
~~~

Record the returned resource type and task ID immediately. Record the exact input PNG, its hash, the command/options and the accepted cost estimate. Follow that **same task** through completion; do not submit another create command because polling or a client connection timed out. Resolve ambiguous outcomes using the existing task ID.

~~~bash
meshy image-to-3d wait TASK_ID --timeout 600 --output-schema v1 --format json --no-update-check
meshy download --resource image-to-3d --task-id TASK_ID --list
~~~

Check download help for the installed version, choose the returned GLB artifact and save the task metadata with it. If using the official Meshy project workflow, initialize its project from the first returned task ID and keep later stages in that project. Save actual consumed credits separately from the preflight estimate.

For image-driven geometry, the PNG is the shape input. The suggested per-item Meshy wording is a construction brief; do not assume the CLI has a freeform geometry-prompt flag. If the installed workflow supports a texture prompt, use only the material-relevant part there. Keep the full exact request payload for reproducibility.

## Blender cleanup of each generated item

1. Inspect the downloaded GLB before accepting it. Remove floating fragments, hidden support plates, fused background pieces and unwanted internal geometry.
2. Establish the specified size and pivot in meters. Check openings, especially the rock arch or twin tower portal. Rebuild these precisely if the generated geometry closes them.
3. Separate structural mesh, glass/foliage and emissive trim as needed. Do not assume generated textures include an emission map; create controlled trim materials or masks in Blender when needed.
4. Repair normals and visible holes. Bake/retopologize when necessary. Produce LOD1/LOD2 from the accepted asset rather than paying to recreate a different design for every LOD.
5. Create low-cost collision proxies independently. Thin foliage, mist, water and decorative trim normally have no collision.
6. Export a GLB through the game's established pipeline, then inspect it under the actual lighting. Record measured triangles, materials, texture sizes, file size and in-game result in Meshy/task_ledger.json.

The source ledger is intentionally empty. A prompt and target budget do not mean a model was generated or verified.

Official sources and version checks: SOURCES.md.
