# 6. Free Roam online

🔄 **Changing** (new in v0.1, still being tuned) · [Manual contents](README.md) · Previous: [Combat](05-combat.md) · Next: [Progress](07-progress.md)

FREE ROAM is a shared online sky city. Fly anywhere with other players, chat, and fight: wrecking another player is a
**kill**, and your lifetime kills have their own World Ranking.

![Free roam](../media/v0.1/free-roam.jpg)

## Joining
1. [Sign in and choose a nickname](01-getting-started.md#signing-in). Without them the button asks you to sign in first.
2. Click **FREE ROAM · ONLINE**. There is no countdown: you start right away at the **spawn hub**, a formation of
   parking spots in the sky above the start deck. Everyone gets their own spot.
3. The top-left panel shows **CONNECTING…**, then **ONLINE · 3 DRIVERS**.

Up to **24 players** fit in the city. When it is full you'll see *FREE ROAM IS FULL · RETRYING…*, and the game keeps
trying until a place frees up.

**One car per account:** if you open FREE ROAM in a second tab, the first one shows *YOU JOINED FROM ANOTHER TAB*.

## Where you can fly
Anywhere: there are no laps, coins or "wrong way". The race course is cleared away: no start gate, rings, guidance line
or signs, and the deck loses its start line and grid. Limits:
- You can't go below the cloud layer (28 m) or above 1,600 m.
- The city is 9 km across in every direction from the center; at the edge you're held back.

Cars bump into each other: flying into another player pushes you both apart.

## The FREE ROAM screen
| Where | What |
|---|---|
| Top left | **ONLINE · N DRIVERS**, then your **FPS · MS · ONLINE** line, then the **player list**: everyone in the city in their car color, sorted by kills this session, with a **KILLS** column. You are marked **(YOU)** |
| Over each car | Its pilot's name, distance, and a **health bar**. Shown at any distance |
| Top center | The **kill feed**: *NeonQueen ✖ skyline_ace* when other players wreck each other |
| Left | **Chat** |
| Center | Messages: *NeonQueen JOINED*, *HIT BY NeonQueen*, *SEEKER INCOMING · NeonQueen*, *DODGED*… |

## Fighting and kills
All weapons work on other players ([Combat](05-combat.md)), with these values:

| Weapon | Damage to a player |
|---|---|
| Blaster bolt | 21 |
| Seeker | 48 |
| EMP | 25 at the center, 12 at the edge |

- **Hits count where you see the car.** If your shot hits a player's car on your screen, it hits them. (Their shots
  on you work the same way.)
- At 0 health the player is **wrecked**: you get the **kill**, **+50% boost** and **+1 seeker**, and everyone sees it
  in their kill feed.

![You got a takedown](../media/v0.1/free-roam-takedown.jpg)

