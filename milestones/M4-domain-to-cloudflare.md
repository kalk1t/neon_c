# M4 · Connect roadstersrace.com (Namecheap) to Cloudflare

**Who:** You · **Time:** 15 min + waiting (usually under 1 h, at most 24 h) · **Needs:** nothing (can run in parallel with M1–M3) · **Next:** [M5](M5-serve-from-pc.md)

## Goal
Cloudflare manages the domain's DNS, which gives free HTTPS, protection against attacks and traffic floods, and the
Tunnel used in M5. The domain stays **registered and paid at Namecheap**; only its nameservers change.

## Steps

### 1. Add the domain to Cloudflare
- [x] Sign up or log in at https://dash.cloudflare.com
- [x] **Add a domain** → `roadstersrace.com` → "Quick scan for DNS records" → Continue
- [x] Plan: **Free** → Continue
- [x] DNS records page: delete any Namecheap parking records (`A` → parking IP, `CNAME www` → parkingpage). The
      Tunnel adds the right ones in M5.
- [x] Cloudflare shows **2 nameservers** like `xxx.ns.cloudflare.com` and `yyy.ns.cloudflare.com`. Copy both.

### 2. Change the nameservers at Namecheap
- [x] https://ap.www.namecheap.com → **Domain List** → `roadstersrace.com` → **Manage**
- [x] **Nameservers** dropdown: `Namecheap BasicDNS` → change to **Custom DNS**
- [x] Paste both Cloudflare nameservers → click the **green ✓**
- [x] If **DNSSEC** is on (Advanced DNS tab), turn it **off**. It can be re-enabled from Cloudflare later.

### 3. Wait and confirm
- [x] Cloudflare → the domain's Overview → **Check nameservers now**
- [x] Cloudflare emails "roadstersrace.com is now active on a Cloudflare Free plan"
      → verified 30 Oct: the nameservers are `ezra.ns.cloudflare.com` and `lily.ns.cloudflare.com`

### 4. Security settings (after Active)
- [x] **SSL/TLS → Overview → Full**
- [x] **SSL/TLS → Edge Certificates → Always Use HTTPS: On** and **Automatic HTTPS Rewrites: On**
- [x] **Speed → Optimization**: leave the defaults (no Rocket Loader, since it can break the game's scripts)

### 5. help@roadstersrace.com → your Gmail (free)
- [x] Cloudflare → roadstersrace.com → **Email → Email Routing → Get started**
- [x] Custom address `help` → action **Send to an email** → your Gmail → verify the email Cloudflare sends you
- [x] Cloudflare will offer to **delete the old Namecheap MX records** (`eforward*.registrar-servers.com`). Accept, then let
      it **add its own MX and TXT records**
- [x] Test: send an email from another account to help@roadstersrace.com → it arrives in your Gmail → verified 30 Oct: MX `route1-3.mx.cloudflare.net` and the Cloudflare SPF record are live
- [ ] To **reply as** help@roadstersrace.com, optional: Gmail → Settings → Accounts → "Send mail as" → add the address
      using Gmail's SMTP (`smtp.gmail.com`, port 587, your Gmail address and a Google **App Password**)

### 6. Cookieless visit statistics (free)
- [x] Cloudflare → **Analytics & Logs → Web Analytics → Add a site** → roadstersrace.com → **Enable** (automatic setup
      for proxied sites). This counts all visitors, pages, countries and referrers without cookies.

## ✅ Done when
- The Cloudflare dashboard shows `roadstersrace.com` as **Active**, and a test email to help@roadstersrace.com arrives in
  your Gmail.

## If something goes wrong
- Still pending after 24 h → check for typos in the Namecheap nameservers, and make sure DNSSEC is off.
- Check from a terminal: `nslookup -type=NS roadstersrace.com` should list the `*.ns.cloudflare.com` servers.
