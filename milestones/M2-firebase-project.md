# M2 · Create the Firebase project

**Who:** You (Claude guides) · **Time:** about 20 min · **Needs:** M1, a Google account · **Next:** [M3](M3-real-signin-locally.md)

## Goal
A Firebase project that runs Google sign-in and stores nicknames, coins and the world ranking.
The free **Spark** plan is enough, so no credit card is needed.

## Steps

### 1. Create the project
- [ ] Go to https://console.firebase.google.com → **Create a project**
- [ ] Name: `roadsters-race`. Note the **project ID** shown under it (e.g. `roadsters-race` or `roadsters-race-1a2b3`).
- [ ] Google Analytics: **on** (the game uses it for traffic and gameplay stats) → create a new GA account
      `Roadsters Race` → choose your country → **Create project**

### 2. Turn on Google sign-in
- [x] Left menu **Build → Authentication → Get started**
- [x] **Sign-in method → Google → Enable**
- [x] Public-facing name: `Roadsters Race` · Support email: your Gmail → **Save**

### 3. Create the database
- [x] **Build → Firestore Database → Create database**
- [x] Location: closest to your players, e.g. `eur3 (europe-west)` or `nam5 (us-central)`. **This can't be changed later.**
- [x] Start in **production mode** → Create.
      → done 30 Oct: Standard edition, `(default)`, location **nam5 (United States)**. The real rules are uploaded in M3.

### 4. Register the web app
- [x] Gear icon ⚙ → **Project settings → General → Your apps → `</>` (Web)**
- [x] Nickname: `roadsters-web`. Leave "Firebase Hosting" unticked → **Register app**
- [x] Copy these 5 values from `firebaseConfig` and paste them to Claude:
  ```
  apiKey: "..."
  authDomain: "....firebaseapp.com"
  projectId: "..."
  appId: "..."
  measurementId: "G-..."
  ```
  If `measurementId` is missing: Project settings → **Integrations → Google Analytics → Enable**, then copy it again.
  These values are public by design (every Firebase website exposes them), so sharing them is safe. Claude puts them
  in `game/.env.local`.

### 5. Brand the Google sign-in screen
**a) Branding:** https://console.cloud.google.com/auth/branding?project=roadsters-race
- [x] App name `Roadsters Race` · User support email: your Gmail
- [ ] App logo: **skip for now**. Adding a logo makes Google require brand verification (days). It can be added later.
- [x] App home page `https://roadstersrace.com` · Privacy policy `https://roadstersrace.com/privacy.html` ·
      Terms of service `https://roadstersrace.com/terms.html`
- [x] Authorized domains: `roadstersrace.com` (keep `roadsters-race.firebaseapp.com` if it's listed) · Developer
      contact: your Gmail → **Save**

**b) Audience:** https://console.cloud.google.com/auth/audience?project=roadsters-race
- [x] User type **External**. If the status says *Testing*, click **Publish app** → **In production**. The basic
      sign-in scopes (name, email) need no Google review.

**c) Let sign-in run on roadstersrace.com:** https://console.cloud.google.com/auth/clients?project=roadsters-race
- [x] Open **Web client (auto created by Google Service)**
- [x] **Authorized JavaScript origins → Add URI** → `https://roadstersrace.com`
- [x] **Authorized redirect URIs → Add URI** → `https://roadstersrace.com/__/auth/handler` → **Save**
- [x] Tell Claude, who then switches `VITE_FIREBASE_AUTH_DOMAIN` to `roadstersrace.com` → you run `restart-server.ps1`
      → verified 30 Oct: the live popup says "Sign in to continue to roadstersrace.com" and links to the privacy policy and terms

### 6. Analytics settings (2 min)
- [ ] Open https://analytics.google.com → Admin (gear) → **Data collection and modification → Data retention** →
      **14 months** → Save. This matches the privacy policy.

## ✅ Done when
- Authentication shows **Google: Enabled**, the Firestore Database page shows an empty database, and Claude has the
  5 config values.

## If something goes wrong
- Until the logo/branding is reviewed, the Google sign-in popup may say "continue to roadsters-race.firebaseapp.com".
  That's normal and doesn't stop anything working.
