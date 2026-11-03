"""Standard-library route geometry; canonical coordinates are X right, Y up, Z forward."""
from __future__ import annotations

import bisect
import json
import math
from pathlib import Path


def add(a, b): return [a[i] + b[i] for i in range(3)]
def sub(a, b): return [a[i] - b[i] for i in range(3)]
def mul(a, s): return [x * s for x in a]
def dot(a, b): return sum(x * y for x, y in zip(a, b))
def cross(a, b): return [a[1]*b[2]-a[2]*b[1], a[2]*b[0]-a[0]*b[2], a[0]*b[1]-a[1]*b[0]]
def length(a): return math.sqrt(dot(a, a))
def unit(a):
    size = length(a)
    if size < 1e-12:
        raise ValueError("A zero vector cannot define a direction")
    return mul(a, 1.0 / size)
def lerp(a, b, t): return add(mul(a, 1-t), mul(b, t))
def distance(a, b): return length(sub(a, b))
def round_vec(a, digits=6): return [round(x, digits) for x in a]


def game_to_blender(v): return [v[0], -v[2], v[1]]
def blender_to_game(v): return [v[0], v[2], -v[1]]


def frame(tangent):
    """A right-handed frame. No banking: right is horizontal, up follows the slope."""
    forward = unit(tangent)
    right = unit(cross([0, 1, 0], forward))
    return right, unit(cross(forward, right)), forward


def read_json(path):
    return json.loads(Path(path).read_text(encoding="utf-8"))


class ClosedRoute:
    """Closed uniform Catmull–Rom spline with an explicitly sampled arc-length table.

    Arc lengths are chord sums at samples_per_span samples per spline span. They
    are an approximation, not a symbolic spline integral. The emitted JSON stores
    this resolution, and all consumers here use the same table.
    """
    def __init__(self, config):
        self.config = config
        self.controls = [p["position_game"] for p in config["control_points"]]
        self.count = len(self.controls)
        if self.count < 4:
            raise ValueError("A closed Catmull–Rom route needs at least four controls")
        self.resolution = int(config.get("interpolation", {}).get("samples_per_span", 128))
        if self.resolution < 8:
            raise ValueError("samples_per_span must be at least 8")
        self.parameters = [i / self.resolution for i in range(self.count * self.resolution + 1)]
        self.points = [self.evaluate(t)[0] for t in self.parameters]
        self.distances = [0.0]
        for a, b in zip(self.points, self.points[1:]):
            self.distances.append(self.distances[-1] + distance(a, b))
        self.total_length = self.distances[-1]
        if self.total_length < 1:
            raise ValueError("Route is degenerate")

    def evaluate_span(self, index, t):
        p0, p1, p2, p3 = [self.controls[i % self.count] for i in (index-1, index, index+1, index+2)]
        pos, vel, acc = [], [], []
        for axis in range(3):
            a = 2*p1[axis]
            b = -p0[axis] + p2[axis]
            c = 2*p0[axis] - 5*p1[axis] + 4*p2[axis] - p3[axis]
            d = -p0[axis] + 3*p1[axis] - 3*p2[axis] + p3[axis]
            pos.append(0.5 * (a + b*t + c*t*t + d*t*t*t))
            vel.append(0.5 * (b + 2*c*t + 3*d*t*t))
            acc.append(c + 3*d*t)
        return pos, vel, acc

    def evaluate(self, parameter):
        parameter %= self.count
        index = int(math.floor(parameter))
        return self.evaluate_span(index, parameter-index)

    def at_distance(self, meters):
        meters %= self.total_length
        i = min(len(self.distances)-2, bisect.bisect_right(self.distances, meters)-1)
        span = self.distances[i+1] - self.distances[i]
        alpha = 0.0 if span <= 1e-12 else (meters-self.distances[i])/span
        parameter = (i + alpha) / self.resolution
        pos, vel, _ = self.evaluate(parameter)
        return pos, unit(vel)

    def samples(self, spacing=8.0):
        count = max(8, math.ceil(self.total_length / spacing))
        return [dict(distance_m=i*self.total_length/count,
                     position_game=self.at_distance(i*self.total_length/count)[0],
                     forward_game=self.at_distance(i*self.total_length/count)[1])
                for i in range(count)]

    def stats(self):
        slopes, radii = [], []
        for parameter in self.parameters[:-1]:
            _, vel, acc = self.evaluate(parameter)
            horizontal_speed = math.hypot(vel[0], vel[2])
            slopes.append(math.degrees(math.atan2(abs(vel[1]), horizontal_speed)))
            numerator = horizontal_speed ** 3
            denominator = abs(vel[0]*acc[2] - vel[2]*acc[0])
            if denominator > 1e-12:
                radii.append(numerator / denominator)
        low = [min(p[i] for p in self.points) for i in range(3)]
        high = [max(p[i] for p in self.points) for i in range(3)]
        return dict(length_m=round(self.total_length, 3),
                    bounds_min_game=round_vec(low, 3),
                    bounds_max_game=round_vec(high, 3),
                    footprint_xz_m=[round(high[0]-low[0], 3), round(high[2]-low[2], 3)],
                    elevation_range_m=[round(low[1], 3), round(high[1], 3)],
                    maximum_grade_degrees=round(max(slopes), 3),
                    minimum_horizontal_turn_radius_m=round(min(radii), 3),
                    arc_table_segments=self.count*self.resolution)


