"""Meaningful offline geometry/data regressions, including deliberately bad inputs.

Run `python3 Blender/test_layout.py --layout-dir Layout` from an extracted pack.
No Blender or host-game execution is performed by these tests.
"""
import argparse
import copy
import math
from pathlib import Path
import sys
import unittest

from route_tools import (ClosedRoute, game_to_blender, blender_to_game, distance, dot,
                         sub, cross, length, unit, read_json, validate_layout,
                         projected_intersections, local_to_game, game_to_local)
from mesh_geometry import deck_mesh, ring_mesh, start_platform_mesh, box_mesh, frustum_mesh


def layouts():
    parser=argparse.ArgumentParser(add_help=False)
    parser.add_argument("--layout-dir",type=Path)
    args,rest=parser.parse_known_args()
    sys.argv=[sys.argv[0]]+rest
    if args.layout_dir:
        dirs=[args.layout_dir]
    else:
        root=Path(__file__).resolve().parents[1]
        dirs=[d for d in (root/"R1",root/"R3",root/"Layout") if (d/"route.json").exists()]
    if not dirs:
        raise SystemExit("No Layout/route.json found; pass --layout-dir explicitly")
    return [(read_json(d/"route.json"),read_json(d/"placements.json")) for d in dirs]


DATA=layouts()


