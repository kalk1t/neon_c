export class Input {
  constructor() {
    this.keys = new Set();
    this.pressed = new Set();
    this.mouse = { dx: 0, dy: 0, left: false, right: false };
    this.padPrev = {};
    addEventListener('keydown', (e) => {
      if (e.target.tagName === 'INPUT') return; // typing a nickname isn't driving
      if (['Space', 'ArrowUp', 'ArrowDown', 'ArrowLeft', 'ArrowRight'].includes(e.code)) e.preventDefault();
      if (!this.keys.has(e.code)) this.pressed.add(e.code);
      this.keys.add(e.code);
    });
    addEventListener('keyup', (e) => this.keys.delete(e.code));
    addEventListener('blur', () => {
      this.keys.clear();
      this.mouse.left = this.mouse.right = false;
    });
    this.lockedAt = 0;
    this.avgMove = 0;
    document.addEventListener('pointerlockchange', () => {
      this.lockedAt = performance.now();
      this.avgMove = 0;
      // a button held while capture is lost never sends its mouseup: release it so it can't stay stuck
      if (!this.locked) this.mouse.left = this.mouse.right = false;
    });
    addEventListener('mousemove', (e) => {
      if (!this.locked) return;
      // which buttons are really down: a release the browser never delivered (it happens, e.g. around the right-click
      // menu) would otherwise leave free look or the blaster stuck on until the button is pressed again
      this.mouse.left &&= !!(e.buttons & 1);
      this.mouse.right &&= !!(e.buttons & 2);
      // the first events after capture can carry the click's leftover motion: skip them so the aim stays centred
      if (performance.now() - this.lockedAt < 150) return;
      // Chrome on Windows sometimes reports one bogus jump while captured. Drop a single event only when it is wildly
      // above the recent motion, and never two in a row, so fast real flicks (high-DPI mice) always get through.
      const mag = Math.hypot(e.movementX, e.movementY);
      const bogus = mag > 2000 || (mag > 300 && mag > 12 * (this.avgMove + 10));
      if (bogus && !this.dropped) {
        this.dropped = true;
        return;
      }
      this.dropped = false;
      this.avgMove += (mag - this.avgMove) * 0.2;
      this.mouse.dx += e.movementX;
      this.mouse.dy += e.movementY;
    });
    addEventListener('mousedown', (e) => {
      if (!this.locked && e.target.tagName !== 'CANVAS') return;
      if (e.button === 0) { this.mouse.left = true; this.pressed.add('Mouse0'); }
      if (e.button === 1) { e.preventDefault(); this.pressed.add('Mouse1'); }
      if (e.button === 2) this.mouse.right = true;
    });
    addEventListener('mouseup', (e) => {
      if (e.button === 0 || !(e.buttons & 1)) this.mouse.left = false;
      if (e.button === 2 || !(e.buttons & 2)) this.mouse.right = false;
    });
    addEventListener('contextmenu', (e) => e.preventDefault());
  }

  get locked() {
    return !!document.pointerLockElement;
  }
  /** Captures the mouse (with the player's normal Windows pointer speed). Chrome refuses a new capture for about a
   *  second after the player pressed Esc, so a refused request is retried while `retry()` holds. */
  lock(el, retry = () => false, tries = 4) {
    if (this.locked) return;
    const again = () => {
      if (tries > 0 && !this.locked && retry()) setTimeout(() => this.lock(el, retry, tries - 1), 400);
    };
    if (!el.requestPointerLock) return;
    // older browsers return nothing (and report failure only via 'pointerlockerror')
    el.requestPointerLock()?.catch?.(again);
  }
  unlock() {
    if (this.locked) document.exitPointerLock();
  }
  /** Mouse movement since last call (pixels). */
  takeMouse() {
    const m = { dx: this.mouse.dx, dy: this.mouse.dy };
    this.mouse.dx = this.mouse.dy = 0;
    return m;
  }

  /** Consume a one-shot press. */
  hit(...codes) {
    let any = false;
    for (const c of codes) if (this.pressed.delete(c)) any = true;
    return any;
  }
  endFrame() {
    this.pressed.clear();
  }
  state() {
    const k = (c) => (this.keys.has(c) ? 1 : 0);
    const s = {
      throttle: k('KeyW'),
      brake: k('KeyS'),
      // A / D (v0.2.13): with the mouse captured the mouse turns the car, so A / D slide it sideways instead; without
      // the mouse (keyboard only) they turn, like the arrows always do
      steer: k('ArrowRight') - k('ArrowLeft') + (this.locked ? 0 : k('KeyD') - k('KeyA')),
      strafe: this.locked ? k('KeyD') - k('KeyA') : 0,
      climb: k('ArrowUp') - k('ArrowDown') + k('KeyE') - k('KeyQ'),
      boost: !!(k('ShiftLeft') || k('ShiftRight')),
      fire: !!(this.mouse.left || k('KeyJ')),
      look: this.mouse.right, // hold RMB: the mouse orbits the camera around the car instead of steering
    };
    s.keySteer = s.steer !== 0;
    s.keyClimb = s.climb !== 0;
    const pad = navigator.getGamepads ? [...navigator.getGamepads()].find((p) => p) : null;
    if (pad) {
      const dz = (v) => (Math.abs(v) < 0.15 ? 0 : v);
      const ps = dz(pad.axes[0]), pc = -dz(pad.axes[1]);
      if (ps) { s.steer = ps; s.keySteer = true; }
      if (pc) { s.climb = pc; s.keyClimb = true; }
      s.throttle = Math.max(s.throttle, pad.buttons[7]?.value || 0);
      s.brake = Math.max(s.brake, pad.buttons[6]?.value || 0);
      s.boost = s.boost || !!pad.buttons[0]?.pressed;
      s.fire = s.fire || !!pad.buttons[5]?.pressed;
      const edge = (i, code) => {
        const p = !!pad.buttons[i]?.pressed;
        if (p && !this.padPrev[i]) this.pressed.add(code);
        this.padPrev[i] = p;
      };
      edge(1, 'KeyF'); // B: spin dodge
      edge(2, 'Space'); // X: seeker missile
      edge(3, 'KeyC');
      edge(4, 'KeyX');
      edge(9, 'Escape');
      edge(8, 'KeyR');
    }
    s.steer = Math.max(-1, Math.min(1, s.steer));
    s.climb = Math.max(-1, Math.min(1, s.climb));
    return s;
  }
}
