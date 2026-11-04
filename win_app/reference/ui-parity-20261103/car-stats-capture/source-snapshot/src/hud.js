import * as THREE from 'three';
import { WEAPON } from './ai.js';
import { SPIN } from './car.js';

const $ = (id) => document.getElementById(id);
export const fmt = (t) => {
  if (t == null || !isFinite(t)) return '--:--.--';
  const m = Math.floor(t / 60), s = t - m * 60;
  return `${String(m).padStart(2, '0')}:${s.toFixed(2).padStart(5, '0')}`;
};
const hex = (c) => `#${c.toString(16).padStart(6, '0')}`;

/** Speed readouts: km/h or mph (Settings, v0.2.15). */
export const SPEED_UNITS = { kmh: { k: 3.6, label: 'km/h' }, mph: { k: 2.23694, label: 'mph' } };

export class HUD {
  constructor(track) {
    this.setTrack(track);
    this.el = {
      hud: $('hud'), speed: $('speed'), boost: $('boost'), shield: $('shield'), lap: $('lap'), time: $('time'), cp: $('cp'),
      atk: $('atk-ring'), msg: $('msg'), sub: $('submsg'), target: $('target'), tdist: $('target-dist'),
      arrow: $('edge-arrow'), list: $('lap-list'), best: $('best'), flash: $('flash'), nitro: $('nitro-call'), pos: $('pos'), posTotal: $('pos-total'),
      posPanel: $('pos-panel'), aim: $('aim'), cross: $('crosshair'), lock: $('lock'), lockName: $('lock-name'),
      tags: $('tags'), hint: $('lockhint'), heat: $('heat'), pips: $('pips'), emp: $('emp-ring'), spin: $('spin-ring'), hitmark: $('hitmark'),
      coins: $('coins'), coinRank: $('coin-rank'), coinPanel: $('coin-panel'),
    };
    this.el.pips.innerHTML = '<i></i>'.repeat(WEAPON.MISSILES);
    this.msgTimer = 0;
    this.subTimer = 0;
    this.tagEls = new Map();
    this.ctx = document.querySelector('#minimap canvas').getContext('2d');
    this._v = new THREE.Vector3();
    this.z = 1; // the HUD's CSS zoom (see update)
  }

  /** A new map: its track for the minimap, ring count and target; names = { race, roam } for the minimap label. */
  setTrack(track, names = { race: 'SKYLINE LOOP', roam: 'SKYLINE CITY' }) {
    this.track = track;
    this.names = names;
    this.mapLabel = null;
    // minimap projection
    const b = track.bounds;
    const cx = (b.min.x + b.max.x) / 2, cz = (b.min.z + b.max.z) / 2;
    const span = Math.max(b.max.x - b.min.x, b.max.z - b.min.z) * 1.22;
    this.map = (x, z) => [220 + ((x - cx) / span) * 440 * 0.82, 220 + ((z - cz) / span) * 440 * 0.82];
  }

  show(on) {
    this.el.hud.classList.toggle('hidden', !on);
  }

  message(text, dur = 1.2, warn = false) {
    this.el.msg.textContent = text;
    this.el.msg.classList.add('show');
    this.el.msg.classList.toggle('warn', warn);
    this.msgTimer = dur;
  }
  /** The line under the big message. Plain text only: it often carries other players' nicknames (C2). */
  sub(text, dur = 1.6) {
    this.el.sub.textContent = text;
    this.el.sub.classList.add('show');
    this.subTimer = dur;
  }
  /** FREE ROAM kill feed: a line per takedown between other players, newest on top, each fading after a few seconds. */
  feed(text) {
    const box = $('feed'), line = document.createElement('div');
    line.textContent = text; // nicknames only ever go through textContent
    box.prepend(line);
    while (box.children.length > 4) box.lastChild.remove();
    setTimeout(() => line.remove(), 4500);
  }
  /** "NITRO" call-out on ignition. */
  nitro() {
    const n = this.el.nitro;
    if (!n) return;
    n.classList.remove('go');
    void n.offsetWidth; // restart the animation
    n.classList.add('go');
  }
  flash(color = 'rgba(255, 43, 214, 0.5)') {
    const f = this.el.flash;
    f.style.background = `radial-gradient(transparent 40%, ${color})`;
    f.style.transition = 'none';
    f.style.opacity = 1;
    requestAnimationFrame(() => {
      f.style.transition = 'opacity .5s';
      f.style.opacity = 0;
    });
  }