class LayoutTests(unittest.TestCase):
    def test_authored_routes_and_clearances_pass(self):
        for config,placements in DATA:
            with self.subTest(map_id=config["map_id"]):
                report=validate_layout(config,placements)
                self.assertTrue(report["ok"],report["errors"])
                self.assertGreater(report["placeholder_collision_parts_checked"],10)
                self.assertEqual(report["projected_self_intersections"],[])

    def test_coordinate_roundtrip_handedness_and_yaw(self):
        self.assertEqual(game_to_blender([3,7,11]),[3,-11,7])
        for point in ([0,0,0],[1,2,3],[-17.2,108.7,-450],[1e-4,-1e4,100]):
            self.assertLess(distance(point,blender_to_game(game_to_blender(point))),1e-10)
            for yaw in (0,37,90,179,-141):
                self.assertLess(distance(point,game_to_local(local_to_game(point,[11,-9,3],yaw),[11,-9,3],yaw)),1e-10)
        gx,gy,gz=[1,0,0],[0,1,0],[0,0,1]
        self.assertEqual(cross(game_to_blender(gx),game_to_blender(gy)),game_to_blender(gz))

    def test_spline_closure_and_tangent_continuity(self):
        for config,_ in DATA:
            route=ClosedRoute(config)
            for i in range(route.count):
                a,av,_=route.evaluate_span(i,1)
                b,bv,_=route.evaluate_span((i+1)%route.count,0)
                self.assertLess(distance(a,b),1e-8)
                self.assertLess(distance(av,bv),1e-8)
            lengths=[distance(a,b) for a,b in zip(route.points,route.points[1:])]
            self.assertGreater(min(lengths),.1)
            self.assertLess(max(lengths),2)

    def test_gates_face_forward_and_resets_precede_gate(self):
        for config,_ in DATA:
            route=ClosedRoute(config)
            for cp in config["checkpoints"]:
                point,normal=cp["position_game"],cp["forward_game"]
                before=route.at_distance(cp["distance_m"]-1)[0]
                after=route.at_distance(cp["distance_m"]+1)[0]
                self.assertLess(dot(sub(before,point),normal),-.99)
                self.assertGreater(dot(sub(after,point),normal),.99)
                self.assertLess(dot(sub(cp["respawn"]["position_game"],point),normal),-17)
                self.assertEqual(cp["inner_diameter_m"],22)
                self.assertEqual(cp["ring_outer_diameter_m"],26)

    def test_validator_rejects_reversed_gate_and_stale_length(self):
        config,placements=copy.deepcopy(DATA[0])
        config["checkpoints"][2]["forward_game"]=[-v for v in config["checkpoints"][2]["forward_game"]]
        config["computed"]["length_m"]+=30
        report=validate_layout(config,placements)
        self.assertFalse(report["ok"])
        self.assertTrue(any("normal must point forward" in s for s in report["errors"]))
        self.assertTrue(any("length is stale" in s for s in report["errors"]))

    def test_validator_rejects_bad_spacing_and_excessive_grade(self):
        config,_=copy.deepcopy(DATA[0])
        config["checkpoints"][1]["distance_m"]=20
        config["control_points"][5]["position_game"][1]+=500
        config["computed"]=ClosedRoute(config).stats()
        report=validate_layout(config)
        self.assertFalse(report["ok"])
        self.assertTrue(any("spacing" in s for s in report["errors"]))
        self.assertTrue(any("grade" in s for s in report["errors"]))

    def test_crossing_detector_catches_figure_eight_and_touch(self):
        bowtie=[[0,0,0],[100,0,100],[0,0,100],[100,0,0]]
        self.assertTrue(projected_intersections(bowtie))
        touching=[[0,0,0],[100,0,100],[0,0,200],[-100,0,100],[0,0,0],[100,0,-100],[0,0,-200],[-100,0,-100]]
        self.assertTrue(projected_intersections(touching))
        config,_=copy.deepcopy(DATA[0])
        for i,p in enumerate(config["control_points"]):
            t=math.tau*i/24
            p["position_game"]=[300*math.sin(t),90,230*math.sin(2*t)]
        route=ClosedRoute(config)
        self.assertTrue(projected_intersections(route.samples(8)))

    def test_hollow_landmark_cannot_be_replaced_by_solid_bbox(self):
        for config,placements in DATA:
            changed=copy.deepcopy(placements)
            target=next(p for p in changed["instances"] if p["placeholder_kind"] in ("skybridge","rock_arch"))
            w,h,d=target["size_xyz_m"]
            target["collision_parts"]=[dict(id="WRONG_FILLED_ENVELOPE",center_local=[0,h/2,0],size_xyz_m=[w,h,d])]
            report=validate_layout(config,changed)
            self.assertFalse(report["ok"])
            self.assertTrue(any("WRONG_FILLED_ENVELOPE" in s for s in report["placeholder_clearance_collisions"]))

    def test_blocking_landmark_is_rejected(self):
        config,placements=copy.deepcopy(DATA[0])
        position=ClosedRoute(config).at_distance(300)[0]
        item=placements["instances"][0]
        item["position_game"]=[position[0],position[1]-10,position[2]]
        item["yaw_degrees"]=0
        item["collision_parts"]=[dict(id="block",center_local=[0,10,0],size_xyz_m=[30,20,30])]
        report=validate_layout(config,placements)
        self.assertFalse(report["ok"])
        self.assertIn(item["id"]+"/block",report["placeholder_clearance_collisions"])

    def test_closed_deck_mesh_manifold_uvs_and_outward_top(self):
        for config,_ in DATA:
            route=ClosedRoute(config)
            data=deck_mesh(route)
            self.assert_closed_mesh(data)
            vertices,faces,uvs,_=data
            for i,face in enumerate(faces):
                normal=cross(sub(vertices[face[1]],vertices[face[0]]),sub(vertices[face[2]],vertices[face[0]]))
                if i%4==0: self.assertGreater(normal[1],0)
                if i%4==1: self.assertLess(normal[1],0)
                self.assertEqual(len(uvs[i]),len(face))
            # Last span is continuous in distance UV, not a giant jump to zero.
            last_top_uv=uvs[-4]
            self.assertGreater(last_top_uv[1][1],last_top_uv[0][1])

    def test_ring_platform_and_primitives_have_closed_oriented_meshes(self):
        config,_=DATA[0]
        route=ClosedRoute(config)
        ring=ring_mesh([0,0,0],[0,0,1])
        self.assert_closed_mesh(ring)
        vertices,faces,_,_=ring
        n=cross(sub(vertices[faces[0][1]],vertices[faces[0][0]]),sub(vertices[faces[0][2]],vertices[faces[0][0]]))
        self.assertGreater(n[2],0)
        self.assert_closed_mesh(start_platform_mesh(route,0))
        self.assert_closed_mesh(box_mesh([0,0,0],[4,7,11]))
        self.assert_closed_mesh(frustum_mesh([0,0,0],20,5,2))

    def assert_closed_mesh(self,data):
        vertices,faces,uvs,materials=data
        self.assertEqual(len(faces),len(uvs))
        self.assertEqual(len(faces),len(materials))
        edges={}
        for face,face_uv in zip(faces,uvs):
            self.assertEqual(len(face),len(face_uv))
            self.assertEqual(len(set(face)),len(face))
            normal=cross(sub(vertices[face[1]],vertices[face[0]]),sub(vertices[face[2]],vertices[face[0]]))
            self.assertGreater(length(normal),1e-8)
            for a,b in zip(face,face[1:]+face[:1]):
                key=tuple(sorted((a,b)))
                edges.setdefault(key,[]).append((a,b))
        for edge,uses in edges.items():
            self.assertEqual(len(uses),2,f"Non-manifold edge {edge}")
            self.assertEqual(uses[0],tuple(reversed(uses[1])),f"Inconsistent winding at {edge}")


if __name__=="__main__": unittest.main(verbosity=2)