def yaw_for_forward(forward):
    return math.degrees(math.atan2(forward[0], forward[2]))


def local_to_game(local, origin, yaw_degrees):
    angle = math.radians(yaw_degrees)
    c, s = math.cos(angle), math.sin(angle)
    return [origin[0] + c*local[0] + s*local[2],
            origin[1] + local[1],
            origin[2] - s*local[0] + c*local[2]]


def game_to_local(point, origin, yaw_degrees):
    d = sub(point, origin)
    angle = math.radians(yaw_degrees)
    c, s = math.cos(angle), math.sin(angle)
    return [c*d[0] - s*d[2], d[1], s*d[0] + c*d[2]]


def segment_intersects_aabb(a, b, low, high):
    """Slab test; used on fine spline chords against expanded collision boxes."""
    enter, leave = 0.0, 1.0
    for axis in range(3):
        direction = b[axis] - a[axis]
        if abs(direction) < 1e-12:
            if a[axis] < low[axis] or a[axis] > high[axis]:
                return False
            continue
        t0, t1 = (low[axis]-a[axis])/direction, (high[axis]-a[axis])/direction
        if t0 > t1:
            t0, t1 = t1, t0
        enter, leave = max(enter, t0), min(leave, t1)
        if enter > leave:
            return False
    return True


def projected_intersections(samples):
    """Report non-adjacent crossings/touches of the sampled route in the XZ plane."""
    points = [s["position_game"] if isinstance(s, dict) else s for s in samples]
    count = len(points)
    found = []
    def orientation(a, b, c):
        return (b[0]-a[0])*(c[2]-a[2]) - (b[2]-a[2])*(c[0]-a[0])
    def on_segment(a,b,c):
        return min(a[0],b[0])-1e-8 <= c[0] <= max(a[0],b[0])+1e-8 and min(a[2],b[2])-1e-8 <= c[2] <= max(a[2],b[2])+1e-8
    for i in range(count):
        a, b = points[i], points[(i+1) % count]
        for j in range(i+2, count):
            if i == 0 and j == count-1:
                continue
            c, d = points[j], points[(j+1) % count]
            if max(a[0], b[0]) < min(c[0], d[0]) or max(c[0], d[0]) < min(a[0], b[0]):
                continue
            if max(a[2], b[2]) < min(c[2], d[2]) or max(c[2], d[2]) < min(a[2], b[2]):
                continue
            o1,o2,o3,o4=orientation(a,b,c),orientation(a,b,d),orientation(c,d,a),orientation(c,d,b)
            if (o1*o2 < -1e-9 and o3*o4 < -1e-9) or any(abs(o)<1e-8 and on_segment(p,q,r) for o,p,q,r in ((o1,a,b,c),(o2,a,b,d),(o3,c,d,a),(o4,c,d,b))):
                found.append([i, j])
    return found


