"""Living Megacity: room interiors for the window shader (interior mapping with a pre-rendered room atlas).

  blender -b --factory-startup --python rooms.py -- <outdir> [family ...]

Each room is a real 3D box room (W x H x D metres, open at y = 0 where the glass is) furnished with simple models
and lit by its ceiling lights, rendered in Cycles from a camera in front of the opening whose frame is exactly the
opening (one-point perspective; the back wall fills the middle half of the image: s = F / (F + D) = 0.5, so F = D).
The game ray-casts the box and looks the hit point up in this image (facades.js), so walls, floor, ceiling and
furniture appear with parallax behind every window.

Writes per family: <fam>_rooms.png (RGB, lit render; a 2 x 4 atlas of eight rooms) and <fam>_rooms_glow.png
(grey: what still glows when the room's lights are off: screens, standby lights, lamps), plus rooms.json.
"""
import bpy, sys, os, json, math, random
from mathutils import Vector

A = sys.argv[sys.argv.index('--') + 1:]
OUT = os.path.abspath(A[0])
WANT = A[1:]
os.makedirs(OUT, exist_ok=True)
CELL_W = 1024   # pixels per room across; the height follows the room's aspect
COLS, ROWS = 2, 4
FAM = {
    # W = rooms span cluster x bay (facades.js), H = ceiling, D = room depth (render and shader must agree)
    # ZF = depth of the furniture card (the second layer: furniture, chairs, hanging lamps; facades.js uFurnZ)
    'office': dict(W=9.0, H=3.7, D=9.0, ZF=4.5, rooms=['open_plan', 'meeting', 'executive', 'open_plan_b',
                                                'lift_lobby', 'fit_out', 'break_room', 'archive']),
    'residential': dict(W=6.8, H=2.75, D=6.5, ZF=3.2, rooms=['living', 'bedroom', 'kitchen', 'home_office',
                                                      'kids', 'living_b', 'dining', 'bedroom_b']),
    # hotel: one room per bay (cluster 1, 3.6 m), each window its own room
    'hotel': dict(W=3.6, H=2.75, D=7.0, ZF=3.5, rooms=['king', 'twin', 'suite', 'king_b', 'twin_b', 'suite_b',
                                                'king_desk', 'corner_king']),
    # pencil (BLD-11): one 4.75 m bay per room, 4.0 m ceilings, deep luxury apartments
    'pencil': dict(W=4.75, H=4.0, D=8.0, ZF=4.0, rooms=['living', 'bedroom', 'kitchen', 'home_office', 'living_b',
                                                 'dining', 'bedroom_b', 'living']),
    'transport': dict(W=12.0, H=4.3, D=10.0, ZF=5.0, rooms=['control', 'waiting', 'workshop', 'staff_office',
                                                     'control_b', 'stair_hall', 'lockers', 'waiting_b']),
}


def reset():
    bpy.ops.wm.read_factory_settings(use_empty=True)
    s = bpy.context.scene
    s.render.engine = 'CYCLES'
    s.cycles.device = 'CPU'
    s.cycles.use_denoising = True
    s.view_settings.view_transform = 'Standard'   # the game applies its own tone mapping (ACES)
    w = bpy.data.worlds.new('w')
    w.use_nodes = True
    w.node_tree.nodes['Background'].inputs['Strength'].default_value = 0.0
    s.world = w
    return s


MATS = {}


def mat(name, col, rough=0.6, metal=0.0, emit=0.0, emit_col=None):
    key = (name, col, rough, metal, emit, emit_col)
    if key in MATS:
        return MATS[key]
    m = bpy.data.materials.new(name)
    m.use_nodes = True
    b = m.node_tree.nodes['Principled BSDF']
    b.inputs['Base Color'].default_value = (*col, 1)
    b.inputs['Roughness'].default_value = rough
    b.inputs['Metallic'].default_value = metal
    if emit:
        b.inputs['Emission Color'].default_value = (*(emit_col or col), 1)
        b.inputs['Emission Strength'].default_value = emit
    m['glow'] = emit
    MATS[key] = m
    return m


def box(x0, x1, y0, y1, z0, z1, m):
    bpy.ops.mesh.primitive_cube_add(size=1, location=((x0 + x1) / 2, (y0 + y1) / 2, (z0 + z1) / 2))
    o = bpy.context.object
    o.scale = (max(x1 - x0, 0.005), max(y1 - y0, 0.005), max(z1 - z0, 0.005))
    o.data.materials.append(m)
    return o


