// FREE ROAM connection: sends our car ~15 times a second and keeps a short history of everyone else's, so their
// cars can be drawn smoothly a little in the past (interpolation). Each state carries its sender's clock, so playback
// follows the sender's timing rather than the bumpy arrival times. Shots and wrecks are relayed as events.
// Protocol: see server/live.js.
import { LIVE_PROTOCOL } from './protocol.js';

const SEND_MS = 1000 / 15;
const IDLE_SEND_MS = 500; // paused / not driving: a slow keep-alive so we aren't dropped
const PING_MS = 2000; // how often we measure the round trip to the server
const DELAY = 0.15; // playback delay behind the sender: covers network jitter and the server's 15/s ticks
const TEXT = {
  auth: 'SIGN IN TO DRIVE ONLINE',
  nickname: 'CHOOSE A NICKNAME TO DRIVE ONLINE',
  full: 'FREE ROAM IS FULL · RETRYING…',
  again: 'YOU JOINED FROM ANOTHER TAB',
  old: 'GAME UPDATED · RELOADING…',
  banned: 'YOU ARE BANNED FROM FREE ROAM',
};

/** How many people are driving right now (for the menu). */
export async function onlineCount() {
  try {
    const r = await fetch('/live/count', { cache: 'no-store' });
    return r.ok ? (await r.json()).online : null;
  } catch {
    return null;
  }
}

export class LiveRoom {
  /**
   * getToken(forceRefresh): Firebase ID token.
   * on.appCheck(): App Check token or null (sent along with the ID token; the server uses it to read our profile)
   * on: { status(text, ok, code), join(p), leave(p), fire(p, m), kill(m), chat(m), paint() = our car colour }
   * kill(m): { k, v, n } player k wrecked player v (either may be us: compare with this.me.id); n = k's session kills
   * chat(m): { id, name, s } a chat message (ours come back too, once the server has passed them on)
   * hit(p, m): { k, d } player p's bolt / missile hit us on their screen
   * killer(m): { id, on, kc } KILLER STATUS: player id (maybe us) became a Killer / cleared it; also sent for us on
   *   every welcome and kill (kc = our killer kill count), so the game can save it
   * cmd(m): { c, ok, msg } the server's answer to our chat command /c
   * saved(): our saved KILLER STATUS { x, kc } (only test logins send it: real ones are read from the profile)
   */
  constructor(getToken, on) {
    Object.assign(this, { getToken, on });
    this.players = new Map(); // id -> { id, name, c, kills, snaps: [{ t, d }], off }
    this.me = null;
    this.online = 0;
    this.closed = false;
    this.retry = 0;
    this.lastSend = 0;
    this.freshToken = false;
    this.ping = null; // ms, round trip to the server (smoothed)
    this.lastPing = 0;
    this.since = null; // performance.now() of our first welcome: time online this session
    this.connect();
  }

  async connect() {
    if (this.closed) return;
    this.fatal = null;
    this.on.status(this.retry ? 'RECONNECTING…' : 'CONNECTING…', false);
    let token = null;
    try { token = await this.getToken(this.freshToken); } catch { token = null; }
    if (this.closed) return;
    if (!token) return this.on.status(TEXT.auth, false, 'auth');
    let ac = null;
    try { ac = (await this.on.appCheck?.()) || null; } catch { ac = null; }
    if (this.closed) return;
    let ws;
    try {
      ws = this.ws = new WebSocket(`${location.protocol === 'https:' ? 'wss' : 'ws'}://${location.host}/live`);
    } catch {
      return this.later();
    }
    // no answer from the server in time: start over
    this.welcomeTimer = setTimeout(() => { if (!this.me) ws.close(); }, 12000);
    ws.onopen = () => ws.send(JSON.stringify({ t: 'hi', token, ...(ac && { ac }), c: this.on.paint?.() ?? -1, car: this.on.car?.() ?? 'roadster', v: LIVE_PROTOCOL, ...this.on.saved?.() }));
    ws.onmessage = (e) => {
      try { this.message(JSON.parse(e.data)); } catch (err) { console.warn('[live]', err); }
    };
    ws.onclose = () => {
      clearTimeout(this.welcomeTimer);
      for (const p of this.players.values()) this.on.leave(p);
      this.players.clear();
      this.me = null;
      this.ping = null;
      if (this.closed || ['nickname', 'again', 'old', 'banned'].includes(this.fatal)) return;
      this.later();
    };
  }

