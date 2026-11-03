# Roadsters Race Windows work

- Safety constraint recorded 2026-11-02: the user reports severe lag and a whole-PC freeze after launching `Run-RoadstersRace.cmd`, requiring a restart, and says they will not run it again. Do not launch the game, graphics smoke tests, or the WebAssembly graphics artifact without a new explicit user instruction authorizing that execution. Continue with static inspection and non-graphics work. See `docs/RECOVERY-2026-11-02.md`.
- Read the surrounding project for reference, but make all changes inside `win_app/`.
- Claude owns the browser implementation in `../game/`. Do not edit it or the live deployment.
- Write the Windows game in C, using raylib. Do not substitute Electron, a webview, C++, or a JavaScript game wrapper.
- The Windows application is intended to become the main game implementation.
- First bring the Windows version up to the browser game's agreed feature baseline. Browser development can continue slowly during the port.
- Do not interpret v0.3 as a fixed delivery deadline. See `PROJECT_DIRECTION.md` for confirmed and pending decisions.
- Preserve the existing Roadsters Race art direction and use existing code, models, and documentation as references for the port.
- Match existing browser handling, combat, rules, and visuals first. Defer gameplay redesign and enhanced Windows graphics until after parity.
- Share as much gameplay code as practical in C between Windows and a future WebAssembly browser build. Keep platform services behind interfaces; thin browser JavaScript integration may be needed for browser APIs and services.
- Accounts, persistent game data, purchases, progression, and multiplayer rooms must be shared with browser players. Preserve cross-play as an architecture requirement throughout the port.
- Do not alter shared backend contracts, live protocols, accounts, or economy rules without an agreed integration plan.
- The initial architecture discussion and product choices are recorded. The user authorized preparing the architecture/porting plan and downloading raylib. Follow the milestones and acceptance gates in `docs/ROADMAP.md`; do not claim an unbuilt or untested stage complete.