def cyl(x, y, z0, z1, r, m, verts=16):
    bpy.ops.mesh.primitive_cylinder_add(radius=r, depth=z1 - z0, location=(x, y, (z0 + z1) / 2), vertices=verts)
    o = bpy.context.object
    o.data.materials.append(m)
    return o


# ---------------------------------------------------------------- shared furniture
def shell(W, H, D, wall, floor, ceil):
    for o in (box(-0.2, W + 0.2, D, D + 0.2, -0.2, H + 0.2, wall),          # back wall
              box(-0.2, 0, -0.1, D, -0.2, H + 0.2, wall),                    # left wall
              box(W, W + 0.2, -0.1, D, -0.2, H + 0.2, wall),                 # right wall
              box(-0.2, W + 0.2, -0.1, D + 0.2, -0.2, 0, floor),             # floor
              box(-0.2, W + 0.2, -0.1, D + 0.2, H, H + 0.2, ceil)):          # ceiling
        o['layer'] = 'box'


def ceiling_lights(W, H, D, kind='panel', warm=False, power=1.0):
    col = (1.0, 0.82, 0.62) if warm else (0.92, 0.96, 1.0)
    if kind == 'panel':
        for x in [W * (i + 0.5) / max(1, round(W / 3)) for i in range(max(1, round(W / 3)))]:
            for y in (D * 0.25, D * 0.6, D * 0.9):
                box(x - 0.6, x + 0.6, y - 0.3, y + 0.3, H - 0.03, H, ceiling_mat(mat('panel', col, emit=5 * power, emit_col=col)))['layer'] = 'box'
                add_light(x, y, H - 0.1, col, 45 * power)
    else:  # pendants / downlights
        for x in [W * (i + 0.5) / max(1, round(W / 2.2)) for i in range(max(1, round(W / 2.2)))]:
            y = D * 0.45
            cyl(x, y, H - 0.9, H, 0.012, mat('wire', (0.05, 0.05, 0.05)))
            cyl(x, y, H - 1.05, H - 0.9, 0.22, ceiling_mat(mat('shade', col, emit=6 * power, emit_col=col)))
            add_light(x, y, H - 1.1, col, 40 * power)


def ceiling_mat(m):
    m['ceiling'] = 1   # the room's main lights: off in the glow layer (dark rooms keep only screens and small lights)
    return m


def add_light(x, y, z, col, watts):
    l = bpy.data.lights.new('l', 'POINT')
    l.energy = watts
    l.color = col
    l.shadow_soft_size = 0.4
    o = bpy.data.objects.new('l', l)
    o.location = (x, y, z)
    bpy.context.scene.collection.objects.link(o)


def desk(x, y, rnd, screen=True, facing=1):
    top = mat('desk', rnd.choice([(0.75, 0.72, 0.66), (0.55, 0.42, 0.3), (0.9, 0.9, 0.88)]), 0.5)
    leg = mat('leg', (0.15, 0.15, 0.16), 0.4, 0.8)
    box(x - 0.8, x + 0.8, y - 0.4, y + 0.4, 0.72, 0.75, top)
    for dx in (-0.75, 0.75):
        box(x + dx - 0.02, x + dx + 0.02, y - 0.35, y + 0.35, 0, 0.72, leg)
    if screen:
        sy = y + 0.25 * facing
        box(x - 0.3, x + 0.3, sy - 0.02, sy + 0.02, 0.85, 1.2, mat('bezel', (0.03, 0.03, 0.035), 0.3))
        sc = rnd.choice([(0.35, 0.55, 1.0), (0.85, 0.9, 1.0), (0.4, 0.9, 0.8)])
        box(x - 0.28, x + 0.28, sy - 0.03 * facing - 0.005, sy - 0.03 * facing + 0.005, 0.87, 1.18, mat('screen', sc, emit=2.5, emit_col=sc))
        box(x - 0.03, x + 0.03, sy - 0.03, sy + 0.03, 0.75, 0.87, leg)
    chair(x, y - 0.75 * facing, rnd)


def chair(x, y, rnd):
    c = mat('chair', rnd.choice([(0.08, 0.08, 0.09), (0.25, 0.1, 0.1), (0.1, 0.15, 0.25)]), 0.7)
    box(x - 0.25, x + 0.25, y - 0.25, y + 0.25, 0.42, 0.48, c)
    box(x - 0.25, x + 0.25, y - 0.27, y - 0.22, 0.48, 1.0, c)
    cyl(x, y, 0, 0.42, 0.03, mat('leg', (0.15, 0.15, 0.16), 0.4, 0.8))


