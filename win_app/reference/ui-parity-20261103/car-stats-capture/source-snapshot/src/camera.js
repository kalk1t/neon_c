// Cameras: the chase / first-person / cockpit view with RMB free look, the menu / pause / shop showcase (drag to
// orbit, scroll to zoom) and the start-up shader warm-up view. Split from main.js (D1).
import * as THREE from 'three';
import { CAR } from './car.js';
import { camera, lens } from './render.js';
import { track } from './world.js';

const $ = (id) => document.getElementById(id);
let g, player, car, dash, D;
/** deps from main.js: g, player, car, dash (the cockpit), touch (getter: the phone controls, or null). */
export function initCamera(deps) {
  D = deps;
  ({ g, player, car, dash } = deps);
  g.steerVis = 0; // the yoke's on-screen turn, eased
  setInterior(localStorage.getItem('neon.view') === 'interior');
}

// lookYaw / lookPitch: the RMB free-look orbit around the car (eases back to 0 once released)
export const cam = { yaw: 0, pitch: 0, fov: 70, pos: new THREE.Vector3(), lookYaw: 0, lookPitch: 0, looking: false };
export const CAMS = [{ d: 9, h: 2.7 }, { d: 15, h: 4.8 }, { d: 0, h: 0 }];
let tourT = 0; // cinematicCamera's position along the track (start-up shader warm-up only)

export function setInterior(v) {
  g.interior = v;
  localStorage.setItem('neon.view', v ? 'interior' : 'chase');
  $('view-hint').innerHTML = `<kbd>I</kbd>${v ? 'EXTERIOR' : 'COCKPIT'} VIEW`;
}

export function chaseCamera(dt) {
  const sf = Math.min(1.4, Math.max(0, car.speed) / CAR.MAX);
  // mouse-aim camera (v0.2.12): with the mouse captured the view follows the aim (the car turns after it), so the aim
  // ring stays in the middle of the screen; otherwise (keys, touch, gamepad, autopilot) it follows the car
  const aimCam = g.camAim && g.state === 'race' && !g.autopilot;
  let dy = (aimCam ? g.aim.yaw : car.yaw) - cam.yaw;
  dy = Math.atan2(Math.sin(dy), Math.cos(dy));
  cam.yaw += dy * (1 - Math.exp(-(aimCam ? 14 : 7) * dt));
  cam.pitch += ((aimCam ? g.aim.pitch * 0.85 : car.pitch * 0.6) - cam.pitch) * (1 - Math.exp(-(aimCam ? 10 : 5) * dt));
  const m = CAMS[g.camMode];
  // free look: released, the orbit swings back behind the car (the shorter way round)
  if (!cam.looking) {
    cam.lookYaw = Math.atan2(Math.sin(cam.lookYaw), Math.cos(cam.lookYaw)) * Math.exp(-4 * dt);
    cam.lookPitch *= Math.exp(-4 * dt);
  }
  const orbit = Math.min(1, (Math.abs(Math.atan2(Math.sin(cam.lookYaw), Math.cos(cam.lookYaw))) + Math.abs(cam.lookPitch)) * 4);
  const vy = cam.yaw + cam.lookYaw, vp = THREE.MathUtils.clamp(cam.pitch - cam.lookPitch, -1.3, 1.3);
  const fwd = new THREE.Vector3(-Math.sin(vy) * Math.cos(vp), Math.sin(vp), -Math.cos(vy) * Math.cos(vp));
  // first person: the camera sits inside our own car, so its body is hidden (shots and thrusters still show)
  // interior (I): the same seat, a little higher and looking a little down, with the car's dashboard drawn over it
  const inside = player.alive && orbit < 0.05;
  const cockpit = (g.camMode === 2 || g.interior) && inside;
  car.model.visible = !cockpit;
  dash.show(g.interior && inside);
  if (cockpit) {
    // interior: eye raised and pitched down so the road ahead fills the windshield above the dashboard
    camera.position.copy(car.pos).addScaledVector(car.fwd, 0.6).add(new THREE.Vector3(0, g.interior ? 1.5 : 0.75, 0));
    camera.lookAt(camera.position.clone().addScaledVector(car.fwd, 20));
    if (g.interior) camera.rotateX(-0.2);
  } else {
    // pull back while wrecked to frame the explosion
    const d = (m.d || 9) * (1 + sf * 0.22) + (player.alive ? 0 : 14);
    camera.position.copy(car.pos).addScaledVector(fwd, -d);
    camera.position.y += (m.h || 2.7) - sf * 0.6 + (player.alive ? 0 : 6);
    // looking around: aim at the car itself instead of the road ahead of it
    camera.lookAt(car.pos.clone().addScaledVector(aimCam ? fwd : car.fwd, 9 * (1 - orbit)).add(new THREE.Vector3(0, 0.9, 0)));
  }
  camera.rotateZ(car.roll * 0.28 * (1 - orbit));
  const shake = g.shake + 0.08 * car.nitro;
  if (shake > 0.001) {
    const t = performance.now() * 0.05;
    camera.rotateX(Math.sin(t * 1.7) * 0.012 * shake);
    camera.rotateY(Math.cos(t * 2.3) * 0.012 * shake);
  }
  g.shake = Math.max(0, g.shake - dt * 2.2);
  const fovT = 68 + 15 * Math.min(1, sf) + 9 * car.nitro + (cockpit && g.interior ? 8 : 0); // the cockpit sees a bit wider
  cam.fov += (fovT - cam.fov) * (1 - Math.exp(-4 * dt));
  g.kick = Math.max(0, (g.kick || 0) - dt * 2.2);
  const punch = g.kick * g.kick;
  camera.fov = cam.fov + punch * 10; // ignition punch: snaps wide, settles back
  camera.updateProjectionMatrix();
  lens.uniforms.uSpeed.value = Math.max(0, sf - 0.55) * 1.2 + 0.5 * car.nitro + punch * 0.9;
  lens.uniforms.uCA.value = 0.003 + sf * 0.004 + g.shake * 0.02 + punch * 0.02;
  lens.uniforms.uLines.value = car.nitro * (0.5 + 0.5 * Math.min(1, sf)) + punch * 0.6;
  lens.uniforms.uAspect.value = camera.aspect;
}

