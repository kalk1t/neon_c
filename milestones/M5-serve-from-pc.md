# M5 · Serve the game from your PC on roadstersrace.com

**Who:** You + Claude · **Time:** about 30 min · **Needs:** M3 and M4 · **Next:** [M6](M6-go-live-test.md)

## Goal
https://roadstersrace.com shows the game from your PC, with HTTPS and without opening router ports or exposing your
home IP address. It comes back by itself after a reboot.

## Steps

### 1. Production server (You)
- [x] In `C:\Users\User\Projects\neon_c\game`: `npm run serve`
- [x] http://localhost:8080 shows the game, and Google sign-in works there too

### 2. Create the Cloudflare Tunnel (You)
- [x] Cloudflare dashboard → **Zero Trust** (first visit: pick a team name, Free plan; a card may be requested but
      isn't charged) → **Networks → Tunnels → Create a tunnel**
- [x] Type **Cloudflared** → name `roadsters-pc` → Save
- [x] Environment **Windows** → copy the command → run it in **PowerShell as Administrator**
      (`cloudflared.exe service install <token>`). This installs a Windows service that starts on boot.
- [x] The dashboard shows the connector as **Connected** (HEALTHY)

### 3. Point the domain at the PC (You)
In the tunnel → **Public Hostname → Add a public hostname**:
- [x] Subdomain *(empty)* · Domain `roadstersrace.com` · Type `HTTP` · URL `localhost:8080` → Save
- [x] Add another: subdomain `www` · Domain `roadstersrace.com` · Type `HTTP` · URL `localhost:8080` → Save

### 4. Allow the domain in Firebase (You)
- [x] Firebase console → **Authentication → Settings → Authorized domains → Add domain**
      `roadstersrace.com`, then add `www.roadstersrace.com` too

### 5. Start on boot (Claude made it, You install it once)
- [x] `game/server/start-server.cmd` builds the game and serves it on port 8080, logging to `game/server/server.log`
- [x] Open **PowerShell as Administrator** (Start → type `powershell` → right-click → Run as administrator) and run:
      `powershell -ExecutionPolicy Bypass -File C:\Users\User\Projects\neon_c\game\server\install-autostart.ps1`
- [x] Windows Task Scheduler now has **"Roadsters Race server"**: it starts at boot (no login needed) and restarts within
      1 minute if it stops
- [x] Tunnel public hostnames `roadstersrace.com` and `www.roadstersrace.com` → `localhost:8080` (not 5173)
- [x] Cloudflare → roadstersrace.com → **Rules → Redirect Rules → Create rule → template "Redirect from WWW to root"**
      → Deploy, so players always end up on one address
      → verified 30 Oct: the task is Running, the Cloudflared service is Automatic, `www` → 301 → `roadstersrace.com`
        (query string kept), and the site serves the production build from port 8080

**After updating the game:** in an **administrator** PowerShell run
`powershell -ExecutionPolicy Bypass -File C:\Users\User\Projects\neon_c\game\server\restart-server.ps1`
(or restart the PC). Ending the task in Task Scheduler is not enough, because the server process keeps running.

### 6. Keep the PC awake (You)
- [x] Settings → System → Power → **Screen and sleep**: "When plugged in, put my device to sleep after" → **Never**
      (turning off the screen is fine)
- [ ] Windows Update → Advanced options → **Active hours** set to the hours people play most

## ✅ Done when
- On your phone **with Wi-Fi off**, https://roadstersrace.com loads the game with a padlock and sign-in works.

## If something goes wrong
- **Error 1033 / 502** → the tunnel or server isn't running: check that `npm run serve` is active and the connector
  shows Connected. The Windows `cloudflared` service holds only one tunnel, and another project (requestyoursoftware.com)
  took it over, so `roadsters-pc` runs as its own task: `game/server/install-tunnel.ps1 -Token <token>`.
- **"Blocked request. This host is not allowed"** → the domain is missing from `vite.config.js` → `preview.allowedHosts`.
- **auth/unauthorized-domain** → step 4 is missing.