def plant(x, y, h=1.2):
    cyl(x, y, 0, 0.45, 0.22, mat('pot', (0.2, 0.19, 0.18), 0.8))
    bpy.ops.mesh.primitive_ico_sphere_add(radius=h * 0.38, location=(x, y, 0.45 + h * 0.35), subdivisions=2)
    o = bpy.context.object
    o.scale = (1, 1, 1.3)
    o.data.materials.append(mat('leaf', (0.08, 0.22, 0.07), 0.7))


def shelf(x0, x1, y, h, rnd):
    wood = mat('shelf', (0.42, 0.3, 0.2), 0.6)
    box(x0, x1, y - 0.18, y + 0.18, 0, 0.03, wood)
    for k in range(1, 5):
        z = h * k / 4
        box(x0, x1, y - 0.18, y + 0.18, z - 0.02, z, wood)
        xs = x0 + 0.05
        while xs < x1 - 0.1:
            w = rnd.uniform(0.03, 0.08)
            hh = rnd.uniform(0.18, h / 4 - 0.06)
            box(xs, xs + w, y - 0.12, y + 0.12, z - h / 4 + 0.01, z - h / 4 + 0.01 + hh,
                mat('book', rnd.choice([(0.5, 0.1, 0.08), (0.1, 0.2, 0.4), (0.7, 0.6, 0.4), (0.15, 0.3, 0.15), (0.8, 0.8, 0.75)]), 0.7))
            xs += w + 0.005


def sofa(x, y, w, rnd, back=1):
    c = mat('fabric', rnd.choice([(0.35, 0.33, 0.3), (0.15, 0.2, 0.3), (0.45, 0.25, 0.15), (0.2, 0.3, 0.22)]), 0.9)
    box(x - w / 2, x + w / 2, y - 0.45, y + 0.45, 0.1, 0.45, c)
    box(x - w / 2, x + w / 2, y + 0.25 * back - 0.2, y + 0.25 * back + 0.2, 0.45, 0.85, c)
    for sx in (-1, 1):
        box(x + sx * w / 2 - 0.12, x + sx * w / 2 + 0.12, y - 0.45, y + 0.45, 0.1, 0.65, c)


def lamp(x, y, h=1.6, col=(1.0, 0.75, 0.45)):
    cyl(x, y, 0, h, 0.015, mat('metal', (0.6, 0.6, 0.6), 0.3, 1.0))
    cyl(x, y, h, h + 0.3, 0.2, mat('lampshade', col, emit=5, emit_col=col))
    add_light(x, y, h + 0.1, col, 40)


def tv(x, y, w=1.4, col=(0.35, 0.5, 0.9)):
    box(x - w / 2, x + w / 2, y - 0.03, y + 0.03, 1.0, 1.0 + w * 0.56, mat('bezel', (0.03, 0.03, 0.035), 0.3))
    box(x - w / 2 + 0.03, x + w / 2 - 0.03, y - 0.04, y - 0.035, 1.03, 0.97 + w * 0.56, mat('screen', col, emit=3, emit_col=col))
    box(x - w * 0.6, x + w * 0.6, y - 0.25, y + 0.15, 0, 0.5, mat('cabinet', (0.12, 0.1, 0.09), 0.5))


def art(x, y, w, h, z, rnd):
    box(x - w / 2, x + w / 2, y - 0.03, y, z, z + h, mat('frame', (0.05, 0.05, 0.05), 0.4))
    box(x - w / 2 + 0.05, x + w / 2 - 0.05, y - 0.035, y - 0.03, z + 0.05, z + h - 0.05,
        mat('canvas', rnd.choice([(0.7, 0.3, 0.2), (0.2, 0.4, 0.6), (0.8, 0.7, 0.3), (0.3, 0.5, 0.35)]), 0.8))


