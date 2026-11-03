import bpy, sys
path = sys.argv[sys.argv.index('--')+1]
bpy.ops.wm.read_factory_settings(use_empty=True)
bpy.ops.import_scene.gltf(filepath=path)
tris = 0
for o in bpy.data.objects:
    if o.type == 'MESH':
        o.data.calc_loop_triangles(); tris += len(o.data.loop_triangles)
        print('MESH', o.name, len(o.data.loop_triangles), [m.name for m in o.data.materials])
for im in bpy.data.images: print('IMG', im.name, im.size[:], im.file_format)
for m in bpy.data.materials:
    if m.use_nodes:
        b = m.node_tree.nodes.get('Principled BSDF')
        if b: print('MAT', m.name, 'metal', round(b.inputs['Metallic'].default_value,2), 'rough', round(b.inputs['Roughness'].default_value,2), 'links', [l.to_socket.name for l in m.node_tree.links])
print('TRIS', tris)
