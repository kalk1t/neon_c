# M1 · Final polish before launch

**Who:** Claude (you install Java once) · **Time:** about 1 session · **Needs:** nothing · **Next:** [M2](M2-firebase-project.md)

## Goal
The game and its security rules are ready for strangers to play: tested, legally covered, and smooth on normal
laptops.

## Steps

### 1. Install Java (You, 5 min)
Firebase's local test tools (the emulator) need Java, which lets Claude test the database rules without a real
project.
- [x] Download **Temurin 21 (LTS)** for Windows x64 (`.msi`) from https://adoptium.net
- [x] Run it with the defaults. Make sure **"Set JAVA_HOME"** and **"Add to PATH"** are on.
- [x] Open a new terminal → `java -version` prints `21.x` or newer (installed: Temurin 25.0.5). Tell Claude "java done".

### 2. Database rules tests (Claude)
- [x] Install `firebase-tools` and `@firebase/rules-unit-testing` as dev dependencies.
- [x] Write `tests/rules.test.mjs` covering:
  - [x] creating a profile with a free nickname works; taking someone else's nickname (any capitals) fails
  - [x] starting with coins > 0, or editing another player's profile, fails
  - [x] a race award of 0–200 works; above 200, negative, decimals, or lowering coins fails
  - [x] a second race within 120 s fails; `bestLap` under 30 s, or getting worse, fails
  - [x] renaming releases the old nickname; the ranking and nicknames are readable by anyone
- [x] Add `npm run test:rules` and fix any rule that fails. → **26/26 passed**, no rule changes needed.

### 3. Legal pages and contact (Claude)
- [x] `public/privacy.html`: what is stored (public: nickname and coin stats; private: Google email and name, visits,
      device), Google Analytics only with consent, Firebase and Cloudflare as processors, account deletion by email.
- [x] `public/terms.html`: free game, no cheating, nicknames must not be offensive, accounts can be removed, coins
      have no real-money value.
- [x] **help@roadstersrace.com** for bugs, ideas, feedback, collaborations and business offers, shown in a contact box
      in the menu, on the pause screen, in the phone notice, and at the top of both legal pages.
- [x] PRIVACY · TERMS · ANALYTICS SETTINGS links in the menu footer.

### 3b. Ranking, player info, traffic (Claude, added 30 Oct)
- [x] The World Ranking shows the **top 10**. Players outside it see "YOUR POSITION: #23" below the list.
- [x] Private `players/{uid}` record for signed-in players: Google email and name, first and last visit, visit count,
      language, time zone, device, screen size, referring site. Only the player and you (in the Firebase console)
      can read it. 6 new rules tests, **32/32 pass**.
- [x] Google Analytics events (`race_start`, `race_finish`, `login`, `sign_up`, `ranking_open`, coins earned), sent only
      after the visitor clicks ALLOW on a consent banner. The choice can be changed in the footer.
- [x] Cookieless visit counts from Cloudflare Web Analytics: switched on in M4, no code needed.

### 4. Share image (Claude)
- [x] A 1200×630 `public/og-image.jpg` taken from the game, plus `og:image`, `og:url` and `twitter:card` meta tags.

### 5. Graphics quality (Claude)
- [x] A **Quality: High / Low** setting in the menu, saved in the browser. Low means lower resolution, fewer
      effects, no extra glow passes and fewer city lights.
- [x] Measure frame rate on Low vs High in the test browser.

### 6. Friendly errors (Claude)
- [x] No WebGL → a "your browser/graphics can't run the game" screen.
- [x] Sign-in popup blocked or closed → a clear message, with a retry using redirect.
- [x] Firebase unreachable or offline → the game still plays, with a "not saved, offline" note.

### 7. Final playtest (Claude)
- [x] An automated race in Chrome: every weapon, coins, results, restart, pause, volume. No console errors.
- [x] Check menu, ranking and results screenshots at 1366×768 and 1920×1080.

### 8. Extra polish (Claude, 30 Oct)
- [x] Google sign-in can run through roadstersrace.com: the server passes `/__/auth/*` through to Firebase. It's switched
      on once the redirect URI is added (M2 step 5).
- [x] Saving a race times out after 15 s with a TRY AGAIN button if the connection drops. Sign-in errors are explained in
      plain language.
- [x] Logo files for the Google screen: `public/brand/logo-120.png`, `logo-512.png`
- [x] `server/restart-server.ps1` puts a new version live (run as admin)

## ✅ Done when (completed 30 Oct: 32/32 rules tests, build OK, race at 60 fps with no errors, online flow OK)
- `npm run test:rules` passes, `npm run build` has no errors, and the playtest is clean with no console errors.

## If something goes wrong
- `java` not found → close and reopen the terminal, or reinstall with "Add to PATH" ticked.
