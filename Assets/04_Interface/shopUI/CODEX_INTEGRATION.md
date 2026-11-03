# Roadsters Race — shop UI handoff

## Scope
Use the six individually sized references in `references/` and the exact owner-approved gold five-spoke wheel coin in `currency/`. Collection A / Neon Circuit is selected. Read `prompts/00_APPROVED_DIRECTION.md` first, then only the source prompt relevant to the component being implemented.

These are reference images and artwork, not implemented application code. Do not replace an interactive screen with one flattened image. Inspect the existing project and its controls, account/wallet state and renderer before editing. Preserve unrelated behavior.

## Visual requirements
Implement the desktop left-side list, phone bottom card strip, empty live 3D viewport, five card states, purchase confirmation/success panels, and SHOP below WORLD RANKING. Use dark navy, cyan outlines, magenta primary actions, and gold prices/wallet; the menu button has the explicit gold-border exception. Keep the mobile outer 4% clear and real interactive targets at least 48 CSS pixels tall. The PNG canvas dimensions alone do not guarantee target size after responsive scaling.

Keep actual 3D car rendering separate from UI layers. The placeholder thumbnail is not a finished car model. No extra cars, prices, stats, weapons or performance upgrades are authorized by this pack. VIPER is cosmetic: same speed and weapons. Use the approved wheel coin wherever the source prompt uses a placeholder diamond.

## State handling
The supplied reference is not a coherent single transaction session: the shop example uses wallet 650 and VIPER 2,500; the confirmation example uses wallet 3,100 and ends at 600. Use current application state, never screenshot values. The source shows ROADSTER equipped/selected while VIPER appears in details. Model equipped and previewed selection independently; do not silently change saved equipment on preview.

Reject purchases with insufficient balance; show the live shortfall (1,850 in the source example). Prevent duplicate purchases, revalidate price and balance in the authoritative purchase path, deduct and grant ownership atomically, and show success only after confirmed completion. EQUIP NOW changes equipment only after successful ownership validation. LATER does not equip. COMING SOON is non-purchasable. Preserve existing persistence and authentication conventions.

## Acceptance
Check desktop and landscape phone layouts, all five card states, exactly one equipped car, affordable and unaffordable states, confirmation/cancel, equip/later, errors and repeated clicks, keyboard focus and dismissal, touch drag versus card selection, and real coin transparency over navy. Check at actual rendered sizes rather than only looking at high-resolution references. Report what was implemented, tested, and still pending without claiming runtime integration from these images alone.
