# M6 · Go-live test

**Who:** You + a friend · **Time:** about 30 min · **Needs:** M5 · **Next:** [M7](M7-running-it.md)

## Goal
Prove the public site works for real people on different computers before you share the link.

## Checklist

### Sign-in and nicknames
- [ ] You: sign in on https://roadstersrace.com in **Chrome** → choose a nickname
- [ ] A friend on **their** computer: sign in → try your nickname with different capitals → it's refused → they
      choose another
- [ ] Sign in in **Edge** and **Firefox** too (and **Safari** if anyone has a Mac or iPhone)
- [ ] Change your nickname with EDIT → the new name shows in the ranking, and the old name is free again

### Coins and ranking
- [ ] Both finish a full race → each results screen shows `SAVED +N`
- [ ] The World Ranking lists both in the right order, and "YOUR RANK" is correct for each
- [ ] Racing again within 2 minutes → the save is refused politely (the anti-spam limit); after 2 minutes it saves

### Contact and privacy
- [ ] The **help@roadstersrace.com** link in the menu opens an email, and the message arrives in your Gmail
- [ ] The consent banner appears on the first visit. ALLOW → within a minute you show up in Google Analytics →
      **Reports → Realtime**
- [ ] Firestore → `players/<your id>` shows your email, visits and device, and `visits` goes up after a reload
- [ ] The PRIVACY and TERMS links open the pages

### Guest flow
- [ ] In a private/incognito window: race without signing in → **SIGN IN WITH GOOGLE TO SAVE** → the coins arrive

### Devices and links
- [ ] Phone: the "desktop racer" notice appears, and CONTINUE ANYWAY works
- [ ] Paste the link into WhatsApp/Discord → the title, description and image show
- [ ] https://www.roadstersrace.com redirects to https://roadstersrace.com
- [ ] http:// (no s) redirects to https://

### Resilience
- [ ] Restart the PC and **don't log in** → after about 2 min the site works again from your phone
- [ ] Turn off your internet for 1 min, then on → the site comes back by itself

## ✅ Done when
Every box is ticked. **🚀 The game is live: share https://roadstersrace.com**

## Where to share it first
Friends and Discord servers, r/WebGames, r/IndieGaming, itch.io (as a browser-game page that links to the site), and
X/TikTok clips of takedowns.