# ---------------------------------------------------------------- the rooms
def room(fam, kind, W, H, D, rnd):
    warm = fam in ('residential', 'hotel', 'pencil')
    wall = mat('wall', rnd.choice([(0.72, 0.7, 0.66), (0.62, 0.64, 0.66), (0.78, 0.74, 0.66), (0.55, 0.58, 0.6)]), 0.85)
    floor_m = mat('floor', rnd.choice([(0.25, 0.25, 0.27), (0.4, 0.3, 0.2), (0.32, 0.3, 0.28), (0.5, 0.42, 0.32)]), 0.5)
    ceil = mat('ceil', (0.8, 0.8, 0.8), 0.9)
    if kind == 'fit_out':
        wall = mat('concrete', (0.45, 0.44, 0.42), 0.95)
        floor_m = mat('slab', (0.38, 0.37, 0.35), 0.9)
        ceil = mat('concrete', (0.45, 0.44, 0.42), 0.95)
    shell(W, H, D, wall, floor_m, ceil)
    if fam == 'office':
        if kind in ('open_plan', 'open_plan_b'):
            ceiling_lights(W, H, D)
            rows = (2.2, 4.6, 7.0)
            for y in rows:
                for x in [1.3 + i * 1.75 for i in range(int((W - 1.3) / 1.75))]:
                    desk(x, y, rnd, facing=-1 if kind == 'open_plan' else 1)
            plant(0.5, D - 0.6)
            plant(W - 0.5, 1.2)
            box(W * 0.3, W * 0.7, D - 0.03, D, 1.0, 2.2, mat('whiteboard', (0.9, 0.9, 0.9), 0.3))
        elif kind == 'meeting':
            ceiling_lights(W, H, D, power=0.9)
            box(W * 0.25, W * 0.75, D * 0.35, D * 0.65, 0.72, 0.76, mat('table', (0.3, 0.22, 0.15), 0.4))
            for k in range(5):
                for yy in (D * 0.3, D * 0.7):
                    chair(W * 0.28 + k * W * 0.11, yy, rnd)
            tv(W / 2, D - 0.1, 2.4, (0.3, 0.6, 1.0))
            box(0.6, 1.2, D * 0.1, D * 0.9, 0, H, mat('glasswall', (0.6, 0.7, 0.75), 0.05, 0.0))
        elif kind == 'executive':
            ceiling_lights(W, H, D, kind='pendant', warm=True, power=0.8)
            desk(W * 0.6, D * 0.55, rnd, facing=-1)
            shelf(W * 0.15, W * 0.85, D - 0.25, 2.2, rnd)
            sofa(W * 0.2, D * 0.3, 2.0, rnd)
            lamp(W * 0.9, D * 0.3)
            plant(W * 0.05 + 0.4, D * 0.7, 1.6)
            art(W * 0.2, D - 0.0, 1.2, 0.9, 1.4, rnd)
        elif kind == 'lift_lobby':
            ceiling_lights(W, H, D, kind='pendant', power=0.9)
            stone = mat('stone', (0.55, 0.5, 0.45), 0.3)
            box(-0.2, W + 0.2, D - 0.05, D, 0, H, stone)
            for x in (W * 0.25, W * 0.5, W * 0.75):
                box(x - 0.55, x + 0.55, D - 0.08, D - 0.05, 0, 2.4, mat('liftdoor', (0.7, 0.68, 0.62), 0.25, 1.0))
                box(x - 0.08, x + 0.08, D - 0.09, D - 0.08, 2.5, 2.6, mat('indicator', (1.0, 0.5, 0.2), emit=4))
            plant(0.6, D * 0.5, 1.5)
            plant(W - 0.6, D * 0.5, 1.5)
        elif kind == 'fit_out':
            ceiling_lights(W, H, D, power=0.5)
            for x in (W * 0.33, W * 0.66):
                box(x - 0.35, x + 0.35, D * 0.5 - 0.35, D * 0.5 + 0.35, 0, H, mat('concrete', (0.45, 0.44, 0.42), 0.95))
            for k in range(6):
                bx, by = rnd.uniform(0.5, W - 0.5), rnd.uniform(1, D - 1)
                box(bx - 0.3, bx + 0.3, by - 0.25, by + 0.25, 0, rnd.uniform(0.3, 0.8), mat('crate', (0.55, 0.42, 0.25), 0.8))
        elif kind == 'break_room':
            ceiling_lights(W, H, D, kind='pendant', warm=True)
            box(0.3, W - 0.3, D - 0.65, D, 0, 0.9, mat('counter', (0.85, 0.85, 0.82), 0.3))
            box(0.3, W - 0.3, D - 0.4, D, 1.5, 2.2, mat('cabinet_w', (0.88, 0.88, 0.86), 0.4))
            box(W - 1.2, W - 0.4, D - 0.75, D, 0, 2.0, mat('fridge', (0.75, 0.76, 0.78), 0.2, 0.8))
            for x in (W * 0.25, W * 0.6):
                cyl(x, D * 0.45, 0.72, 0.75, 0.55, mat('table', (0.9, 0.9, 0.88), 0.4))
                cyl(x, D * 0.45, 0, 0.72, 0.04, mat('leg', (0.15, 0.15, 0.16), 0.4, 0.8))
                for a in range(3):
                    chair(x + math.cos(a * 2.1) * 0.8, D * 0.45 + math.sin(a * 2.1) * 0.8, rnd)
        else:  # archive
            ceiling_lights(W, H, D, power=0.6)
            for y in (D * 0.3, D * 0.55, D * 0.8):
                shelf(0.5, W - 0.5, y, 2.3, rnd)
    elif fam in ('residential', 'pencil'):
        ceiling_lights(W, H, D, kind='pendant', warm=True, power=0.55)
        rug = mat('rug', rnd.choice([(0.5, 0.2, 0.15), (0.2, 0.25, 0.4), (0.6, 0.55, 0.45)]), 0.95)
        if kind in ('living', 'living_b'):
            box(W * 0.2, W * 0.8, D * 0.25, D * 0.75, 0, 0.01, rug)
            sofa(W * 0.5, D * 0.75 if kind == 'living' else D * 0.35, 2.4, rnd, back=1 if kind == 'living' else -1)
            box(W * 0.4, W * 0.6, D * 0.45, D * 0.6, 0, 0.4, mat('coffee', (0.35, 0.25, 0.18), 0.5))
            tv(W * 0.5, D - 0.05, 1.5) if kind == 'living_b' else shelf(W * 0.1, W * 0.35, D - 0.25, 2.0, rnd)
            lamp(W * 0.88, D * 0.75)
            plant(W * 0.08 + 0.3, D * 0.4, 1.4)
            art(W * 0.6, D, 1.0, 0.7, 1.3, rnd)
        elif kind in ('bedroom', 'bedroom_b'):
            sheet = mat('sheet', rnd.choice([(0.85, 0.85, 0.82), (0.5, 0.55, 0.65), (0.7, 0.6, 0.5)]), 0.9)
            box(W * 0.3, W * 0.7, D * 0.4, D - 0.1, 0, 0.55, sheet)
            box(W * 0.3, W * 0.7, D - 0.2, D - 0.05, 0, 1.1, mat('headboard', (0.3, 0.22, 0.15), 0.6))
            for x in (W * 0.22, W * 0.78):
                box(x - 0.25, x + 0.25, D - 0.5, D - 0.05, 0, 0.5, mat('night', (0.3, 0.22, 0.15), 0.6))
                lamp(x, D - 0.28, 0.5, (1.0, 0.7, 0.4))
            box(0.1, 1.0, D * 0.2, D * 0.6, 0, 2.2, mat('wardrobe', (0.8, 0.78, 0.74), 0.5))
        elif kind in ('kitchen', 'dining'):
            box(0.2, W - 0.2, D - 0.65, D, 0, 0.9, mat('counter', (0.2, 0.2, 0.22), 0.3))
            box(0.2, W - 0.2, D - 0.4, D, 1.5, 2.2, mat('cabinet_w', (0.85, 0.82, 0.76), 0.4))
            box(W - 1.0, W - 0.2, D - 0.75, D, 0, 2.0, mat('fridge', (0.75, 0.76, 0.78), 0.2, 0.8))
            box(W * 0.3, W * 0.7, D * 0.35, D * 0.6, 0.74, 0.78, mat('table', (0.45, 0.32, 0.2), 0.5))
            for k in range(3):
                for yy in (D * 0.3, D * 0.65):
                    chair(W * 0.35 + k * W * 0.15, yy, rnd)
            if kind == 'dining':
                plant(W * 0.1, D * 0.3, 1.6)
        elif kind == 'home_office':
            desk(W * 0.5, D * 0.6, rnd, facing=1)
            shelf(0.2, W * 0.4, D - 0.25, 2.1, rnd)
            shelf(W * 0.6, W - 0.2, D - 0.25, 2.1, rnd)
            lamp(W * 0.85, D * 0.35)
            plant(0.4, D * 0.3, 1.0)
        else:  # kids
            box(0.2, 1.2, D * 0.3, D - 0.1, 0, 0.5, mat('sheet', (0.4, 0.6, 0.85), 0.9))
            box(0.2, 1.2, D * 0.3, D - 0.1, 1.4, 1.6, mat('sheet', (0.9, 0.5, 0.5), 0.9))
            for k in range(14):
                bx, by = rnd.uniform(1.6, W - 0.4), rnd.uniform(0.6, D - 0.6)
                s = rnd.uniform(0.12, 0.3)
                box(bx, bx + s, by, by + s, 0, s, mat('toy', rnd.choice([(0.9, 0.2, 0.2), (0.2, 0.5, 0.9), (0.95, 0.8, 0.1), (0.2, 0.7, 0.3)]), 0.6))
            box(W * 0.3, W - 0.3, D - 0.05, D, 1.0, 1.9, mat('poster', (0.2, 0.5, 0.8), 0.8))
    elif fam == 'hotel':
        ceiling_lights(W, H, D, kind='pendant', warm=True, power=0.35)
        wood = mat('wood', rnd.choice([(0.32, 0.2, 0.12), (0.4, 0.28, 0.17), (0.22, 0.15, 0.1)]), 0.5)
        sheet = mat('sheet', rnd.choice([(0.9, 0.9, 0.87), (0.85, 0.82, 0.76), (0.9, 0.88, 0.84)]), 0.9)
        throw = mat('throw', rnd.choice([(0.5, 0.15, 0.12), (0.2, 0.25, 0.35), (0.55, 0.45, 0.3)]), 0.9)
        box(0.2, W - 0.2, 0.8, D - 0.3, 0, 0.01, mat('carpet', rnd.choice([(0.3, 0.22, 0.2), (0.25, 0.25, 0.3), (0.4, 0.33, 0.25)]), 0.95))
        box(0.3, W - 0.3, D - 0.06, D, 0.9, 2.3, wood)                               # headboard wall panel
        if kind in ('king', 'king_b', 'king_desk', 'corner_king'):
            bx = W / 2 if kind != 'corner_king' else W * 0.45
            box(bx - 0.95, bx + 0.95, D - 2.15, D - 0.1, 0, 0.6, sheet)
            box(bx - 0.95, bx + 0.95, D - 1.2, D - 0.95, 0.6, 0.63, throw)
            for x in (bx - 1.25, bx + 1.25):
                if 0.2 < x < W - 0.2:
                    box(x - 0.22, x + 0.22, D - 0.5, D - 0.06, 0, 0.5, wood)
                    lamp(x, D - 0.28, 0.5, (1.0, 0.7, 0.4))
        elif kind in ('twin', 'twin_b'):
            for x in (W * 0.27, W * 0.73):
                box(x - 0.48, x + 0.48, D - 2.1, D - 0.1, 0, 0.55, sheet)
                box(x - 0.48, x + 0.48, D - 1.1, D - 0.9, 0.55, 0.58, throw)
            box(W / 2 - 0.2, W / 2 + 0.2, D - 0.45, D - 0.06, 0, 0.5, wood)
            lamp(W / 2, D - 0.26, 0.5, (1.0, 0.7, 0.4))
        else:  # suite: the sitting room
            sofa(W * 0.5, D * 0.72, 2.0, rnd)
            box(W * 0.35, W * 0.65, D * 0.45, D * 0.58, 0, 0.4, wood)
            lamp(W - 0.35, D * 0.75)
            plant(0.35, D * 0.4, 1.3)
            tv(W * 0.5, D - 0.08, 1.3, (0.35, 0.5, 0.9)) if kind == 'suite_b' else art(W * 0.5, D, 1.2, 0.8, 1.4, rnd)
        if kind in ('king', 'twin', 'king_desk', 'twin_b'):                        # armchair by the window
            box(W - 1.1, W - 0.35, 1.0, 1.75, 0, 0.42, throw)
            box(W - 1.1, W - 0.35, 1.65, 1.8, 0.42, 0.85, throw)
            lamp(W - 0.3, 2.2, 1.5)
        if kind in ('king_desk', 'king_b', 'twin_b'):                             # desk on the side wall
            box(0.2, 0.75, 2.2, 3.6, 0.74, 0.78, wood)
            box(0.2, 0.25, 2.4, 3.4, 1.1, 1.7, mat('mirror', (0.7, 0.72, 0.75), 0.05, 1.0))
            chair(0.95, 2.9, rnd)
    else:  # transport
        ceiling_lights(W, H, D, power=1.1)
        steel = mat('steel', (0.35, 0.37, 0.4), 0.4, 0.9)
        if kind in ('control', 'control_b'):
            for row, y in enumerate((D * 0.45, D * 0.7)):
                box(0.8, W - 0.8, y - 0.4, y + 0.4, 0, 0.8, mat('console', (0.12, 0.13, 0.15), 0.4))
                for x in [1.2 + i * 1.3 for i in range(int((W - 2.4) / 1.3))]:
                    sc = rnd.choice([(0.2, 0.8, 0.6), (0.3, 0.5, 1.0), (0.9, 0.6, 0.2)])
                    box(x - 0.45, x + 0.45, y + 0.25, y + 0.3, 0.85, 1.4, mat('screen', sc, emit=3, emit_col=sc))
                    chair(x, y - 0.8, rnd)
            box(0.5, W - 0.5, D - 0.05, D, 1.2, 3.6, mat('wallscreen', (0.15, 0.4, 0.9), emit=1.8, emit_col=(0.15, 0.4, 0.9)))
        elif kind in ('waiting', 'waiting_b'):
            for y in (D * 0.3, D * 0.5, D * 0.7):
                box(1.0, W - 1.0, y - 0.25, y + 0.25, 0.4, 0.46, steel)
                box(1.0, W - 1.0, y + 0.2, y + 0.25, 0.46, 0.85, steel)
            box(W * 0.3, W * 0.7, D - 0.05, D, 2.2, 3.2, mat('board', (1.0, 0.7, 0.2), emit=2.5))
            plant(0.6, D * 0.85, 1.6)
        elif kind == 'workshop':
            for k in range(4):
                x = 1.5 + k * (W - 3) / 3
                box(x - 0.7, x + 0.7, D * 0.5 - 0.5, D * 0.5 + 0.5, 0, 1.2, mat('machine', (0.25, 0.35, 0.3), 0.5, 0.6))
                box(x - 0.1, x + 0.1, D * 0.5 - 0.55, D * 0.5 - 0.5, 0.9, 1.0, mat('led', (0.2, 1.0, 0.3), emit=6))
            shelf(0.4, W - 0.4, D - 0.3, 2.5, rnd)
        elif kind == 'stair_hall':
            for k in range(14):
                z = k * H / 14
                box(1.0, 3.2, D * 0.3 + k * 0.3, D * 0.3 + k * 0.3 + 0.3, 0, z + 0.3, mat('concrete', (0.5, 0.49, 0.47), 0.9))
            box(4.0, W - 1.0, D - 0.05, D, 0, 2.6, mat('liftdoor', (0.7, 0.68, 0.62), 0.25, 1.0))
            box(W - 2.0, W - 1.4, D - 0.08, D - 0.05, 2.7, 2.95, mat('exit', (0.1, 1.0, 0.3), emit=6))
        else:  # staff_office, lockers
            if kind == 'lockers':
                for x in [0.6 + i * 0.5 for i in range(int((W - 1.2) / 0.5))]:
                    box(x, x + 0.48, D - 0.6, D, 0, 1.9, mat('locker', rnd.choice([(0.2, 0.35, 0.5), (0.3, 0.3, 0.32)]), 0.4, 0.7))
                box(W * 0.3, W * 0.7, D * 0.45, D * 0.55, 0.4, 0.45, steel)
            else:
                for x in (2.0, 4.5, 7.0, 9.5):
                    desk(x, D * 0.5, rnd, facing=-1)
                shelf(0.4, W - 0.4, D - 0.25, 2.2, rnd)