def validate_layout(config, placements=None):
    """Return data-math checks only; this does not validate a game or Blender runtime."""
    errors = []
    def require(condition, message):
        if not condition:
            errors.append(message)
    require(config.get("closed") is True, "Route must be explicitly closed")
    require(config.get("coordinate_system") == "game_x_right_y_up_z_forward_meters", "Unexpected coordinate system")
    controls = config.get("control_points", [])
    require(16 <= len(controls) <= 24, "Expected 16 to 24 controls")
    for control in controls:
        p = control.get("position_game", [])
        require(len(p) == 3 and all(isinstance(v, (int, float)) and math.isfinite(v) for v in p), "Invalid control position")
    if errors:
        return dict(ok=False, errors=errors, scope="Authored source layout only")
    route = ClosedRoute(config)
    stats = route.stats()
    first_pos, first_vel, _ = route.evaluate_span(0, 0)
    last_pos, last_vel, _ = route.evaluate_span(route.count-1, 1)
    require(distance(first_pos, last_pos) < 1e-9, "Spline closing positions disagree")
    require(distance(first_vel, last_vel) < 1e-9, "Spline closing tangents disagree")
    require(distance(controls[0]["position_game"], controls[-1]["position_game"]) > 1, "Do not duplicate first control at end")
    require(config["route_width_m"] == 18, "Deck/route width must be 18 m")
    require(config["deck_below_route_m"] == 3, "Deck top must be 3 m below flight center")
    require(2000 <= route.total_length <= 4000, "Course length outside 2–4 km target")
    require(abs(config["computed"]["length_m"]-route.total_length) < .01, "Stored route length is stale")
    limits = config["design_limits"]
    require(stats["maximum_grade_degrees"] <= limits["maximum_grade_degrees"], "Route grade exceeds authored limit")
    require(stats["minimum_horizontal_turn_radius_m"] >= limits["minimum_horizontal_turn_radius_m"], "Turn radius too small")
    require(stats["elevation_range_m"][0] >= limits["minimum_elevation_m"], "Route below target elevation")
    require(stats["elevation_range_m"][1] <= limits["maximum_elevation_m"], "Route above target elevation")
    for dimension, target in zip(stats["footprint_xz_m"], limits["footprint_target_xz_m"]):
        require(abs(dimension-target) <= target*.12, "Footprint differs from target by more than 12%")
    samples = route.samples(8)
    crossings = projected_intersections(samples)
    require(not crossings, "Non-adjacent route segments cross in the XZ plane")
    checkpoints = config["checkpoints"]
    require(len(checkpoints) >= 8, "Too few checkpoints")
    require(len({c["id"] for c in checkpoints}) == len(checkpoints), "Checkpoint IDs must be unique")
    spacings = []
    for i, checkpoint in enumerate(checkpoints):
        require(checkpoint["sequence"] == i, "Checkpoints must have zero-based contiguous sequence numbers")
        require(checkpoint["inner_diameter_m"] == 22, "Checkpoint inner diameter must be 22 m")
        s = checkpoint["distance_m"]
        require(0 <= s < route.total_length, "Checkpoint distance must be inside one lap")
        if i:
            require(s > checkpoints[i-1]["distance_m"], "Checkpoint distances must increase")
        pos, tangent = route.at_distance(s)
        require(distance(pos, checkpoint["position_game"]) < .002, "Checkpoint is off the route")
        require(abs(length(checkpoint["forward_game"])-1) < 1e-5, "Checkpoint normal must be normalized")
        require(dot(tangent, checkpoint["forward_game"]) > .99999, "Checkpoint normal must point forward along route")
        next_s = checkpoints[(i+1) % len(checkpoints)]["distance_m"]
        gap = (next_s-s) % route.total_length
        spacings.append(gap)
        require(100 <= gap <= 350, "Checkpoint spacing outside 100–350 m")
        reset = checkpoint["respawn"]
        rpos, rtangent = route.at_distance(reset["distance_m"])
        require(distance(rpos, reset["position_game"]) < .002, "Respawn must be sampled on the route")
        require(dot(rtangent, reset["forward_game"]) > .99999, "Respawn tangent is stale")
        require(abs((s-reset["distance_m"]) % route.total_length - reset["behind_checkpoint_m"]) < .002, "Respawn distance behind checkpoint is wrong")
    for spawn in config["spawns"]:
        pos, tangent = route.at_distance(spawn["distance_m"])
        require(distance(pos, spawn["position_game"]) < .002, "Spawn must be sampled on the route")
        require(dot(tangent, spawn["forward_game"]) > .99999, "Spawn tangent is stale")
    clearance_checks = 0
    clearance_collisions = []
    if placements:
        require(placements["map_id"] == config["map_id"], "Placement and route map IDs disagree")
        require(len({p["id"] for p in placements["instances"]}) == len(placements["instances"]), "Placement IDs must be unique")
        margin = placements["reserved_flight_corridor"]
        horizontal = margin["half_width_m"]
        vertical = margin["half_height_m"]
        # Dense chords are at most ~1.5 m here; slab tests cover every chord.
        for item in placements["instances"]:
            if item["collision_mode"] == "none":
                continue
            local_points = [game_to_local(p, item["position_game"], item.get("yaw_degrees", 0)) for p in route.points]
            for part in item["collision_parts"]:
                center, size = part["center_local"], part["size_xyz_m"]
                padding = [horizontal, vertical, horizontal]
                low = [center[k]-size[k]/2-padding[k] for k in range(3)]
                high = [center[k]+size[k]/2+padding[k] for k in range(3)]
                clearance_checks += 1
                if any(segment_intersects_aabb(a, b, low, high) for a, b in zip(local_points, local_points[1:])):
                    clearance_collisions.append(item["id"] + "/" + part["id"])
        require(not clearance_collisions, "Flight corridor overlaps placeholder collision parts: " + ", ".join(clearance_collisions))
    return dict(ok=not errors, errors=errors, scope="Authored source layout math; not Blender execution or game collision verification", computed=stats,
                control_count=len(controls), checkpoint_count=len(checkpoints),
                checkpoint_spacing_range_m=[round(min(spacings), 3), round(max(spacings), 3)],
                projected_self_intersections=crossings,
                placeholder_collision_parts_checked=clearance_checks,
                placeholder_clearance_collisions=clearance_collisions,
                coordinate_roundtrip_error=distance([13.5, -7.25, 91.0], blender_to_game(game_to_blender([13.5, -7.25, 91.0]))))
