// Procedural audio (the asset pack ships no sound files). Mixed to sit low and clean:
// soft sine/triangle voices, everything through a gentle high-cut + compressor, a short reverb send,
// stereo placement for world events, and a voice cap so rapid fire never piles up into noise.
const MASTER = 0.28; // overall level at volume slider = 100%
const MAX_VOICES = 18;

export class Sound {
  constructor() {
    this.ctx = null;
    this.muted = false;
    this.volume = 0.7;
    this.voices = 0;
  }

  init() {
    if (this.ctx) return this.ctx.resume();
    const ctx = (this.ctx = new (window.AudioContext || window.webkitAudioContext)());
    this.master = ctx.createGain();
    this.applyVolume();
    const hicut = ctx.createBiquadFilter();
    hicut.type = 'lowpass';
    hicut.frequency.value = 7500;
    hicut.Q.value = 0.5;
    const comp = ctx.createDynamicsCompressor();
    comp.threshold.value = -24;
    comp.knee.value = 12;
    comp.ratio.value = 4;
    comp.attack.value = 0.004;
    comp.release.value = 0.2;
    this.master.connect(hicut).connect(comp).connect(ctx.destination);

    // cues → dry + small reverb
    this.fx = ctx.createGain();
    this.fx.gain.value = 0.8;
    this.fx.connect(this.master);
    const verb = ctx.createConvolver();
    verb.buffer = this.impulse(1.4);
    const wet = ctx.createGain();
    wet.gain.value = 0.16;
    this.fx.connect(verb).connect(wet).connect(this.master);

    // engine: triangle + octave + sine sub, low-passed, slight vibrato
    this.engGain = ctx.createGain();
    this.engGain.gain.value = 0;
    this.engLP = ctx.createBiquadFilter();
    this.engLP.type = 'lowpass';
    this.engLP.Q.value = 0.7;
    this.engLP.connect(this.engGain).connect(this.master);
    const vib = ctx.createOscillator();
    vib.frequency.value = 5.5;
    const vibAmt = ctx.createGain();
    vibAmt.gain.value = 1.2;
    vib.connect(vibAmt);
    vib.start();
    this.oscs = [['triangle', 1, 0.45], ['sine', 2.004, 0.12], ['sine', 0.5, 0.6]].map(([type, mul, vol]) => {
      const o = ctx.createOscillator();
      const g = ctx.createGain();
      o.type = type;
      g.gain.value = vol;
      vibAmt.connect(o.frequency);
      o.connect(g).connect(this.engLP);
      o.start();
      return { o, mul };
    });

    // air rush: brown noise, band-limited, kept very low
    const buf = ctx.createBuffer(1, ctx.sampleRate * 2, ctx.sampleRate);
    const d = buf.getChannelData(0);
    let last = 0;
    for (let i = 0; i < d.length; i++) {
      last = last * 0.97 + (Math.random() * 2 - 1) * 0.03;
      d[i] = last * 6;
    }
    this.noiseBuf = buf;
    const n = ctx.createBufferSource();
    n.buffer = buf;
    n.loop = true;
    const hp = ctx.createBiquadFilter();
    hp.type = 'highpass';
    hp.frequency.value = 140;
    this.windLP = ctx.createBiquadFilter();
    this.windLP.type = 'lowpass';
    this.wind = ctx.createGain();
    this.wind.gain.value = 0;
    n.connect(hp).connect(this.windLP).connect(this.wind).connect(this.master);
    n.start();
  }

  impulse(sec) {
    const ctx = this.ctx, len = ctx.sampleRate * sec;
    const b = ctx.createBuffer(2, len, ctx.sampleRate);
    for (let c = 0; c < 2; c++) {
      const d = b.getChannelData(c);
      for (let i = 0; i < len; i++) d[i] = (Math.random() * 2 - 1) * Math.pow(1 - i / len, 3.5);
    }
    return b;
  }

  applyVolume() {
    if (this.master) this.master.gain.setTargetAtTime(this.muted ? 0 : MASTER * this.volume, this.ctx.currentTime, 0.03);
  }
  setVolume(v) {
    this.volume = v;
    this.applyVolume();
  }
  setMuted(m) {
    this.muted = m;
    this.applyVolume();
  }

  engine(sf, boosting, active) {
    if (!this.ctx) return;
    const t = this.ctx.currentTime;
    const f = 52 + sf * 78 + (boosting ? 14 : 0);
    for (const { o, mul } of this.oscs) o.frequency.setTargetAtTime(f * mul, t, 0.12);
    this.engLP.frequency.setTargetAtTime(320 + sf * 650 + (boosting ? 350 : 0), t, 0.15);
    this.engGain.gain.setTargetAtTime(active ? 0.045 + sf * 0.03 : 0, t, 0.25);
    this.windLP.frequency.setTargetAtTime(220 + sf * 700, t, 0.2);
    this.wind.gain.setTargetAtTime(active ? sf * sf * 0.03 + (boosting ? 0.015 : 0) : 0, t, 0.3);
  }

  /** Output node for a cue, optionally panned (-1..1). */
  out(pan) {
    if (!pan) return this.fx;
    const p = this.ctx.createStereoPanner();
    p.pan.value = Math.max(-1, Math.min(1, pan));
    p.connect(this.fx);
    return p;
  }