  /** Try again with a growing pause (1 s … 15 s). */
  later() {
    if (this.closed) return;
    this.retry = Math.min(this.retry + 1, 5);
    if (!this.fatal) this.on.status('RECONNECTING…', false);
    clearTimeout(this.timer);
    this.timer = setTimeout(() => this.connect(), Math.min(15000, 500 * 2 ** this.retry));
  }

  message(m) {
    const now = performance.now() / 1000;
    if (m.t === 'welcome') {
      clearTimeout(this.welcomeTimer);
      this.me = m;
      this.me.kills = 0; // kills are counted per session: every join starts from zero
      this.since ??= performance.now();
      this.online = m.n;
      this.retry = 0;
      this.freshToken = false;
      this.on.status('ONLINE', true);
      this.me.killer = !!m.x;
      this.on.welcome?.(m);
      this.on.killer?.({ id: m.id, on: this.me.killer, kc: m.kc ?? 0 });
    } else if (m.t === 'join') {
      const p = { id: m.id, name: m.name, c: m.c ?? -1, car: m.car ?? 'roadster', kills: m.k ?? 0, killer: !!m.x, kc: m.kc ?? 0, snaps: [], off: null };
      this.players.set(m.id, p);
      this.on.join(p);
    } else if (m.t === 'leave') {
      const p = this.players.get(m.id);
      if (p) { this.players.delete(m.id); this.on.leave(p); }
    } else if (m.t === 'w') {
      this.online = m.n;
      for (const [id, ...d] of m.p) {
        const p = this.players.get(id);
        if (!p) continue;
        const sent = d[9];
        if (p.snaps.length && sent <= p.snaps.at(-1).t) continue; // nothing new since the last tick
        // clock offset to the sender, tracking the least delayed packets: eases down quickly, up slowly (no jumps)
        const off = now - sent;
        if (p.off == null || Math.abs(off - p.off) > 2) p.off = off;
        else p.off += (off - p.off) * (off < p.off ? 0.3 : 0.02);
        p.snaps.push({ t: sent, d });
        if (p.snaps.length > 20) p.snaps.shift();
      }
    } else if (m.t === 'f') {
      const p = this.players.get(m.id);
      if (p) this.on.fire(p, m);
    } else if (m.t === 'kill') {
      if (m.k === this.me?.id) this.me.kills = m.n;
      const p = this.players.get(m.k);
      if (p) {
        p.kills = m.n;
        if (m.kc != null) p.kc = m.kc; // their killer kill count: which aura they wear
      }
      this.on.kill?.(m);
      if (m.k === this.me?.id && m.kc != null) this.on.killer?.({ id: m.k, on: !!this.me.killer, kc: m.kc });
    } else if (m.t === 'killer') {
      const p = this.players.get(m.id);
      if (p) {
        p.killer = !!m.on;
        if (m.kc != null) p.kc = m.kc;
      }
      if (m.id === this.me?.id) this.me.killer = !!m.on;
      this.on.killer?.(m);
    } else if (m.t === 'cmd') {
      this.on.cmd?.(m);
    } else if (m.t === 'pong') {
      const rtt = performance.now() - m.c;
      if (rtt >= 0 && rtt < 30000) this.ping = this.ping == null ? rtt : this.ping + (rtt - this.ping) * 0.3;
    } else if (m.t === 'hit') {
      const p = this.players.get(m.id);
      if (p) this.on.hit?.(p, m);
    } else if (m.t === 'chat') {
      this.on.chat?.(m);
    } else if (m.t === 'err') {
      this.fatal = m.code;
      // an expired sign-in: fetch a fresh token and try again once
      if (m.code === 'auth' && !this.freshToken) this.freshToken = true;
      else if (m.code === 'auth') this.fatal = 'nickname'; // still refused: stop retrying
      this.on.status(TEXT[m.code] || 'OFFLINE', false, m.code);
      // this tab runs an older game than the server: load the new one
      // (at most once a minute: a server not yet restarted after an update must not cause a reload loop)
      if (m.code === 'old') {
        const last = +sessionStorage.getItem('neon.reloadedAt') || 0;
        if (Date.now() - last > 60000) {
          sessionStorage.setItem('neon.reloadedAt', String(Date.now()));
          setTimeout(() => location.reload(), 1500);
        } else this.on.status('SERVER UPDATING · TRY AGAIN SOON', false);
      }
    }
  }

