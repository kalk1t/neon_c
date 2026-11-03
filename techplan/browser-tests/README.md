# Browser tests (moved)

Since 2026-11-02 the browser tests live in the game repository: `game/tests/browser/` (`killer.test.mjs`,
`void.test.mjs`, `save.test.mjs`, `rivals.test.mjs`, plus `killershot.mjs` for screenshots). Run them with `npm run test:browser` in
`game/`; it starts its own dev server with test logins. GitHub Actions runs them on every push to `main`
(`game/.github/workflows/ci.yml`). This folder used to hold loose copies (finding D13, now fixed).