def layers(D):
    """Box layer = the shell, flush ceiling panels and whatever stands against the back wall (shelves, TV, counters,
    lift doors): drawn on the room's surfaces. Card layer = everything standing in the room (desks, chairs, sofas,
    beds, hanging lamps): drawn on a plane at depth ZF, so it moves in front of the back wall."""
    box_l, card = [], []
    for o in bpy.context.scene.collection.objects:
        if o.type != 'MESH':
            continue
        if o.get('layer') == 'box':
            box_l.append(o)
            continue
        ymax = max((o.matrix_world @ Vector(c)).y for c in o.bound_box)
        (box_l if ymax > D - 0.9 else card).append(o)
    return box_l, card


def render(fam, kind, idx, W, H, D):
    s = bpy.context.scene
    h = round(CELL_W * H / W)
    s.render.resolution_x, s.render.resolution_y = CELL_W, h
    s.render.image_settings.color_mode = 'RGBA'
    F = D   # s = F / (F + D) = 0.5
    cam = bpy.data.objects.new('cam', bpy.data.cameras.new('cam'))
    s.collection.objects.link(cam)
    cam.location = (W / 2, -F, H / 2)
    cam.rotation_euler = (math.radians(90), 0, 0)   # looking +Y
    cam.data.sensor_fit = 'HORIZONTAL'
    cam.data.sensor_width = 36
    cam.data.lens = 36 * F / W                        # the opening W fills the frame at distance F
    cam.data.clip_start = 0.05
    s.camera = cam
    box_l, card = layers(D)

    def shot(name, layer, samples):
        # the other layer still lights and shadows this one; it is just not seen by the camera
        s.render.film_transparent = layer == 'card'
        for o in box_l:
            o.visible_camera = True
            o.is_holdout = layer == 'card'
        for o in card:
            o.visible_camera = layer == 'card'
        s.cycles.samples = samples
        s.render.filepath = os.path.join(OUT, f'_{fam}_{idx}_{name}.png')
        bpy.ops.render.render(write_still=True)

    shot('lit', 'box', 64)
    shot('furn', 'card', 64)
    # what still glows with the lights off: emissive surfaces only (screens, lamps, LEDs), no lamps' light
    for o in list(s.collection.objects):
        if o.type == 'LIGHT':
            o.hide_render = True
    for m in bpy.data.materials:
        if not m.get('glow') or m.get('ceiling'):
            b = m.node_tree.nodes['Principled BSDF']
            b.inputs['Base Color'].default_value = (0, 0, 0, 1)
            b.inputs['Roughness'].default_value = 1
            b.inputs['Metallic'].default_value = 0
            b.inputs['Specular IOR Level'].default_value = 0
            b.inputs['Emission Strength'].default_value = 0
    shot('glow', 'box', 8)
    shot('furn_glow', 'card', 8)
    return h