  hitmark(kill = false) {
    const h = this.el.hitmark;
    h.classList.remove('on', 'kill');
    void h.offsetWidth; // restart the animation
    h.classList.add('on');
    h.classList.toggle('kill', kill);
    clearTimeout(this.hmT);
    this.hmT = setTimeout(() => h.classList.remove('on', 'kill'), kill ? 380 : 130);
  }
  coinPop() {
    const p = this.el.coinPanel;
    p.classList.remove('pop');
    void p.offsetWidth;
    p.classList.add('pop');
  }

  laps(list, best) {
    const bi = list.length ? list.indexOf(Math.min(...list)) : -1;
    this.el.list.innerHTML = list.map((t, i) => `<li class="${i === bi ? 'best' : ''}">${fmt(t)}</li>`).join('');
    this.el.best.textContent = fmt(best);
  }

  /** Projects a world point to HUD coordinates; returns null when behind the camera. */
  screen(p, camera) {
    const v = this._v.copy(p).project(camera);
    if (v.z > 1) return null;
    return [(v.x * 0.5 + 0.5) * innerWidth / this.z, (-v.y * 0.5 + 0.5) * innerHeight / this.z, v];
  }

  place(el, s) {
    if (!s) { el.style.display = 'none'; return; }
    el.style.display = 'block';
    el.style.left = `${s[0]}px`;
    el.style.top = `${s[1]}px`;
  }

