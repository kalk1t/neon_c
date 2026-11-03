(function () {
  'use strict';
  var config = window.ROADSTERS_PACK;
  var byId = function (id) { return document.getElementById(id); };
  document.title = config.title + ' — Asset preview';
  byId('pack-title').textContent = config.title;
  byId('pack-subtitle').textContent = 'Selected cockpit artwork with adjustable live race instruments.';
  var ui = window.RoadstersCockpit.mount(byId('stage'), {
    assetBaseUrl: config.assetBaseUrl,
    theme: config.theme,
    layout: config.layout,
    onViewChange: function (view) {
      byId('external-message').hidden = view !== 'external';
      byId('view-label').textContent = view.toUpperCase() + ' VIEW';
    }
  });
  var raceTime = 24170;
  function refresh() {
    var speed = Number(byId('speed').value);
    var boost = Number(byId('boost').value);
    var steering = Number(byId('steering').value);
    var cooldown = Number(byId('cooldown').value);
    byId('speed-output').textContent = speed + ' km/h';
    byId('boost-output').textContent = boost + '%';
    byId('steering-output').textContent = steering + '%';
    byId('cooldown-output').textContent = (cooldown / 1000).toFixed(1) + ' s';
    ui.update({ speedKmh: speed, lap: 1, totalLaps: 3, raceTimeMs: raceTime, boost: boost / 100, steering: steering / 100, attackState: byId('attack-state').value, attackCooldownMs: cooldown });
  }
  var inputs = Array.from(document.querySelectorAll('.controls input, .controls select'));
  inputs.forEach(function (input) { input.addEventListener('input', refresh); });
  refresh();
  var previous = performance.now();
  var frame;
  function tick(now) {
    var elapsed = Math.min(100, now - previous);
    previous = now;
    if (byId('timer-running').checked) { raceTime += elapsed; ui.update({ raceTimeMs: raceTime }); }
    frame = requestAnimationFrame(tick);
  }
  frame = requestAnimationFrame(tick);
  window.addEventListener('pagehide', function () {
    cancelAnimationFrame(frame);
    inputs.forEach(function (input) { input.removeEventListener('input', refresh); });
    ui.destroy();
  }, { once: true });
  // Exposed for manual inspection in the offline preview only.
  window.roadstersPreview = ui;
})();
