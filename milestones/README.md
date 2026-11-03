# Roadsters Race: road to roadstersrace.com

Follow the milestones in order, and tick the boxes as you go. Each file ends with a **✅ Done when** check.

| # | Milestone | Who | Time | Status |
|---|---|---|---|---|
| [M1](M1-final-polish.md) | Final polish (rules tests, legal pages, quality setting, errors) | Claude (+ you install Java) | 1 session | ✅ |
| [M2](M2-firebase-project.md) | Create the Firebase project | You | 20 min | ✅ |
| [M3](M3-real-signin-locally.md) | Real Google sign-in on your PC | You + Claude | 15 min | ✅ |
| [M4](M4-domain-to-cloudflare.md) | Namecheap → Cloudflare | You | 15 min + wait | ✅ |
| [M5](M5-serve-from-pc.md) | Serve from your PC on roadstersrace.com | You + Claude | 30 min | ✅ |
| [M6](M6-go-live-test.md) | Go-live test → **launch** 🚀 | You + friend | 30 min | 🔄 launched 2026-11-01; the two-account check isn't confirmed yet |
| [M7](M7-running-it.md) | Running the live game | Ongoing | — | 🔄 ongoing (releases, CI, tech debt: `techplan/10-tech-debt-risks.md`) |
| [M8](M8-before-the-shop.md) | Secure coins before the shop | Claude (+ Blaze plan) | 1–2 sessions | ✅ v0.2 (2026-11-01) |

M4 has a waiting period, so you can start it early, alongside M1–M3.

Setup: Namecheap (domain) → Cloudflare (DNS, HTTPS, Tunnel) → your Windows PC (`npm run serve`) and
Firebase (Google sign-in + Firestore for nicknames, coins and the ranking).