  update(dt, g, camera) {
    const e = this.el, P = g.player, car = P.car;
    // style.css shrinks the HUD on smaller screens (zoom 0.8 on laptops, 0.55 on phones), and zoom scales left/top too:
    // positions over the 3D view are divided by it, or the crosshair, aim and tags drift to the top-left
    this.z = parseFloat(getComputedStyle(e.cross).zoom) || 1;
    if ((this.msgTimer -= dt) <= 0) e.msg.classList.remove('show');
    if ((this.subTimer -= dt) <= 0) e.sub.classList.remove('show');
    e.speed.textContent = Math.round(Math.hypot(car.speed, car.side ?? 0) * SPEED_UNITS[this.units ?? 'kmh'].k); // forwards and sideways
    e.boost.style.width = `${car.boost * 100}%`;
    e.boost.parentElement.classList.toggle('on', car.boosting);
    e.boost.parentElement.classList.toggle('low', car.boost < 0.2);
    const hpPct = (100 * P.hp) / P.maxHp; // health as a share of the car's own (CAR_STATS, v0.2.17)
    e.shield.style.width = `${Math.max(0, hpPct)}%`;
    e.shield.parentElement.classList.toggle('low', hpPct < 35);
    e.lap.textContent = g.laps === Infinity ? `${P.lap}` : `${Math.min(P.lap, g.laps)}/${g.laps}`;
    e.time.textContent = fmt(P.lapTime);
    e.cp.textContent = `${P.passed.size}/${this.track.checkpoints.length}`;
    e.posPanel.style.display = g.racers.length > 1 ? 'flex' : 'none';
    e.pos.textContent = g.position;
    e.posTotal.textContent = `/${g.racers.length}`;
    // weapons: blaster heat, seeker charges (ring = next charge), EMP cooldown
    e.heat.style.width = `${g.heat * 100}%`;
    e.heat.parentElement.classList.toggle('over', g.overheated);
    const full = Math.floor(g.missiles);
    [...e.pips.children].forEach((p, i) => p.classList.toggle('on', i < full));
    const charges = car.stats.missiles; // the TITAN carries 4 (v0.2.17)
    if (e.pips.children.length !== charges) e.pips.innerHTML = '<i></i>'.repeat(charges);
    e.atk.style.setProperty('--p', `${(full >= charges ? 1 : g.missiles - full) * 100}%`);
    e.atk.classList.toggle('cool', full < 1);
    e.emp.style.setProperty('--p', `${(1 - g.empCD / WEAPON.EMP_COOLDOWN) * 100}%`);
    e.emp.classList.toggle('cool', g.empCD > 0);
    e.spin.style.setProperty('--p', `${(1 - g.spinCD / car.stats.spinCD) * 100}%`);
    e.spin.classList.toggle('cool', g.spinCD > 0);
    // coins
    e.coins.textContent = P.coins;
    if (g.racers.length > 1) {
      const better = g.racers.filter((r) => r.coins > P.coins).length;
      e.coinRank.textContent = `${better + 1}/${g.racers.length}`;
    } else e.coinRank.textContent = '';
    e.hint.style.display = g.state === 'race' && !g.mouseLocked ? 'block' : 'none';

    // mouse aim ring (with the mouse captured: one ring, in the middle, the camera follows it) and where the nose points:
    // a small dot then, the full crosshair without the mouse (keys, touch, gamepad)
    const aimS = g.mouseLocked && P.alive ? this.screen(car.pos.clone().addScaledVector(g.aimDir, 90), camera) : null;
    e.cross.classList.toggle('nose', !!aimS);
    this.place(e.cross, this.screen(car.pos.clone().addScaledVector(car.fwd, 90), camera));
    this.place(e.aim, aimS);
    if (aimS) { e.hitmark.style.left = `${aimS[0]}px`; e.hitmark.style.top = `${aimS[1]}px`; }

    // lock-on bracket
    if (g.lock && g.lock.alive) {
      this.place(e.lock, this.screen(g.lock.car.pos, camera));
      const locked = g.lockT >= 1;
      e.lock.classList.toggle('locked', locked);
      // bracket closes in while acquiring
      e.lock.firstChild.style.transform = `scale(${1 + (1 - g.lockT) * 0.9}) rotate(${(1 - g.lockT) * 45}deg)`;
      e.lockName.textContent = `${locked ? 'LOCKED' : 'LOCKING'} ${g.lock.name} ${Math.round(car.pos.distanceTo(g.lock.car.pos))}m`;
    } else e.lock.style.display = 'none';

    this.updateTags(g, camera);

    // next-target marker
    const tgt = g.targetPos();
    const v = this._v.copy(tgt).project(camera);
    const dist = car.pos.distanceTo(tgt);
    const W = innerWidth / this.z, H = innerHeight / this.z;
    const behind = v.z > 1;
    const on = !behind && Math.abs(v.x) < 0.92 && Math.abs(v.y) < 0.9;
    e.target.style.display = on ? 'block' : 'none';
    e.arrow.style.display = on ? 'none' : 'block';
    if (on) {
      e.target.style.left = `${(v.x * 0.5 + 0.5) * W}px`;
      e.target.style.top = `${(-v.y * 0.5 + 0.5) * H}px`;
      e.tdist.textContent = `${Math.round(dist)} m`;
    } else {
      let x = v.x, y = v.y;
      if (behind) { x = -x; y = -y; }
      const a = Math.atan2(y, x);
      const r = Math.min(W, H) * 0.38;
      e.arrow.style.left = `${W / 2 + Math.cos(a) * r - 20}px`;
      e.arrow.style.top = `${H / 2 - Math.sin(a) * r - 25}px`;
      e.arrow.style.transform = `rotate(${-a}rad)`;
    }
    this.drawMap(g);
  }