  tone(freq, dur = 0.15, type = 'sine', vol = 0.2, when = 0, slide = 0, pan = 0) {
    if (!this.ctx || vol <= 0.002 || this.voices >= MAX_VOICES) return;
    const t = this.ctx.currentTime + when;
    const o = this.ctx.createOscillator();
    const g = this.ctx.createGain();
    o.type = type;
    o.frequency.setValueAtTime(freq, t);
    if (slide) o.frequency.exponentialRampToValueAtTime(slide, t + dur);
    g.gain.setValueAtTime(0, t);
    g.gain.linearRampToValueAtTime(vol, t + 0.008);
    g.gain.exponentialRampToValueAtTime(0.0001, t + dur);
    o.connect(g).connect(this.out(pan));
    this.voices++;
    o.onended = () => this.voices--;
    o.start(t);
    o.stop(t + dur + 0.05);
  }

  /** Filtered noise sweep (whooshes, thumps). */
  rumble(dur, from, to, vol, pan = 0, type = 'lowpass') {
    if (!this.ctx || vol <= 0.002 || this.voices >= MAX_VOICES) return;
    const t = this.ctx.currentTime;
    const s = this.ctx.createBufferSource();
    s.buffer = this.noiseBuf;
    const f = this.ctx.createBiquadFilter();
    f.type = type;
    f.Q.value = type === 'bandpass' ? 1.4 : 0.7;
    f.frequency.setValueAtTime(from, t);
    f.frequency.exponentialRampToValueAtTime(to, t + dur);
    const g = this.ctx.createGain();
    g.gain.setValueAtTime(0, t);
    g.gain.linearRampToValueAtTime(vol, t + Math.min(0.03, dur * 0.2));
    g.gain.exponentialRampToValueAtTime(0.0001, t + dur);
    s.connect(f).connect(g).connect(this.out(pan));
    this.voices++;
    s.onended = () => this.voices--;
    s.start(t, Math.random());
    s.stop(t + dur);
  }

  // ---- race cues ----
  countdown(go) {
    go ? this.tone(880, 0.6, 'sine', 0.12) : this.tone(440, 0.22, 'sine', 0.09);
  }
  checkpoint() {
    [784, 1175].forEach((f, i) => this.tone(f, 0.32, 'sine', 0.05, i * 0.07));
  }
  lap() {
    [659, 784, 988, 1318].forEach((f, i) => this.tone(f, 0.45, 'sine', 0.055, i * 0.08));
  }
  coin(vol = 1, pan = 0) {
    this.tone(1318, 0.09, 'sine', 0.05 * vol, 0, 0, pan);
    this.tone(1976, 0.22, 'sine', 0.045 * vol, 0.06, 0, pan);
  }
  boostOn() {
    this.rumble(0.55, 1600, 260, 0.04);
    this.tone(95, 0.4, 'sine', 0.05, 0, 40); // low thump under the whoosh
    this.rumble(0.3, 5000, 1800, 0.015, 0, 'bandpass'); // hiss of the afterburner lighting
  }
  /** Nitro runs dry: short fizzle. */
  boostOff() {
    this.rumble(0.35, 2400, 500, 0.018, 0, 'bandpass');
  }

  // ---- weapons ----
  /** Pulse blaster: short, soft, slightly varied so rapid fire doesn't drone. */
  blaster() {
    const f = 620 + Math.random() * 60;
    this.tone(f, 0.075, 'triangle', 0.028, 0, f * 0.45);
    this.tone(f * 2, 0.04, 'sine', 0.008);
  }
  overheat() {
    this.tone(520, 0.4, 'triangle', 0.05, 0, 180);
    this.rumble(0.5, 3000, 600, 0.025, 0, 'bandpass');
  }
  missile() {
    this.rumble(0.7, 400, 2200, 0.06, 0, 'bandpass');
    this.tone(180, 0.35, 'sine', 0.06, 0, 90);
  }
  emp() {
    this.tone(700, 0.7, 'sine', 0.09, 0, 55);
    this.tone(1400, 0.3, 'triangle', 0.025, 0, 300);
    this.rumble(0.9, 2400, 80, 0.07);
  }
  deny() {
    this.tone(220, 0.1, 'triangle', 0.035);
  }
  lockTick() {
    this.tone(1320, 0.04, 'sine', 0.02);
  }
  locked() {
    this.tone(1568, 0.07, 'sine', 0.035);
    this.tone(2093, 0.12, 'sine', 0.03, 0.07);
  }
  /** Rival fire, distance-scaled. */
  zap(vol = 1, pan = 0) {
    this.tone(760, 0.18, 'sine', 0.06 * vol, 0, 240, pan);
  }
  /** Confirmation tick when your shot connects. */
  hitMark(big = false) {
    this.tone(big ? 1760 : 2350, big ? 0.08 : 0.035, 'sine', big ? 0.05 : 0.018);
  }
  hit(k) {
    const v = Math.min(0.16, 0.05 + k * 0.005);
    this.tone(140, 0.25, 'sine', v, 0, 45);
    this.rumble(0.18, 800, 120, v * 0.35);
  }
  damage(vol = 1, pan = 0) {
    this.tone(300, 0.14, 'triangle', 0.035 * vol, 0, 140, pan);
  }
  explosion(vol = 1, pan = 0) {
    this.tone(85, 0.8, 'sine', 0.18 * vol, 0, 28, pan);
    this.rumble(1.0, 1100, 60, 0.12 * vol, pan);
  }
}
