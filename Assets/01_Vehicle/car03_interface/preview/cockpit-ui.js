/* Roadsters Race cockpit overlay. No dependencies, network calls, or game camera assumptions. */
(function (global) {
  'use strict';

  var THEMES = {
    car01: { id: 'car01', name: 'Car01 · Flight Deck', primary: '#35e5ff', secondary: '#ff3bdd' },
    car02: { id: 'car02', name: 'Car02 · Twin Controls', primary: '#26e6ff', secondary: '#ed3dcc' },
    car03: { id: 'car03', name: 'Car03 · Armored Sport', primary: '#ff38d2', secondary: '#31e3f3' }
  };
  var ICON_CONTENT = {
    'ui/icon_boost.png': [268/1254, 258/1254, 717/1254, 750/1254],
    'ui/icon_attack.png': [82/1254, 158/1254, 1091/1254, 940/1254],
    'ui/icon_camera.png': [214/1254, 238/1254, 838/1254, 771/1254],
    'ui/icon_warning.png': [180/1254, 240/1254, 894/1254, 775/1254],
    'ui/reticle.png': [400/1254, 395/1254, 454/1254, 456/1254]
  };

  // Coordinates are normalized to the fitted art canvas, not the whole browser window.
  // These positions were measured against the delivered base PNGs. Tune in the host game.
  var LAYOUTS = {
    car01: {
      aspectRatio: [1672, 941],
      base: { box: [0, 0, 1, 1], path: 'artwork/cockpit_base.png' },
      controls: [{ path: 'artwork/control_neutral.png', box: [0.25, 0.61, 0.50, 0.486], contentBox: [104/1671, 83/941, 1462/1671, 800/941], rotation: 16, origin: '50% 58%' }],
      panels: {
        speed: { box: [0.438, 0.517, 0.124, 0.079], path: null, textBox: [0, 0, 1, 1], fontScale: 1.0 },
        boost: { box: [0.264, 0.544, 0.073, 0.09], path: null, textBox: [0, 0, 1, 1], fontScale: 1.2 },
        attack: { box: [0.67, 0.54, 0.073, 0.10], path: null, textBox: [0, 0, 1, 1], fontScale: 1.1 },
        race: { box: [0.432, 0.611, 0.14, 0.029], path: null, textBox: [0, 0, 1, 1], fontScale: 1.15 }
      },
      reticle: { box: [0.486, 0.365, 0.028, 0.05], path: 'ui/reticle.png' }
    },
    car02: {
      aspectRatio: [1672, 941],
      base: { box: [0, 0, 1, 1], path: 'artwork/cockpit_base.png' },
      controls: [
        { path: 'artwork/control_left.png', box: [0.18, 0.59, 0.104, 0.26], contentBox: [240/1254, 87/1254, 774/1254, 1092/1254], rotation: 8, origin: '50% 90%' },
        { path: 'artwork/control_right.png', box: [0.716, 0.59, 0.108, 0.26], contentBox: [266/1254, 77/1254, 821/1254, 1115/1254], rotation: 8, origin: '50% 90%' }
      ],
      panels: {
        speed: { box: [0.416, 0.463, 0.168, 0.085], path: null, textBox: [0, 0, 1, 1], fontScale: 1.05 },
        boost: { box: [0.654, 0.49, 0.102, 0.069], path: null, textBox: [0, 0, 1, 1], fontScale: 1.1 },
        attack: { box: [0.65, 0.563, 0.12, 0.027], path: null, textBox: [0, 0, 1, 1], fontScale: 1.05 },
        race: { box: [0.263, 0.506, 0.10, 0.065], path: null, textBox: [0, 0, 1, 1], fontScale: 1.4 }
      },
      reticle: { box: [0.486, 0.365, 0.028, 0.05], path: 'ui/reticle.png' }
    },
    car03: {
      aspectRatio: [1672, 941],
      base: { box: [0, 0, 1, 1], path: 'artwork/cockpit_base.png' },
      controls: [{ path: 'artwork/control_neutral.png', box: [0.215, 0.645, 0.57, 0.45], contentBox: [17/1254, 228/1254, 1220/1254, 849/1254], rotation: 16, origin: '50% 39%' }],
      panels: {
        speed: { box: [0.435, 0.537, 0.13, 0.086], path: null, textBox: [0, 0, 1, 1], fontScale: 1.1 },
        boost: { box: [0.617, 0.536, 0.08, 0.095], path: null, textBox: [0, 0, 1, 1], fontScale: 1.2 },
        attack: { box: [0.768, 0.628, 0.104, 0.103], path: null, textBox: [0, 0, 1, 1], fontScale: 1.1 },
        race: { box: [0.430, 0.492, 0.14, 0.03], path: null, textBox: [0, 0, 1, 1], fontScale: 1.15 }
      },
      reticle: { box: [0.486, 0.365, 0.028, 0.05], path: 'ui/reticle.png' },
      decorations: [{ box: [0.314, 0.528, 0.06, 0.107], path: 'ui/reticle.png', opacity: 0.45 }]
    }
  };

  function finite(value, fallback) {
    return typeof value === 'number' && Number.isFinite(value) ? value : fallback;
  }
  function clamp(value, min, max) { return Math.min(max, Math.max(min, value)); }
  function copy(value) { return JSON.parse(JSON.stringify(value)); }
  function mergeLayout(base, override) {
    var next = copy(base);
    if (!override) return next;
    Object.keys(override).forEach(function (key) {
      if (key === 'panels') {
        Object.keys(override.panels).forEach(function (name) {
          next.panels[name] = Object.assign({}, next.panels[name], override.panels[name]);
        });
      } else if (key === 'base' || key === 'reticle') {
        next[key] = Object.assign({}, next[key], override[key]);
      } else { next[key] = copy(override[key]); }
    });
    return next;
  }
  function timeString(ms) {
    var total = Math.max(0, Math.floor(ms / 10));
    var hundredths = total % 100;
    var seconds = Math.floor(total / 100) % 60;
    var minutes = Math.floor(total / 6000);
    return String(minutes).padStart(2, '0') + ':' + String(seconds).padStart(2, '0') + '.' + String(hundredths).padStart(2, '0');
  }
  function editableTarget(target) {
    return !!(target && target.closest && target.closest('input, textarea, select, button, [contenteditable]:not([contenteditable="false"])'));
  }

  function mount(root, options) {
    options = options || {};
    if (!root || !root.ownerDocument || root.nodeType !== 1) throw new TypeError('mount requires a DOM element');
    var doc = root.ownerDocument;
    var win = doc.defaultView || global;
    var theme = typeof options.theme === 'object' ? Object.assign({}, THEMES.car01, options.theme) : THEMES[options.theme] || THEMES.car01;
    var layout = mergeLayout(LAYOUTS[theme.id] || LAYOUTS.car01, options.layout);
    var baseUrl = String(options.assetBaseUrl || './').replace(/\/?$/, '/');
    var state = { speedKmh: 0, lap: 1, totalLaps: 3, raceTimeMs: 0, boost: 1, attackState: 'ready', attackCooldownMs: 0, steering: 0, view: options.initialView === 'external' ? 'external' : 'cockpit' };
    var disposed = false;
    var imageErrors = new Set();
    var controlNodes = [];
    var imageListeners = [];
    var hadRootClass = root.classList.contains('rr-cockpit');
    root.classList.add('rr-cockpit');

    var layer = doc.createElement('div');
    layer.className = 'rr-cockpit__layer rr-cockpit--' + theme.id;
    layer.style.setProperty('--rr-primary', theme.primary);
    layer.style.setProperty('--rr-secondary', theme.secondary);
    layer.setAttribute('aria-label', theme.name + ' instruments');
    var canvas = doc.createElement('div');
    canvas.className = 'rr-cockpit__canvas';
    layer.appendChild(canvas);

    function setBox(node, box) {
      node.style.left = box[0] * 100 + '%';
      node.style.top = box[1] * 100 + '%';
      node.style.width = box[2] * 100 + '%';
      node.style.height = box[3] * 100 + '%';
    }
    function imageNode(path, parent, contentBox) {
      var holder = doc.createElement('div');
      holder.className = 'rr-cockpit__image-holder';
      var img = doc.createElement('img');
      img.src = baseUrl + path;
      img.alt = '';
      img.draggable = false;
      img.decoding = 'async';
      if (contentBox && contentBox[2] > 0 && contentBox[3] > 0) {
        // Fit the supplied alpha content rectangle without rewriting the PNG.
        holder.classList.add('rr-cockpit__image-holder--trimmed');
        img.style.width = 100 / contentBox[2] + '%';
        img.style.height = 100 / contentBox[3] + '%';
        img.style.left = -100 * contentBox[0] / contentBox[2] + '%';
        img.style.top = -100 * contentBox[1] / contentBox[3] + '%';
      }
      var onError = function () {
        if (disposed) return;
        imageErrors.add(path);
        holder.classList.add('rr-cockpit__image-holder--missing');
        holder.title = 'Missing asset: ' + path;
        if (typeof options.onAssetError === 'function') options.onAssetError(path);
      };
      img.addEventListener('error', onError);
      imageListeners.push([img, onError]);
      holder.appendChild(img);
      parent.appendChild(holder);
      return holder;
    }
    function art(config, className) {
      var el = doc.createElement('div');
      el.className = 'rr-cockpit__art ' + (className || '');
      setBox(el, config.box);
      if (config.path) imageNode(config.path, el, config.contentBox || ICON_CONTENT[config.path]);
      canvas.appendChild(el);
      return el;
    }
    art(layout.base, 'rr-cockpit__base');
    layout.controls.forEach(function (config) {
      var el = art(config, 'rr-cockpit__control');
      el.style.transformOrigin = config.origin || '50% 50%';
      controlNodes.push({ element: el, degrees: config.rotation || 0 });
    });
    if (layout.reticle && layout.reticle.path) art(layout.reticle, 'rr-cockpit__reticle');
    (layout.decorations || []).forEach(function (config) {
      var el = art(config, 'rr-cockpit__decoration');
      if (typeof config.opacity === 'number') el.style.opacity = String(config.opacity);
    });

    function panel(name) {
      var config = layout.panels[name];
      var el = art(config, 'rr-cockpit__panel rr-cockpit__panel--' + name);
      var content = doc.createElement('div');
      content.className = 'rr-cockpit__panel-content';
      if (config.textBox) setBox(content, config.textBox);
      if (config.fontScale) content.style.fontSize = config.fontScale + 'em';
      el.appendChild(content);
      return content;
    }
    function span(parent, className, text) {
      var el = doc.createElement('span');
      el.className = className;
      if (text) el.textContent = text;
      parent.appendChild(el);
      return el;
    }
    function icon(parent, path) {
      var wrap = doc.createElement('span');
      wrap.className = 'rr-cockpit__icon';
      if (ICON_CONTENT[path]) wrap.style.setProperty('--rr-icon-ratio', String(ICON_CONTENT[path][2] / ICON_CONTENT[path][3]));
      imageNode(path, wrap, ICON_CONTENT[path]);
      parent.appendChild(wrap);
      return wrap;
    }
    var speedPanel = panel('speed');
    var speedValue = span(speedPanel, 'rr-cockpit__speed', '0');
    span(speedPanel, 'rr-cockpit__unit', 'KM/H');
    var boostPanel = panel('boost');
    var boostLabel = span(boostPanel, 'rr-cockpit__label', 'BOOST');
    icon(boostLabel, 'ui/icon_boost.png');
    var boostValue = span(boostPanel, 'rr-cockpit__value', '100%');
    var boostTrack = span(boostPanel, 'rr-cockpit__meter');
    var boostFill = span(boostTrack, 'rr-cockpit__meter-fill');
    boostTrack.setAttribute('role', 'progressbar');
    boostTrack.setAttribute('aria-label', 'Boost charge');
    boostTrack.setAttribute('aria-valuemin', '0');
    boostTrack.setAttribute('aria-valuemax', '100');
    var attackPanel = panel('attack');
    var attackIcon = icon(attackPanel, 'ui/icon_attack.png');
    var warningIcon = icon(attackPanel, 'ui/icon_warning.png');
    warningIcon.hidden = true;
    span(attackPanel, 'rr-cockpit__label', 'ATTACK');
    var attackValue = span(attackPanel, 'rr-cockpit__attack-value', 'READY');
    var racePanel = panel('race');
    var lapValue = span(racePanel, 'rr-cockpit__lap', 'LAP 1/3');
    var timerValue = span(racePanel, 'rr-cockpit__time', '00:00.00');

    var toggle = doc.createElement('button');
    toggle.type = 'button';
    toggle.className = 'rr-cockpit__toggle';
    toggle.setAttribute('aria-keyshortcuts', 'I');
    icon(toggle, 'ui/icon_camera.png');
    var key = doc.createElement('kbd');
    key.textContent = 'I';
    toggle.appendChild(key);
    span(toggle, '', 'SWITCH VIEW');
    layer.appendChild(toggle);
    root.appendChild(layer);

    function fit() {
      if (disposed) return;
      var width = root.clientWidth;
      var height = root.clientHeight;
      var ratio = layout.aspectRatio[0] / layout.aspectRatio[1];
      var fittedWidth = Math.min(width, height * ratio);
      var fittedHeight = fittedWidth / ratio;
      canvas.style.width = fittedWidth + 'px';
      canvas.style.height = fittedHeight + 'px';
      canvas.style.fontSize = Math.max(7, fittedWidth / 100) + 'px';
    }
    function render() {
      if (disposed) return;
      speedValue.textContent = String(Math.round(state.speedKmh));
      lapValue.textContent = 'LAP ' + state.lap + '/' + state.totalLaps;
      timerValue.textContent = timeString(state.raceTimeMs);
      var percentage = Math.round(state.boost * 100);
      boostValue.textContent = percentage + '%';
      boostFill.style.width = percentage + '%';
      boostTrack.setAttribute('aria-valuenow', String(percentage));
      boostPanel.dataset.charge = percentage === 0 ? 'empty' : percentage === 100 ? 'full' : 'partial';
      attackPanel.dataset.state = state.attackState;
      attackValue.textContent = state.attackState === 'ready' ? 'READY' : state.attackState === 'disabled' ? 'UNAVAILABLE' : (Math.max(0, state.attackCooldownMs) / 1000).toFixed(1) + 's';
      attackIcon.hidden = state.attackState === 'disabled';
      warningIcon.hidden = state.attackState !== 'disabled';
      canvas.hidden = state.view !== 'cockpit';
      layer.dataset.view = state.view;
      toggle.setAttribute('aria-label', 'Switch to ' + (state.view === 'cockpit' ? 'external' : 'cockpit') + ' view');
      toggle.setAttribute('aria-pressed', String(state.view === 'cockpit'));
      controlNodes.forEach(function (control) { control.element.style.transform = 'rotate(' + state.steering * control.degrees + 'deg)'; });
    }
    function setView(view) {
      if (disposed) return;
      if (view !== 'cockpit' && view !== 'external') throw new TypeError('view must be cockpit or external');
      if (state.view === view) return;
      state.view = view;
      render();
      if (typeof options.onViewChange === 'function') options.onViewChange(view);
    }
    function toggleView() { setView(state.view === 'cockpit' ? 'external' : 'cockpit'); }
    function onKey(event) {
      if (disposed || options.keyboard === false || event.defaultPrevented || event.repeat || event.altKey || event.ctrlKey || event.metaKey || editableTarget(event.target)) return;
      if (String(event.key).toLowerCase() !== 'i' && event.code !== 'KeyI') return;
      event.preventDefault();
      toggleView();
    }
    function update(data) {
      if (disposed || !data || typeof data !== 'object') return;
      state.speedKmh = Math.max(0, finite(data.speedKmh, state.speedKmh));
      state.lap = Math.max(1, Math.floor(finite(data.lap, state.lap)));
      state.totalLaps = Math.max(1, Math.floor(finite(data.totalLaps, state.totalLaps)));
      state.raceTimeMs = Math.max(0, finite(data.raceTimeMs, state.raceTimeMs));
      state.boost = clamp(finite(data.boost, state.boost), 0, 1);
      state.attackCooldownMs = Math.max(0, finite(data.attackCooldownMs, state.attackCooldownMs));
      state.steering = clamp(finite(data.steering, state.steering), -1, 1);
      if (data.attackState === 'ready' || data.attackState === 'cooldown' || data.attackState === 'disabled') state.attackState = data.attackState;
      render();
    }
    var resizeObserver = typeof win.ResizeObserver === 'function' ? new win.ResizeObserver(fit) : null;
    if (resizeObserver) resizeObserver.observe(root);
    else win.addEventListener('resize', fit);
    doc.addEventListener('keydown', onKey);
    toggle.addEventListener('click', toggleView);
    fit();
    render();
    if (typeof options.onViewChange === 'function') options.onViewChange(state.view);

    return {
      update: update,
      setView: setView,
      getState: function () { return Object.assign({}, state); },
      getMissingAssets: function () { return Array.from(imageErrors); },
      destroy: function () {
        if (disposed) return;
        disposed = true;
        doc.removeEventListener('keydown', onKey);
        toggle.removeEventListener('click', toggleView);
        if (resizeObserver) resizeObserver.disconnect();
        else win.removeEventListener('resize', fit);
        imageListeners.forEach(function (item) { item[0].removeEventListener('error', item[1]); });
        layer.remove();
        if (!hadRootClass) root.classList.remove('rr-cockpit');
      }
    };
  }

  global.RoadstersCockpit = { mount: mount, themes: THEMES, layouts: LAYOUTS, formatRaceTime: timeString };
})(typeof window !== 'undefined' ? window : globalThis);