- Your kills **this session** (since you joined) are shown in the player list. They start from 0 every time you join.
- Every kill is also added to your **lifetime kills**, which are saved to your account (at most once a minute, so a
  new kill can take up to a minute to appear in your profile) and ranked in **FREE ROAM KILLS**
  ([Progress](07-progress.md#world-ranking)).
- After a session the main menu shows *LAST FREE ROAM 4 KILLS · LIFETIME 31*.
- The same player can't be wrecked twice within 2 seconds, so kills can't be farmed.

## KILLER STATUS
Wreck **3 players** in FREE ROAM and you become a **KILLER**: *KILLER* flashes on screen, and your car bursts into
the **CRIMSON FLAME**: a crimson underglow, flames along the sides, a fire wreath on every thruster pod and an ember
trail behind you.

Keep going: at **10 kills** as a Killer, *VOID CROWN* flashes on screen and the flame turns into the **VOID CROWN**: a
crystal crown floating above the roof, crystal shards circling the car, and violet flames, wreaths and embers.

- **Everyone sees it**: other players see your aura in FREE ROAM, and their kill feed says
  *NeonQueen IS A KILLER*.
- **It stays.** The aura is on your car in FREE ROAM, in races, in the main menu and in the shop, on every car you
  own, after reloading the page and after reconnecting.
- **No R respawn.** While you're a Killer, **R** doesn't take you home: *RESPAWN UNAVAILABLE WHILE KILLER*. After
  being wrecked you still respawn as normal.
- Only kills of **players** count (in races, AI rivals don't), with the same rules as kills.
- **To clear it**, type **/pkclear** in the FREE ROAM chat. The aura disappears at once (for everyone), R works again
  and your Killer count starts again from 0. Nobody else sees the command.
- Chat commands start with **/**. Unknown ones answer *UNKNOWN COMMAND* (only to you).

## Health
- You have your car's health (the **SHIELD** bar): ROADSTER 100, VIPER 75, TITAN 140.
- **It regenerates:** after **5 seconds** without being hit, it comes back at **8 per second** (empty to full in
  about 12 seconds).
- When you're hit, the screen names the shooter: *HIT BY NeonQueen*, *SEEKER BY…*, *EMP BY…*.
- A seeker locked on you is announced: **SEEKER INCOMING · NeonQueen**. Spin dodge (**F**) at the right moment.

## Respawning
![Wrecked](../media/v0.1/free-roam-wrecked.jpg)

- When wrecked, you're down for **3 seconds** (*RESPAWN IN 3 · 2 · 1*), then you respawn at the hub with full health.
- **Spawn protection:** for **3 seconds** after (re)spawning, nothing can hurt you, so nobody can camp the hub.
- **R** (reset) takes you back to the hub too, but **keeps your current health** and gives **no** spawn protection.
  It's a way home, not an escape from a fight. Not available while you're a [Killer](#killer-status).

## Chat
![Chat open](../media/v0.1/free-roam-chat.jpg)

| | Computer | Phone |
|---|---|---|
| Open | **Enter** (or **T**) | **CHAT** button (top) |
| Send | **Enter** | The keyboard's **Send** key |
| Close without sending | **Enter** on an empty line, or **Esc** (Esc also opens the pause screen, because it releases the mouse) | Tap elsewhere / close the keyboard |

- Messages appear on the left, see-through, with names in their car colors. They **fade after 12 seconds**;
  opening the chat brings the recent ones back.
- *NeonQueen joined* / *NeonQueen left* lines show who comes and goes.
- Up to **120 characters** per message. Sending too fast shows **SLOW DOWN**: you can send 5 messages quickly, then
  about one every 3 seconds.
- While typing, your car keeps flying straight on.
- Be friendly: rude words are shown as stars. Messages aren't stored, except when someone reports a player: then that
  player's last few messages are sent to us.
- **/mute NAME** hides a player's messages on your device; **/unmute NAME** shows them again; **/muted** lists who you
  muted. Only you notice.
- **/report NAME** sends that player's recent messages to us (once every 30 seconds). Players who abuse the chat can
  be banned from FREE ROAM; races still work for them.
- **/help** lists all chat commands.

## FPS, ping and time online
Under the **ONLINE** line:

| Value | Meaning |
|---|---|
| **FPS** | Frames per second your device is drawing. 30+ is smooth, 60 is ideal. Low? Try **LOW** graphics |
| **MS** | Ping: how long a message takes to reach the server and back. **Green** under 80 ms, **yellow** under 160, **red** above |
| **ONLINE** | How long you've been in FREE ROAM this time (keeps counting through short reconnects) |

## Connection messages
| Message | Meaning |
|---|---|
| **CONNECTING…** / **RECONNECTING…** | Joining, or the connection dropped and the game is retrying by itself |
| **SIGN IN TO DRIVE ONLINE** | Your sign-in expired: sign in again |
| **CHOOSE A NICKNAME TO DRIVE ONLINE** | Set a nickname first |
| **FREE ROAM IS FULL · RETRYING…** | 24 players are in; the game retries automatically |
| **YOU JOINED FROM ANOTHER TAB** | Your account is now playing in another tab |
| **GAME UPDATED · RELOADING…** | A new version of the game is out; the page reloads itself |
| **SERVER UPDATING · TRY AGAIN SOON** | The server is being restarted; wait a minute and try again |
