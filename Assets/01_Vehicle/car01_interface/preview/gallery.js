(function () {
  'use strict';
  var config = window.ROADSTERS_PACK;
  document.getElementById('gallery-title').textContent = config.title + ' — Assets';
  var items = [
    ['references/cockpit_preview.png', 'Selected cockpit preview', 'Illustrative composition with sample readings.'],
    ['artwork/cockpit_base.png', 'Cockpit base', 'Transparent windshield, blank instrument surfaces, controls separated.'],
    ['panels/speed_panel.png', 'Speed display housing', 'Blank panel for live numeric speed.'],
    ['panels/boost_panel.png', 'Boost gauge housing', 'Blank panel for live charge values.'],
    ['panels/attack_panel.png', 'Attack status housing', 'Blank panel for ready, cooldown and unavailable states.'],
    ['panels/race_panel.png', 'Race information housing', 'Blank panel for lap and race time.'],
    ['ui/icon_boost.png', 'Boost icon', 'Separate transparent PNG.'],
    ['ui/icon_attack.png', 'Attack icon', 'Separate transparent PNG.'],
    ['ui/icon_camera.png', 'Camera icon', 'Separate transparent PNG.'],
    ['ui/icon_warning.png', 'Warning icon', 'Separate transparent PNG.'],
    ['ui/reticle.png', 'Aiming reticle', 'Separate transparent PNG.'],
    ['ui/keycap_i.png', 'I keycap', 'Separate static keycap artwork for the camera-view hint.'],
    ['references/interior_modeling_reference.png', 'Interior modeling reference', 'Design reference for a later 3D cockpit; not a finished mesh.']
  ];
  if (config.id === 'car02') items.splice(2, 0, ['artwork/control_left.png', 'Left control grip', 'Isolated transparent control assembly.'], ['artwork/control_right.png', 'Right control grip', 'Isolated transparent control assembly.']);
  else items.splice(2, 0, ['artwork/control_neutral.png', config.id === 'car01' ? 'Steering yoke' : 'Steering wheel', 'Neutral pose; rotate in the host interface.']);
  var grid = document.getElementById('asset-grid');
  items.forEach(function (item) {
    var card = document.createElement('figure'); card.className = 'asset-card';
    var link = document.createElement('a'); link.href = config.assetBaseUrl + item[0];
    var img = document.createElement('img'); img.src = link.href; img.alt = item[1]; img.loading = 'lazy';
    img.addEventListener('error', function () { card.classList.add('asset-missing'); });
    var caption = document.createElement('figcaption');
    var title = document.createElement('strong'); title.textContent = item[1]; caption.appendChild(title);
    caption.appendChild(document.createTextNode(item[2])); caption.appendChild(document.createElement('br'));
    var filename = document.createElement('code'); filename.textContent = item[0]; caption.appendChild(filename);
    link.appendChild(img); card.appendChild(link); card.appendChild(caption); grid.appendChild(card);
  });
})();