def build(fam):
    F = FAM[fam]
    W, H, D = F['W'], F['H'], F['D']
    cell_h = None
    for idx, kind in enumerate(F['rooms']):
        reset()
        MATS.clear()
        rnd = random.Random(f'{fam}-{kind}-{idx}')
        room(fam, kind, W, H, D, rnd)
        cell_h = render(fam, kind, idx, W, H, D)
    # assemble the 2 x 4 atlases (cell 0 top-left, row-major; the shader uses the same order)
    import numpy as np
    def load(path):
        img = bpy.data.images.load(path)
        a = np.empty(img.size[0] * img.size[1] * 4, dtype=np.float32)
        img.pixels.foreach_get(a)
        a = a.reshape(img.size[1], img.size[0], 4)[::-1]   # Blender rows are bottom-up
        bpy.data.images.remove(img)
        return a
    KINDS = (('lit', f'{fam}_rooms', False), ('glow', f'{fam}_rooms_glow', False),
             ('furn', f'{fam}_furn', True), ('furn_glow', f'{fam}_furn_glow', False))
    for key, name, alpha in KINDS:
        arr = np.zeros((ROWS * cell_h, COLS * CELL_W, 4), np.float32)
        for idx in range(len(F['rooms'])):
            r, c = divmod(idx, COLS)
            arr[r * cell_h:(r + 1) * cell_h, c * CELL_W:(c + 1) * CELL_W] = load(os.path.join(OUT, f'_{fam}_{idx}_{key}.png'))
        img = bpy.data.images.new(name, COLS * CELL_W, ROWS * cell_h, alpha=alpha)
        if not alpha:
            arr[..., 3] = 1
        img.pixels.foreach_set(arr[::-1].ravel())
        img.filepath_raw = os.path.join(OUT, f'{name}.png')
        img.file_format = 'PNG'
        img.save()
    for idx in range(len(F['rooms'])):
        for k in ('lit', 'glow', 'furn', 'furn_glow'):
            os.remove(os.path.join(OUT, f'_{fam}_{idx}_{k}.png'))
    return dict(family=fam, W=W, H=H, D=D, ZF=F['ZF'], s=0.5, cols=COLS, rows=ROWS, cell_px=[CELL_W, cell_h], rooms=F['rooms'])


res = [build(f) for f in (WANT or FAM)]
with open(os.path.join(OUT, 'rooms.json'), 'w') as fh:
    json.dump(res, fh, indent=1)
print('ROOMS', json.dumps([(r['family'], r['cell_px']) for r in res]))