/** A camera above the track looking ahead: used once at start-up to warm up the shaders (TRACK FLYOVER, which also
 *  used it, was removed in v0.2.3). */
export function cinematicCamera(t, dt, speed = 0.011) {
  tourT += dt * speed;
  const u = tourT % 1;
  const p = track.curve.getPointAt(u);
  const ahead = track.curve.getPointAt((u + 0.012) % 1);
  const tan = track.curve.getTangentAt(u);
  const side = new THREE.Vector3().crossVectors(tan, THREE.Object3D.DEFAULT_UP).normalize();
  const sway = Math.sin(t * 0.13);
  camera.position.copy(p).addScaledVector(side, 26 * sway).add(new THREE.Vector3(0, 9 + 6 * Math.cos(t * 0.21), 0));
  camera.lookAt(ahead);
  camera.fov = 62;
  camera.updateProjectionMatrix();
  lens.uniforms.uSpeed.value = 0.15;
  lens.uniforms.uCA.value = 0.003;
  lens.uniforms.uLines.value = 0;
}

// showcase (main menu and pause): the camera swings round to the front of the car; drag to orbit, scroll to zoom
export const show = { yaw: 0.6, pitch: 0.14, dist: 10.5, drag: null };
export function showcaseCamera(dt) {
  if (show.drag) show.swing = null;
  else if (g.state === 'menu' && show.swing !== null) {
    // the main menu: sway in front of the car, so the LED screen behind it stays in view (until the player drags)
    show.swing = (show.swing ?? 0) + dt;
    show.yaw += (0.62 + 0.42 * Math.sin(show.swing * 0.16) - show.yaw) * Math.min(1, dt * 2);
  } else show.yaw += dt * 0.12;
  const target = car.pos.clone().add(new THREE.Vector3(0, -0.1, 0));
  const a = car.yaw + show.yaw, cp = Math.cos(show.pitch);
  const want = new THREE.Vector3(-Math.sin(a) * cp, Math.sin(show.pitch), -Math.cos(a) * cp).multiplyScalar(show.dist).add(target);
  const k = 1 - Math.exp(-5 * dt);
  camera.position.lerp(want, k);
  // aim to the car's left on screen so it sits to the right of the menu (the main menu is wider than the pause one);
  // the shop on a phone has its cards along the bottom, so the car stays centred
  const side = g.state === 'menu' ? 0.2 : g.state === 'shop' ? (D.touch ? 0 : 0.13) : 0.12;
  target.addScaledVector(new THREE.Vector3(Math.cos(a), 0, -Math.sin(a)), show.dist * side);
  if (g.state === 'shop') target.y -= show.dist * 0.07; // look a little lower: the car rides higher, above the detail text
  camera.lookAt(target);
  camera.fov += (34 - camera.fov) * k;
  camera.updateProjectionMatrix();
  lens.uniforms.uSpeed.value = 0;
  lens.uniforms.uLines.value = 0;
  lens.uniforms.uCA.value = 0.0015;
}
$('pause').addEventListener('pointerdown', (e) => {
  if (e.target.closest('button, a')) return;
  show.drag = { x: e.clientX, y: e.clientY };
});
$('menu').addEventListener('pointerdown', (e) => {
  if (e.target !== $('menu') || e.pointerType === 'touch') return; // phones: the menu scrolls instead
  show.drag = { x: e.clientX, y: e.clientY };
});
// shop: drag the stage (the cards and buttons are separate, so a swipe there never selects a car)
$('shop-stage').addEventListener('pointerdown', (e) => { show.drag = { x: e.clientX, y: e.clientY }; });
$('shop-stage').addEventListener('wheel', zoomShowcase, { passive: true });
addEventListener('pointermove', (e) => {
  if (!show.drag || (g.state !== 'paused' && g.state !== 'menu' && g.state !== 'shop')) return;
  show.yaw -= (e.clientX - show.drag.x) * 0.008;
  show.pitch = THREE.MathUtils.clamp(show.pitch + (e.clientY - show.drag.y) * 0.005, -0.15, 1.2);
  show.drag = { x: e.clientX, y: e.clientY };
});
addEventListener('pointerup', () => { show.drag = null; });
function zoomShowcase(e) { show.dist = THREE.MathUtils.clamp(show.dist * (1 + Math.sign(e.deltaY) * 0.1), 6, 22); }
const zoomShow = zoomShowcase;
$('pause').addEventListener('wheel', zoomShow, { passive: true });
$('menu').addEventListener('wheel', (e) => { if (e.target === $('menu')) zoomShow(e); }, { passive: true });
