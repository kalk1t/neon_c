# 7. Progress

✅ **Done** · [Manual contents](README.md) · Previous: [Free Roam online](06-free-roam.md) · Next: [Settings](08-settings.md)

## Coins
Gold coins float in short zig-zag strings between the checkpoint rings, **6 per gap, 54 per lap**. Coins appear in
**RACE** only (not in Time Attack or Free Roam).

- Fly through a coin to take it. **Rivals collect coins too**, and the first car to touch one gets it.
- A taken coin comes back after **6 seconds**, so later laps still have coins.
- The coin panel (top left) shows your coins and your place in the coin race, e.g. **17 · 2/11**.
- Rivals swerve toward coins just ahead of them.
- A **takedown steals 25%** of the wrecked rival's coins.

### Coin race bonus
At the finish, everyone is ranked by coins collected. The top three get a bonus: **1st +15, 2nd +8, 3rd +4**.

## Wallet
When you're **signed in**, the coins from each race (plus the bonus) are saved to your account:

- **WALLET**: coins you have to spend in the [shop](#shop).
- **Lifetime coins**: everything you've ever earned. This is what the coin ranking counts, so spending coins in the
  shop never lowers your rank.

Saving rules (the game's server checks every race):
- At most **200 coins** per race are saved.
- Races are saved at most once a **minute**. Finish two races faster than that and the second one waits:
  *RACES TOO CLOSE TOGETHER · SAVING IN 30 S* counts down, then it's saved. Stay on the results screen until it says
  SAVED (starting another race first loses it).
- Coins are only saved from a **RACE** that really lasted at least **60 seconds**. TIME ATTACK saves your best lap, no coins.
- **Sign in before the race.** The server has to see a race start to save it. A race started signed out shows
  *N COINS EARNED · NOT SAVED* and a **SIGN IN TO SAVE YOUR NEXT RACE** button.
- If the connection fails, a **TRY AGAIN** button keeps the race until it's saved.
- *NOT SAVED · THE SERVER DIDN'T SEE THIS RACE START* means the connection dropped as the race began.
  *NOT SAVED · RACE TOOK TOO LONG* means more than 45 minutes passed between the start and the finish.
- Without signing in, coins aren't kept. *OFFLINE, NOT SAVED* means online play isn't available right now.

## Shop
Click **SHOP** on the main menu to spend your wallet on new cars. Since 0.2.17 each car **flies and fights differently**:
the shop shows its stats (bars for top speed, acceleration, handling and armor) and its traits. No car is simply
better: each is good at something and weaker at something else. The stats count in RACE and in FREE ROAM.

| | ROADSTER (free) | VIPER (2,500) | TITAN (7,500) |
|---|---|---|---|
| Role | All-rounder | Fast and agile, fragile | Heavy tank, slower |
| Top speed | 135 m/s · 486 km/h | 146 m/s · 525 km/h (+8%) | 127 m/s · 457 km/h (−6%) |
| To 90% of top speed | 3.9 s | 3.3 s | 4.6 s |
| Handling (turn rate) | 100% | 112% | 90% |
| Health | 100 | 75 (4 bolts wreck it) | 140 (7 bolts) |
| Knock-back from hits | normal | ×1.3 | ×0.4, stuns 40% shorter |
| Car-to-car bumps | equal | lighter (0.8) | heavy (×2): pushes the other car away |
| Spin dodge | every 6 s | every 4 s | every 6 s |
| Seekers | 3 | 3 | 4 |
| EMP reach | 95 m | 95 m | 119 m (+25%) |

| Car | Price | Cockpit |
|---|---|---|
| **ROADSTER** | Free (everyone has it) | Flight deck with an open yoke |
| **VIPER** | 2,500 coins | Narrow cockpit with twin side grips |
| **TITAN** | 7,500 coins | Wide armored cockpit with a round wheel |

![Shop](../media/v0.2/shop.jpg)

1. Click a car on the left. It drives onto the stage: **drag** to turn it, **scroll** to zoom.
2. The big button says what you can do: **BUY · 2,500**, **EQUIP**, **✓ EQUIPPED** or **SIGN IN TO BUY**. If your
   wallet is short, **BUY** is greyed out and the card says how much you still need, e.g. **NEED 1,850 MORE**.
3. **BUY** asks you to confirm and shows your wallet before and after. Then choose **EQUIP NOW** or **LATER**.
4. The equipped car is the one you race with, in RACE, TIME ATTACK and FREE ROAM, in your chosen **CAR COLOR**.

![Confirm a purchase](../media/v0.2/shop-confirm.jpg)

Good to know:
- Buying needs a signed-in account and an internet connection. The server checks the price and your wallet, so a
  car can't be bought twice and a slow connection can't charge you twice.
- Cars you buy stay on your account on every device. The equipped car is remembered on each device.
- Spending lowers your **WALLET** only. Your lifetime coins and coin rank stay the same.
- In FREE ROAM, other players see the car you drive, in your color (since 0.2.4).
- **Esc** closes the shop. With the keyboard, **↑ / ↓** move between the cars and **Enter** picks one.

![Shop on a phone](../media/v0.2/phone-shop.jpg)

## Best laps and times
- **BEST LAP**: your fastest single lap on each road (RACE or Time Attack). Saved on this device, shown on the main
  menu (for the road picked), in the road list and in the lap panel. A new best lap shows **★ BEST LAP**.
- **Best 2 laps**: your fastest complete 2-lap **RACE** on each road. Saved to your account and ranked on that road's
  **TIME** ranking. A new one shows on the results screen: *NEW BEST 2 LAPS ON ROAD 3 2:31.20 · TIME RANK #2*.

## Unlocking roads
Win a road in **1st place in RACE** and the next road opens (see [the roads](03-game-modes.md#the-roads)). Time
Attack doesn't unlock roads. Signed in, the server keeps your progress, so it is the same on every device.

## World Ranking
Click **♛ WORLD RANKING** on the main menu. Three tabs, **COIN · TIME · KILLS**, each showing the **top 10** and
your own position (even if you're further down).

| Tab | Ranks by | Who's in it |
|---|---|---|
| **COIN** | Lifetime coins collected | Everyone with an account |
| **TIME** | Fastest complete 2-lap race, **one ranking per road**: pick road **1-9** in the row under the tabs (it opens on the road you picked last) | Players who finished a full RACE on that road |
| **KILLS** | Lifetime kills in FREE ROAM | Players with at least one kill |

![Coins ranking](../media/v0.1/ranking-coins.jpg)
![Fastest 2 laps](../media/v0.1/ranking-time.jpg)
![Free roam kills](../media/v0.1/ranking-kills.jpg)

Your line at the bottom reads, for example, *YOU: #4 · 650 COINS COLLECTED · WALLET 650*, or tells you how to get in
(*FINISH A 2-LAP RACE ON ROAD 3 TO SET YOUR TIME*, *WRECK A PLAYER IN FREE ROAM TO JOIN THIS RANKING*).

Your account panel (top right of the menu) always shows **WORLD #** (coin rank) and **KILL RANK #**.

## Killer status
3 player kills in FREE ROAM make you a **Killer**: your car wears the CRIMSON FLAME everywhere (the VOID CROWN from 10 kills), until you type
**/pkclear** in the FREE ROAM chat. It is saved to your account. See [Free Roam](06-free-roam.md#killer-status).