  /** Name + health tags over nearby rivals. */
  updateTags(g, camera) {
    const me = g.player.car.pos;
    for (const r of [...g.racers, ...g.remotes]) {
      if (r.isPlayer) continue;
      let el = this.tagEls.get(r);
      if (!el) {
        el = document.createElement('div');
        el.className = 'tag';
        el.style.color = hex(r.color);
        el.innerHTML = `<span></span><div class="hp"><i></i></div>`;
        this.el.tags.appendChild(el);
        this.tagEls.set(r, el);
      }
      const d = me.distanceTo(r.car.pos);
      // other players' names show at any distance; rivals only nearby
      const s = r.car.root.visible && (r.remote || (r.alive && d < 320)) ? this.screen(r.car.pos.clone().add(new THREE.Vector3(0, 3.2, 0)), camera) : null;
      if (!s || Math.abs(s[2].x) > 1 || Math.abs(s[2].y) > 1) { el.style.display = 'none'; continue; }
      el.style.display = 'block';
      el.style.left = `${s[0]}px`;
      el.style.top = `${s[1]}px`;
      el.classList.toggle('remote', !!r.remote);
      if (r.remote) {
        el.style.color = hex(r.color);
        el.style.opacity = 1;
        el.firstChild.textContent = r.name;
        el.firstChild.dataset.d = d < 1000 ? `${Math.round(d)} m` : `${(d / 1000).toFixed(1)} km`;
        el.lastChild.firstChild.style.width = `${Math.max(0, (100 * r.hp) / r.maxHp)}%`;
        continue;
      }
      el.style.opacity = d < 40 ? 1 : Math.max(0.35, 1 - (d - 40) / 300);
      el.firstChild.textContent = `${g.rankOf(r)} ${r.name}`;
      el.lastChild.firstChild.style.width = `${Math.max(0, (100 * r.hp) / r.maxHp)}%`;
      el.firstChild.textContent += r.coins ? ` · ${r.coins}¢` : '';
    }
    for (const [r, el] of this.tagEls) if (!g.racers.includes(r) && !g.remotes.includes(r)) el.style.display = 'none';
  }

  /** FREE ROAM map: the city's building footprints (from city.js towers), and where home is (the spawn hub). */
  setCity(towers, home) {
    this.home = home;
    this.blocks = towers.map((b) => {
      const cs = Math.cos(b.rot), sn = Math.sin(b.rot);
      const pts = [[-1, -1], [1, -1], [1, 1], [-1, 1]].map(([u, v]) => {
        const lx = (u * b.w) / 2, lz = (v * b.d) / 2;
        return [b.x + lx * cs + lz * sn, b.z - lx * sn + lz * cs];
      });
      return { x: b.x, z: b.z, r: Math.hypot(b.w, b.d) / 2, top: b.top, pts };
    });
  }

  drawMap(g) {
    const label = g.mode === 'roam' ? this.names.roam : this.names.race;
    if (this.mapLabel !== label) {
      this.mapLabel = label;
      document.querySelector('#minimap span').textContent = label;
    }
    if (g.mode === 'roam' && this.blocks) return this.drawCityMap(g);
    const c = this.ctx, t = this.track;
    c.clearRect(0, 0, 440, 440);
    c.lineJoin = 'round';
    c.beginPath();
    t.pts.forEach((p, i) => {
      const [x, y] = this.map(p.x, p.z);
      i ? c.lineTo(x, y) : c.moveTo(x, y);
    });
    c.closePath();
    c.strokeStyle = 'rgba(25,230,255,0.25)';
    c.lineWidth = 14;
    c.stroke();
    c.strokeStyle = '#19e6ff';
    c.lineWidth = 4;
    c.shadowColor = '#19e6ff';
    c.shadowBlur = 12;
    c.stroke();
    c.shadowBlur = 0;
    const P = g.player;
    t.checkpoints.forEach((cp) => {
      const [x, y] = this.map(cp.pos.x, cp.pos.z);
      const next = cp.index === P.nextCP;
      c.fillStyle = P.passed.has(cp.index) ? '#19e6ff' : '#2bff7a';
      c.beginPath();
      c.arc(x, y, next ? 11 : 7, 0, Math.PI * 2);
      c.fill();
      if (next) {
        c.strokeStyle = '#fff';
        c.lineWidth = 3;
        c.stroke();
      }
    });
    const [sx, sy] = this.map(0, 0);
    c.fillStyle = '#fff';
    c.fillRect(sx - 12, sy - 3, 24, 6);
    // rivals
    for (const r of [...g.racers, ...g.remotes]) {
      if (r.isPlayer || !r.alive || !r.car.root.visible) continue;
      const [x, y] = this.map(r.car.pos.x, r.car.pos.z);
      c.fillStyle = hex(r.color);
      c.beginPath();
      c.arc(x, y, r === g.lock ? 9 : 6, 0, Math.PI * 2);
      c.fill();
    }
    // player
    const [px, py] = this.map(P.car.pos.x, P.car.pos.z);
    c.save();
    c.translate(px, py);
    c.rotate(-P.car.yaw);
    c.fillStyle = '#fff';
    c.shadowColor = '#ff2bd6';
    c.shadowBlur = 14;
    c.beginPath();
    c.moveTo(0, -16);
    c.lineTo(11, 12);
    c.lineTo(0, 6);
    c.lineTo(-11, 12);
    c.closePath();
    c.fill();
    c.restore();
  }