  /** Our car: ~15 per second while driving, a slow keep-alive otherwise. hp: 0-100 (0 while wrecked). */
  send(car, driving, hp = 100) {
    const now = performance.now();
    if (this.me && this.ws?.readyState === 1 && now - this.lastPing > PING_MS) {
      this.lastPing = now;
      this.ws.send(JSON.stringify({ t: 'ping', c: now }));
    }
    if (!this.me || this.ws?.readyState !== 1 || now - this.lastSend < (driving ? SEND_MS : IDLE_SEND_MS)) return;
    this.lastSend = now;
    this.ws.send(JSON.stringify({ t: 's', d: [car.pos.x, car.pos.y, car.pos.z, car.yaw, car.pitch, car.roll, car.speed, car.thrust, car.nitro, now / 1000, Math.max(0, hp)] }));
  }

  /** We fired: k = 'bolt' | 'missile' | 'emp' | 'spin' (the dodge roll); tg = the remote player a missile homes on. */
  fire(k, o, d, tg) {
    if (!this.me || this.ws?.readyState !== 1) return;
    this.ws.send(JSON.stringify({ t: 'f', k, o: [o.x, o.y, o.z], d: [d.x, d.y, d.z], tg: tg?.id ?? null }));
  }

  /** We were wrecked by remote player `by` (the server credits them the kill and tells everyone). */
  die(by) {
    if (!this.me || this.ws?.readyState !== 1 || by == null) return;
    this.ws.send(JSON.stringify({ t: 'die', by: by.id }));
  }

  /** Our shot hit remote player p where we see them (they take the damage on their side). */
  hit(p, k, d) {
    if (!this.me || this.ws?.readyState !== 1 || !p) return;
    this.ws.send(JSON.stringify({ t: 'hit', v: p.id, k, d: [d.x, d.y, d.z] }));
  }

  /** Say something to everyone in FREE ROAM. False when not connected. */
  chat(s) {
    if (!this.me || this.ws?.readyState !== 1) return false;
    this.ws.send(JSON.stringify({ t: 'chat', s }));
    return true;
  }

  /** A player's state DELAY seconds ago (on their clock), interpolated between the two snapshots around it. */
  sample(p) {
    const s = p.snaps;
    if (!s.length) return null;
    const t = performance.now() / 1000 - p.off - DELAY;
    if (t <= s[0].t || s.length === 1) return s[0].d;
    for (let i = s.length - 1; i > 0; i--) {
      const a = s[i - 1], b = s[i];
      if (t >= a.t) {
        if (t >= b.t) return b.d; // newest is older than the render time: hold it
        const k = (t - a.t) / (b.t - a.t);
        // a respawn is a jump, not a flight across the map
        if (Math.hypot(b.d[0] - a.d[0], b.d[1] - a.d[1], b.d[2] - a.d[2]) > 150) return k < 0.5 ? a.d : b.d;
        return a.d.map((v, j) => (j === 3 ? lerpAngle(v, b.d[j], k) : v + (b.d[j] - v) * k));
      }
    }
    return s[0].d;
  }

  close() {
    this.closed = true;
    clearTimeout(this.timer);
    clearTimeout(this.welcomeTimer);
    this.ws?.close();
    for (const p of this.players.values()) this.on.leave(p);
    this.players.clear();
  }
}

function lerpAngle(a, b, k) {
  const d = Math.atan2(Math.sin(b - a), Math.cos(b - a));
  return a + d * k;
}
