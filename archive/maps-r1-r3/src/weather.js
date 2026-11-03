import * as THREE from 'three';
import { FOG_COLOR, rng } from './materials.js';

/**
 * Rain: streaks in a box that follows the camera (so it costs the same anywhere on the map). They fall at RAIN_SPEED,
 * slanted by the wind, and wrap round inside the box. amount 0..1 scales how many are drawn.
 */
const BOX = 140, RAIN_SPEED = 46, N = 5000;
export function buildRain(scene, amount = 1, tint = new THREE.Color(0.55, 0.65, 0.85)) {
  const r = rng(91);
  const base = new Float32Array(N * 3), seed = new Float32Array(N);
  for (let i = 0; i < N; i++) {
    base[i * 3] = r() * BOX; base[i * 3 + 1] = r() * BOX; base[i * 3 + 2] = r() * BOX;
    seed[i] = r();
  }
  // each streak is a line: two vertices sharing the drop's position, the second one up-wind (aEnd = 1)
  const geo = new THREE.BufferGeometry();
  const pos = new Float32Array(N * 2 * 3), end = new Float32Array(N * 2), sd = new Float32Array(N * 2);
  for (let i = 0; i < N; i++) {
    for (let k = 0; k < 2; k++) {
      pos.set(base.subarray(i * 3, i * 3 + 3), (i * 2 + k) * 3);
      end[i * 2 + k] = k;
      sd[i * 2 + k] = seed[i];
    }
  }
  geo.setAttribute('position', new THREE.BufferAttribute(pos, 3));
  geo.setAttribute('aEnd', new THREE.BufferAttribute(end, 1));
  geo.setAttribute('aSeed', new THREE.BufferAttribute(sd, 1));
  geo.setDrawRange(0, Math.round(N * amount) * 2);
  const mat = new THREE.ShaderMaterial({
    transparent: true, depthWrite: false, blending: THREE.AdditiveBlending, fog: false,
    uniforms: { uTime: { value: 0 }, uCam: { value: new THREE.Vector3() }, uVel: { value: new THREE.Vector3() }, uTint: { value: tint } },
    vertexShader: /* glsl */ `
      attribute float aEnd, aSeed; uniform float uTime; uniform vec3 uCam, uVel; varying float vA;
      void main(){
        vec3 fall = vec3(-6., -${RAIN_SPEED}.0 * (0.85 + aSeed*0.3), 3.);
        vec3 p = position + fall * uTime;
        // wrap into the box around the camera
        p = mod(p - uCam + ${BOX / 2}.0, ${BOX}.0) - ${BOX / 2}.0 + uCam;
        // the streak's tail: along the fall and against the camera's motion (fast flying stretches the rain)
        vec3 tail = -(fall * 0.045) + uVel * 0.03;
        p += tail * aEnd;
        vec4 mv = modelViewMatrix * vec4(p, 1.);
        float d = -mv.z;
        vA = (1. - aEnd * 0.9) * smoothstep(2., 8., d) * (1. - smoothstep(30., ${BOX / 2}.0, d));
        gl_Position = projectionMatrix * mv;
      }`,
    fragmentShader: /* glsl */ `
      uniform vec3 uTint; varying float vA;
      void main(){ gl_FragColor = vec4(uTint * vA * 0.55, 1.); }`,
  });
  const lines = new THREE.LineSegments(geo, mat);
  lines.frustumCulled = false;
  lines.renderOrder = 5;
  scene.add(lines);
  return {
    mesh: lines,
    update(t, camPos, vel) {
      mat.uniforms.uTime.value = t;
      mat.uniforms.uCam.value.copy(camPos);
      if (vel) mat.uniforms.uVel.value.copy(vel);
    },
  };
}

/**
 * Water: a big dark mirror-like plane with rain ripples and wind waves (normal map scrolled two ways). It reflects the
 * scene's environment map and fades into the fog.
 */
export function buildWater(scene, { y, color = 0x04060c, rough = 0.12, size = 9000, cx = 0, cz = 0 }) {
  const tex = rippleTexture();
  tex.wrapS = tex.wrapT = THREE.RepeatWrapping;
  tex.repeat.set(size / 40, size / 40);
  const mat = new THREE.MeshStandardMaterial({
    color, metalness: 0.9, roughness: rough, normalMap: tex, normalScale: new THREE.Vector2(0.35, 0.35), envMapIntensity: 1.2,
  });
  const mesh = new THREE.Mesh(new THREE.PlaneGeometry(size, size).rotateX(-Math.PI / 2), mat);
  mesh.position.set(cx, y, cz);
  scene.add(mesh);
  return {
    mesh,
    update(t) {
      tex.offset.set(t * 0.011, t * 0.007);
    },
  };
}

/** Tileable normal map: soft wave noise plus small ring ripples (rain on water). */
function rippleTexture() {
  const S = 256, c = document.createElement('canvas');
  c.width = c.height = S;
  const g = c.getContext('2d');
  const h = new Float32Array(S * S);
  const r = rng(17);
  // waves: a few sine trains in tile-friendly directions
  for (let k = 0; k < 6; k++) {
    const fx = Math.round(1 + r() * 4), fz = Math.round(1 + r() * 4), ph = r() * 6.28, a = 0.4 + r() * 0.6;
    for (let y = 0; y < S; y++) for (let x = 0; x < S; x++) h[y * S + x] += a * Math.sin(((x * fx + y * fz) / S) * 6.283 + ph);
  }
  // rain rings
  for (let k = 0; k < 70; k++) {
    const cx = r() * S, cy = r() * S, rad = 3 + r() * 9;
    for (let y = -14; y <= 14; y++) for (let x = -14; x <= 14; x++) {
      const d = Math.hypot(x, y);
      if (d > rad + 3) continue;
      const v = Math.cos((d - rad) * 1.6) * Math.max(0, 1 - Math.abs(d - rad) / 3);
      const px = (Math.floor(cx + x) + S) % S, py = (Math.floor(cy + y) + S) % S;
      h[py * S + px] += v * 1.4;
    }
  }
  const img = g.createImageData(S, S);
  for (let y = 0; y < S; y++) for (let x = 0; x < S; x++) {
    const dx = h[y * S + ((x + 1) % S)] - h[y * S + ((x - 1 + S) % S)];
    const dy = h[((y + 1) % S) * S + x] - h[((y - 1 + S) % S) * S + x];
    const n = new THREE.Vector3(-dx, -dy, 4).normalize();
    const i = (y * S + x) * 4;
    img.data[i] = (n.x * 0.5 + 0.5) * 255;
    img.data[i + 1] = (n.y * 0.5 + 0.5) * 255;
    img.data[i + 2] = (n.z * 0.5 + 0.5) * 255;
    img.data[i + 3] = 255;
  }
  g.putImageData(img, 0, 0);
  return new THREE.CanvasTexture(c);
}