  /** FREE ROAM: a radar of the city around you (north up): buildings, other players, the way home. */
  drawCityMap(g) {
    const c = this.ctx, P = g.player.car.pos;
    const VIEW = 650; // metres from the centre to the rim
    const k = 200 / VIEW;
    const at = (x, z) => [220 + (x - P.x) * k, 220 + (z - P.z) * k];
    c.clearRect(0, 0, 440, 440);
    c.save();
    c.beginPath();
    c.arc(220, 220, 219, 0, Math.PI * 2);
    c.clip();
    // range rings every 200 m
    c.strokeStyle = 'rgba(25, 230, 255, 0.12)';
    c.lineWidth = 2;
    for (const r of [200, 400, 600]) {
      c.beginPath();
      c.arc(220, 220, r * k, 0, Math.PI * 2);
      c.stroke();
    }
    // buildings: taller ones brighter, the tallest in pink; the ones you'd hit at your height outlined
    for (const b of this.blocks) {
      if (Math.abs(b.x - P.x) > VIEW + b.r || Math.abs(b.z - P.z) > VIEW + b.r) continue;
      c.beginPath();
      b.pts.forEach(([x, z], i) => {
        const [sx, sy] = at(x, z);
        i ? c.lineTo(sx, sy) : c.moveTo(sx, sy);
      });
      c.closePath();
      const h = Math.min(1, b.top / 700);
      c.fillStyle = b.top > 520 ? `rgba(255, 43, 214, ${0.35 + h * 0.3})` : `rgba(25, 230, 255, ${0.12 + h * 0.4})`;
      c.fill();
      if (b.top > P.y) {
        c.strokeStyle = b.top > 520 ? '#ff6ae0' : '#5ff0ff';
        c.lineWidth = 2;
        c.stroke();
      }
    }
    c.restore();
    // a dot, or a pip on the rim pointing at it when it's out of range
    const mark = (x, z, color, size, ring) => {
      let dx = (x - P.x) * k, dz = (z - P.z) * k;
      const d = Math.hypot(dx, dz);
      const out = d > 196;
      if (out) { dx *= 196 / d; dz *= 196 / d; }
      c.fillStyle = color;
      c.beginPath();
      c.arc(220 + dx, 220 + dz, out ? size * 0.75 : size, 0, Math.PI * 2);
      c.fill();
      if (ring) {
        c.strokeStyle = '#fff';
        c.lineWidth = 2;
        c.stroke();
      }
    };
    if (this.home) mark(this.home.x, this.home.z, '#2bff7a', 8, true);
    for (const r of g.remotes) if (r.car.root.visible) mark(r.car.pos.x, r.car.pos.z, hex(r.color), r === g.lock ? 9 : 7, r === g.lock);
    // you, in the middle
    c.save();
    c.translate(220, 220);
    c.rotate(-g.player.car.yaw);
    c.fillStyle = '#fff';
    c.shadowColor = '#ff2bd6';
    c.shadowBlur = 14;
    c.beginPath();
    c.moveTo(0, -16);
    c.lineTo(11, 12);
    c.lineTo(0, 6);
    c.lineTo(-11, 12);
    c.closePath();
    c.fill();
    c.restore();
  }
}
