# Roadsters Race — Neon Circuit UI

## Final individual reference images

- `references/28_shop_screen_desktop.png` — 1920 × 1080.
- `references/29_shop_screen_phone.png` — 2340 × 1080, no phone frame.
- `references/30_shop_car_card_states.png` — 1600 × 900, five states in one row.
- `references/31_shop_buy_dialog.png` — 1200 × 800, confirmation above success.
- `references/32_shop_menu_button.png` — 1200 × 500, normal / hover / NEW.
- `references/33_coin_currency_icon.png` — 1024 × 1024, real RGBA transparency and approximately 15% transparent padding.

The currency is the exact owner-selected **second coin, the gold five-spoke racing wheel**. No wing, crown, R-emblem, diamond or alternate coin is approved in this package.

## Additional files

`components/` contains separate transparent button, card, modal, menu, wallet, placeholder and icon PNGs, plus the empty stage. Each transparent component's content rectangle and padding are listed in `manifest.json`. `currency/` includes the approved original and small UI sizes. `prompts/` preserves the six original Markdown files unchanged, with a separate approved-direction override. `theme.css` contains color/font-family tokens, not application logic. `index.html` is an offline reference gallery only.

## Scope and source fidelity

These are individual raster UI references and reusable artwork, not a working shop, a 3D car, or a tested production implementation. Use live application text, prices, balances, focus states, purchases and selected/equipped state rather than hard-coding entire screenshot images into the game. No font files are bundled. The rendered reference typography uses available Inter fonts as a visual substitute; the supplied prompts request Orbitron/Rajdhani-like typography.

The source screenshots show ROADSTER equipped/selected while VIPER appears in the detail area. That combination is intentionally preserved instead of silently resolving it. The source BUY display is likewise retained with a 650 wallet and 2,500 price; actual purchase validation must reject the 1,850 shortfall. `button_buy_price_disabled.png` is supplied as a clearly supplemental implementation state. The separate confirmation example has a 3,100 wallet, so its 600 post-purchase balance is valid.

The exact selected coin supersedes the original file 33's hexagon / R / chevron description. Its approved perspective is preserved rather than redesigning the artwork. Main stage areas contain no car, no performance-stat bars, no city and no added gameplay upgrade claims.

## Quality checks

All six reference sizes and readable text placements were checked in the build. Coin corner pixels are fully transparent, and small-size variants use the same approved artwork. Final file sizes, modes and SHA-256 hashes are recorded in the manifest.
