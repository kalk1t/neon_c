// Shared by the game (src/online/live.js) and the live server (server/live.js).
// Bump LIVE_PROTOCOL whenever the FREE ROAM messages change: a tab still running an older game (opened before an update
// and reconnected on its own) is then told to reload, instead of playing on with code that misses new features
// (e.g. version 1 tabs never saved their kills).
export const LIVE_PROTOCOL = 9; // 3: spin dodge, 4: chat + shooter-decided hits, 5: KILLER STATUS + chat commands, 6: join.kc,
// 7: the relay saves kills and KILLER STATUS (older games still try to save them and would only get errors), and
// every player's shop car (hi.car, welcome / join .car) is shown to the others; 8: err 'banned', filtered names and chat,
// /report and /help (S6); 9: bolts do 21 damage and fly 750 m/s for 1.5 s (v0.2.14: the victim's game applies the damage,
// the relay checks the reach, so old and new games would disagree)
/** Shop cars a player can drive in FREE ROAM (src/online/index.js SHOP_CARS, src/car.js CAR_MODELS); the first is free. */
export const CAR_IDS = ['roadster', 'viper', 'titan'];
/** Nickname format (firestore.rules validNick has the same pattern; tests/constants.test.mjs checks it). */
export const NICK_RE = /^[A-Za-z0-9_-]{3,16}$/;
/** Characters per FREE ROAM chat message: the game stops typing there, the relay cuts longer ones. */
export const CHAT_MAX = 120;
// KILLER STATUS: confirmed FREE ROAM player kills that make a Killer (the relay counts; firestore.rules agrees)
export const KILLER_KILLS = 3;
// Killer kill count that upgrades the aura from the CRIMSON FLAME to the VOID CROWN (killer.js auraStyleFor)
export const VOID_KILLS = 10;
