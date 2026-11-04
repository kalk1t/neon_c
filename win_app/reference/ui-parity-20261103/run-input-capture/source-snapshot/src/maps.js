/**
 * The map: SKYLINE LOOP. main.js buildWorld builds it from this data.
 *
 * laps:   RACE length
 * floor:  below this height a racer has left the course (SIGNAL LOST); FREE ROAM cars stop at roamFloor
 * atmo:   sky, fog, lights and post-processing (main.js applyAtmosphere, env.js buildSky)
 */
export const MAPS = {
  skyline: {
    id: 'skyline',
    name: 'SKYLINE LOOP',
    roamName: 'SKYLINE CITY',
    laps: 2,
    floor: 34,
    roamFloor: 40,
    atmo: {
      fog: 0x1d1846, fogDensity: 0.00062, exposure: 0.95, bloom: 0.55, env: 0.8,
      hemi: [0x5a50d0, 0x120620, 0.45], sun: [0xa8b4ff, 0.9], rim: [0xff4fd8, 0.6],
      sky: { horizon: [0.3, 0.2, 0.55], mid: [0.07, 0.07, 0.24], zen: [0.008, 0.012, 0.06], band: 0.35, stars: 1, moon: 1, cloud: 0.55, cloudCol: [0.22, 0.18, 0.42] },
      skylineBack: true, cloudSea: true,
    },
  },
};
export const mapById = (id) => MAPS[id] ?? MAPS.skyline;
