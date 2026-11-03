# car03 — Exact generation prompts

## Selected source and method

The design authority is the exact image attached by the user as `f97e1ea1-e31e-449c-9cc7-ac6f3adc49b0.png`. It is preserved byte-for-byte as `concept.png`. It depicts the layered armored grand-tourer with stacked closed hood plates and hollow rectangular magenta headlight outlines.

Images were generated with the built-in image-generation tool, with `transparent_background: true` for every call. The exact prompts and input roles below retain the selected design's generation history and the final views' refinements. Intermediate drafts are documented but are not included as deliverable images. Reusing a prompt is not a guarantee of identical pixels.

## 1. Design exploration for the selected appearance

Input: text only. This is the concept-generation prompt corresponding to the later user-attached layered armored design.

```text
Use case: stylized-concept.
Generate ONE isolated transparent PNG asset reference for CAR03 in the NEON CIRCUIT art direction. One complete vehicle only, never a collection sheet.

FRAMING AND CAMERA: LANDSCAPE image, front three-quarter view with the NOSE pointing toward the viewer's RIGHT. Use a consistent mildly elevated studio camera suitable for comparing variants. The entire vehicle INCLUDING every pod and spoiler must fit within the CENTRAL 60 PERCENT of the image WIDTH, with very generous empty transparent margins of approximately 20 percent on the left and right. Keep it centered; do not zoom in to fill the canvas. No part cut off. Nothing other than the single car.

SUBJECT: a wide, heavy, muscular ARMORED FLYING GRAND-TOURER COUPE, the brute of the garage. Broad shoulders; a BLUNT aggressive armored front with a WIDE LOW air intake; a short fastback roofline and compact dark tinted canopy; thick angular armor plates layered over the flanks; a low powerful stance. It must look chunky, wide and heavy-set like an armored muscle car, unmistakably different from both a sleek wedge interceptor and a compact sports car. Still a LOW TWO-DOOR COUPE, not an SUV, truck, tank, spacecraft or aircraft. Approximately 5.6-meter overall car length in intended proportions; width INCLUDING the pods must be no more than 70 percent of length, about 3.9 m maximum. Long enough to remain a grand-tourer, not a square block.

MATERIALS: glossy NEUTRAL obsidian-black / graphite body, brushed neutral chrome trim and bolts. MAGENTA-dominant THIN illuminated strips with only SMALL cyan accents. Broad paint panels must stay neutral dark black, NOT saturated pink or blue. Colored light is confined to narrow trim and restrained turbine details. Neutral studio illumination must reveal the dark surfaces without washing them in colored light. High detail, clean stylized 3D game-asset studio render, clear engineered surfaces.

FOUR PODS, EXACT GEOMETRY: exactly FOUR separate large chunky CYLINDRICAL metallic antigravity lift-pod housings total, one at each conventional wheel position: front left/right and rear left/right, bilaterally symmetric. Each sits OUTSIDE the body on a short visibly exposed mechanical arm. Leave a clear EMPTY GAP between the body flank and the pod, crossed only by the thin short mounting arm, so every pod reads as a separable part. Their cylinder axes run TRANSVERSELY left-to-right across the car like wheel axles; circular turbine end faces point OUTWARD TO THE SIDES, never forward, upward or backward. Substantial metallic turbine blades and machined housings, entirely metallic, NO rubber, tires, tread or ordinary wheels. Maintain correct four-corner layout even when the far pair is partially occluded by this camera; do not add extra pods to make four faces visible.

TAIL AND WEAPON: one LARGER CENTRAL rear-facing turbine exhaust port plus a symmetric pair of SMALLER rear SIDE nozzles, all built into the tail and distinct from the four lift-pod housings. Low rear spoiler wholly within body width; it must NOT span over the rear pods. Front weapon aperture is flush, dark and recessed into the front fascia, not firing and without a projecting weapon barrel.

BACKGROUND AND EFFECTS: genuinely transparent alpha background, no floor, no cast shadow, no ground reflection. No propulsion flames or light trails, no fog, bloom, glow halo, colored atmospheric cloud, backdrop or environment. Controlled crisp neon on the object only. No text, logos, labels, watermarks, dimension markings, extra views or layout elements. High-detail 3D modeling reference.

DESIGN VARIANT 3 — LAYERED FORTRESS (do not print this title): Make the heaviest-looking of the three low muscle coupes, with a broad squared BLUNT nose, deeply chamfered upright front corner armor, a single full-width low recessed intake, and a distinctive stepped hood made of three large overlapping angular armor tiers. Very compact low black canopy tucked into the body; pronounced broad rear shoulders; visibly overlapping thick flank armor plates and substantial brushed chrome bolted edges. Thin magenta lines mark selected plate seams and rectangular headlight outlines, with only small cyan turbine-hub highlights. Short robust low spoiler entirely inside body width. Four chunky external metallic turbine pods on clearly visible short arms, separated from the armor by clean gaps. Convey weight through layered solid geometry rather than making it taller: remain a low 5.6m grand-tourer coupe, not a truck, tank, SUV, wedge racer or weapon platform. Keep every shared rule above.
```

## 2. Concept framing

Input: the concept from section 1. The user subsequently attached their selected PNG; that exact attachment, not a regenerated copy, is the packaged `concept.png`.

```text
OUTPAINT / ZOOM OUT THIS CAR CONCEPT. Preserve the exact single car from the supplied image, including its body design, armor, grille, canopy, rear spoiler, four metallic turbine pods, visible short mounting arms and open gaps between body and pods, colors and camera view. This is a framing edit only.

The input car fills almost the entire width. Make the complete car MUCH SMALLER within a LANDSCAPE 1536 x 1024 canvas. Uniformly reduce its displayed scale without stretching or redesigning it. Place the ENTIRE car including all pods and spoiler within ONLY THE CENTRAL 50 TO 60 PERCENT of the full canvas WIDTH. Leave very broad fully transparent empty margins, at least 20 percent of the canvas on BOTH left and right. This large increase in empty margin is the main purpose of the edit. Keep the whole car centered. Do not preserve the original tight zoom.

Preserve its right-facing front three-quarter angle, wide heavy LOW armored grand-tourer proportions, blunt front, predominantly neutral black/graphite body with brushed chrome, thin MAGENTA-dominant lighting and small cyan accents. Exactly four external cylindrical metallic pods on short mechanical arms, transverse cylinder axes with turbine faces outward to the sides. No rubber or tires. No firing, barrel or propulsion effects.

Output ONE isolated transparent PNG. No floor, shadow, background, halo, haze, fog, light spill, reflection, text, labels, new objects or extra view. Empty exterior space must remain fully alpha transparent. Do not fill the added margins with anything. Do not change the vehicle's geometry or invent additional details.
```

## 3. Initial side reference

Input image1: exact user attachment (`concept.png`).

```text
Use case: stylized-concept.
Create ONE isolated transparent PNG modeling reference of the EXACT CAR in the supplied image. This exact user-attached image is the only visual design authority. Change the camera to the requested view; retain the same layered armored grand-tourer.

IDENTITY MUST MATCH: a low wide heavy armored coupe with a blunt broad front, THREE OVERLAPPING CENTRAL HOOD ARMOR LEVELS with beveled chrome edges, including the smaller raised top plate near the windshield. These are CLOSED solid armor plates, NOT an open hood scoop. Two HOLLOW RECTANGULAR MAGENTA HEADLIGHT OUTLINES, wide low black horizontal-slat intake, substantial angular chrome corner guards. Compact tinted canopy beneath a visible raised armor roof cap; short fastback. Thick overlapping ANGULAR flank and shoulder armor plates with thin magenta seams and the distinctive angular magenta outline around side glass/rear shoulders. Do not replace this with a rectangular bolted classic-muscle door plate, plain flat hood, wedge nose, or twin hood ribs.
Glossy NEUTRAL obsidian-black/graphite body, brushed chrome edging and bolts; MAGENTA-dominant THIN lights, with small cyan turbine-center highlights. Keep large body surfaces neutral and dark. Low straight black rear wing with magenta edge accents and modest endplates, wholly within body width and not reaching over the rear pods.

PODS: exactly FOUR large chunky CYLINDRICAL metallic turbine lift pods at the four conventional wheel corners. Each is outside the body on a short visible mechanical arm, with a clearly open body-to-pod gap crossed only by the narrow mounting arm. Cylinder axes run TRANSVERSELY left-to-right like wheel axles; the round turbine faces point OUTWARD TO THE SIDES. Thus side views see circular end discs, front/rear/top see barrel profiles. Do not rotate the pod axes toward front/back/up. No rubber, tires, tread or additional pods. Keep symmetry and the same robust metal bands, magenta ring accents and small cyan turbine hubs seen in the source.
Use nominal eventual model proportions ~5.6m overall length, a low roof and total width including pods no greater than70% of length (~3.92m). Do not display dimension text.

TAIL: exactly THREE body-mounted rear-facing exhaust nozzles, one larger CENTRAL turbine and two matching smaller lateral ports in one row. They are distinct from the four side-facing lift pods. Rear styling must use the same layered black armor, chrome edges and thin magenta accents. Front weapon aperture is flush and recessed into the low intake, not firing, no protruding weapon barrel.

PRESENTATION: STRICT ORTHOGRAPHIC projection, not perspective. Neutral studio illumination revealing black surfaces. Full vehicle centered in LANDSCAPE 1536 x1024 canvas. Generous TRANSPARENT margins, complete projected car occupying only the central60–70% of canvas width/height; no edge-filling close-up and nothing cut off. Real alpha transparency. No floor, shadow, external glow, bloom, haze, fog, cloud, reflections beneath, exhaust fire, trail, text, labels, ruler, watermark or additional views.

REQUESTED VIEW: perfectly square-on ORTHOGRAPHIC LEFT SIDE ELEVATION, NOSE RIGHT, tail LEFT. Camera at body mid-height looking exactly 90 degrees to the longitudinal axis. ZERO front or rear yaw; the front grille's front-facing surface must NOT be visible—show only its side silhouette. Zero camera pitch: no look down across the roof. Two near-side pods appear as completely circular end discs; far-side two are occluded directly behind, not added above the roof. Circular pod discs must not have oblique long barrel sides extending sideways in the image. Preserve the reference's hood armor STEPS as stepped side silhouettes, raised roof cap, short fastback, angular layered shoulder armor, and low spoiler. Do not redesign the body. Place the front and rear pod axles at the same conventional wheel locations implied by the selected concept.
```

## 3. Initial front reference

Input image1: exact user attachment (`concept.png`).

```text
Use case: stylized-concept.
Create ONE isolated transparent PNG modeling reference of the EXACT CAR in the supplied image. This exact user-attached image is the only visual design authority. Change the camera to the requested view; retain the same layered armored grand-tourer.

IDENTITY MUST MATCH: a low wide heavy armored coupe with a blunt broad front, THREE OVERLAPPING CENTRAL HOOD ARMOR LEVELS with beveled chrome edges, including the smaller raised top plate near the windshield. These are CLOSED solid armor plates, NOT an open hood scoop. Two HOLLOW RECTANGULAR MAGENTA HEADLIGHT OUTLINES, wide low black horizontal-slat intake, substantial angular chrome corner guards. Compact tinted canopy beneath a visible raised armor roof cap; short fastback. Thick overlapping ANGULAR flank and shoulder armor plates with thin magenta seams and the distinctive angular magenta outline around side glass/rear shoulders. Do not replace this with a rectangular bolted classic-muscle door plate, plain flat hood, wedge nose, or twin hood ribs.
Glossy NEUTRAL obsidian-black/graphite body, brushed chrome edging and bolts; MAGENTA-dominant THIN lights, with small cyan turbine-center highlights. Keep large body surfaces neutral and dark. Low straight black rear wing with magenta edge accents and modest endplates, wholly within body width and not reaching over the rear pods.

PODS: exactly FOUR large chunky CYLINDRICAL metallic turbine lift pods at the four conventional wheel corners. Each is outside the body on a short visible mechanical arm, with a clearly open body-to-pod gap crossed only by the narrow mounting arm. Cylinder axes run TRANSVERSELY left-to-right like wheel axles; the round turbine faces point OUTWARD TO THE SIDES. Thus side views see circular end discs, front/rear/top see barrel profiles. Do not rotate the pod axes toward front/back/up. No rubber, tires, tread or additional pods. Keep symmetry and the same robust metal bands, magenta ring accents and small cyan turbine hubs seen in the source.
Use nominal eventual model proportions ~5.6m overall length, a low roof and total width including pods no greater than70% of length (~3.92m). Do not display dimension text.

TAIL: exactly THREE body-mounted rear-facing exhaust nozzles, one larger CENTRAL turbine and two matching smaller lateral ports in one row. They are distinct from the four side-facing lift pods. Rear styling must use the same layered black armor, chrome edges and thin magenta accents. Front weapon aperture is flush and recessed into the low intake, not firing, no protruding weapon barrel.

PRESENTATION: STRICT ORTHOGRAPHIC projection, not perspective. Neutral studio illumination revealing black surfaces. Full vehicle centered in LANDSCAPE 1536 x1024 canvas. Generous TRANSPARENT margins, complete projected car occupying only the central60–70% of canvas width/height; no edge-filling close-up and nothing cut off. Real alpha transparency. No floor, shadow, external glow, bloom, haze, fog, cloud, reflections beneath, exhaust fire, trail, text, labels, ruler, watermark or additional views.

REQUESTED VIEW: straight ORTHOGRAPHIC FRONT ELEVATION, camera exactly centered on the longitudinal axis at body mid-height with no yaw, overhead pitch, perspective or roll. Perfect bilateral symmetry. Show the broad BLUNT nose with TWO HOLLOW RECTANGULAR magenta headlight outlines and the low horizontal-slat intake; raised CLOSED stacked hood armor, compact tinted windshield and roof cap behind. No open classic-car hood scoop. Front pods show their METALLIC CYLINDER BARREL PROFILES with the side-facing circular turbine discs only as edge rims; do not put large forward-facing turbine circles at the front. Detached pods have short visible arms and open gaps to body. Rear pods can be naturally occluded. The weapon is just a small dark flush aperture inside the intake, not a gun or glowing orb. Low wing stays within body width.
```

## 3. Initial rear reference

Input image1: exact user attachment (`concept.png`).

```text
Use case: stylized-concept.
Create ONE isolated transparent PNG modeling reference of the EXACT CAR in the supplied image. This exact user-attached image is the only visual design authority. Change the camera to the requested view; retain the same layered armored grand-tourer.

IDENTITY MUST MATCH: a low wide heavy armored coupe with a blunt broad front, THREE OVERLAPPING CENTRAL HOOD ARMOR LEVELS with beveled chrome edges, including the smaller raised top plate near the windshield. These are CLOSED solid armor plates, NOT an open hood scoop. Two HOLLOW RECTANGULAR MAGENTA HEADLIGHT OUTLINES, wide low black horizontal-slat intake, substantial angular chrome corner guards. Compact tinted canopy beneath a visible raised armor roof cap; short fastback. Thick overlapping ANGULAR flank and shoulder armor plates with thin magenta seams and the distinctive angular magenta outline around side glass/rear shoulders. Do not replace this with a rectangular bolted classic-muscle door plate, plain flat hood, wedge nose, or twin hood ribs.
Glossy NEUTRAL obsidian-black/graphite body, brushed chrome edging and bolts; MAGENTA-dominant THIN lights, with small cyan turbine-center highlights. Keep large body surfaces neutral and dark. Low straight black rear wing with magenta edge accents and modest endplates, wholly within body width and not reaching over the rear pods.

PODS: exactly FOUR large chunky CYLINDRICAL metallic turbine lift pods at the four conventional wheel corners. Each is outside the body on a short visible mechanical arm, with a clearly open body-to-pod gap crossed only by the narrow mounting arm. Cylinder axes run TRANSVERSELY left-to-right like wheel axles; the round turbine faces point OUTWARD TO THE SIDES. Thus side views see circular end discs, front/rear/top see barrel profiles. Do not rotate the pod axes toward front/back/up. No rubber, tires, tread or additional pods. Keep symmetry and the same robust metal bands, magenta ring accents and small cyan turbine hubs seen in the source.
Use nominal eventual model proportions ~5.6m overall length, a low roof and total width including pods no greater than70% of length (~3.92m). Do not display dimension text.

TAIL: exactly THREE body-mounted rear-facing exhaust nozzles, one larger CENTRAL turbine and two matching smaller lateral ports in one row. They are distinct from the four side-facing lift pods. Rear styling must use the same layered black armor, chrome edges and thin magenta accents. Front weapon aperture is flush and recessed into the low intake, not firing, no protruding weapon barrel.

PRESENTATION: STRICT ORTHOGRAPHIC projection, not perspective. Neutral studio illumination revealing black surfaces. Full vehicle centered in LANDSCAPE 1536 x1024 canvas. Generous TRANSPARENT margins, complete projected car occupying only the central60–70% of canvas width/height; no edge-filling close-up and nothing cut off. Real alpha transparency. No floor, shadow, external glow, bloom, haze, fog, cloud, reflections beneath, exhaust fire, trail, text, labels, ruler, watermark or additional views.

REQUESTED VIEW: straight ORTHOGRAPHIC REAR ELEVATION, camera exactly centered behind the longitudinal axis at body mid-height; no camera yaw/pitch, perspective or roll. Perfect bilateral symmetry. Broad low rear with layered angular black armor and modest chrome edges, compact rear glass, low wing wholly inside body width. EXACTLY THREE separate body-mounted rear nozzles across the lower tail: a LARGER CENTRAL circular turbine and two SMALLER MATCHING lateral ports. Magenta thin outer rims, tiny cyan centers; no exhaust effects. Narrow magenta taillight outlines above. The external rear lift pods are separate farther left/right on visibly short mechanical arms with open gaps; show their cylindrical BARREL SIDES, not extra rear-facing circular turbine openings. The rear nozzle axis points backward, while lift-pod axes run left-to-right. Do not add a fifth roof or central lift pod.
```

## 4. side view and framing

Input image1: initial side output from section3.

```text
Edit this exact LEFT SIDE car reference ONLY to zoom out and improve the margins. Preserve all body, armor, lighting, pod positions, pod size, roof cap, side profile and current square lateral camera unchanged. Nose remains RIGHT. Exactly two near-side circular turbine faces visible, far pair occluded. Keep the stacked closed hood armor, angular flank plates and magenta window outline; no hood scoop or rectangular door armor. Keep the canvas LANDSCAPE 1536x1024. Zoom OUT from the input and place the complete car INCLUDING all pods within only the CENTRAL 60 TO 65 PERCENT of the frame's width and height, with very broad empty transparent margins on all four sides. This is a deliberate smaller centered object, not an edge-filling product shot. Uniform scaling only; never stretch. Real transparent alpha, no new content in the margins. No floor, cast shadow, background, external glow, halo, fog, reflections, text, labels, watermark, effects or extra views.
```

## 4. rear view and framing

Input image1: initial rear output from section3.

```text
Edit this exact STRAIGHT REAR car reference ONLY to zoom out and improve the margins. Preserve the exact three body-mounted exhausts (large center, smaller symmetric side pair), two visible outer lift-pod barrel profiles on short arms with open gaps, body armor, tail lights, roof and body-width wing. Same camera, zero yaw and zero pitch, perfect bilateral symmetry. Do not alter part counts or design. Keep the canvas LANDSCAPE 1536x1024. Zoom OUT from the input and place the complete car INCLUDING all pods within only the CENTRAL 60 TO 65 PERCENT of the frame's width and height, with very broad empty transparent margins on all four sides. This is a deliberate smaller centered object, not an edge-filling product shot. Uniform scaling only; never stretch. Real transparent alpha, no new content in the margins. No floor, cast shadow, background, external glow, halo, fog, reflections, text, labels, watermark, effects or extra views.
```

## 4. front view and framing

Input image1: exact attachment; image2: initial rear reference; image3: initial front draft.

```text
CORRECT THE CAMERA VIEW of image3, using image1 as exact car design authority and image2 as the correct level rear projection reference. Same single layered armored grand-tourer, no redesign.
The current FRONT draft is taken from ABOVE, incorrectly revealing the rear pods higher in the image. Lower the camera all the way to POD-HUB HEIGHT and use a STRICTLY LEVEL ORTHOGRAPHIC FRONT ELEVATION, looking horizontally down the vehicle's longitudinal axis. ZERO downward angle, zero perspective, no yaw or roll. Front and rear pod hubs share the same physical height. The rear pod pair must be fully OCCLUDED directly behind the FRONT pair; only ONE visible pod barrel on each side. Do NOT stack rear pods above the front pods in the image. The car physically still has four pods.
Preserve the blunt full-width nose, two hollow rectangular magenta headlight outlines, horizontal-slat low grille, small recessed non-firing aperture, closed stepped hood armor (not a scoop), compact tinted canopy with armor roof cap. From this low level front projection see only the appropriate foreshortened profile of the hood steps, not a top-down view across the hood. The two visible front pods are transverse metal cylinder BARRELS on short arms, with open gaps to the body; circular side faces point away left/right and appear only as edge rims. Neutral black/chrome body, thin magenta and tiny cyan accents. Low wing inside body width. Keep the canvas LANDSCAPE 1536x1024. Zoom OUT from the input and place the complete car INCLUDING all pods within only the CENTRAL 60 TO 65 PERCENT of the frame's width and height, with very broad empty transparent margins on all four sides. This is a deliberate smaller centered object, not an edge-filling product shot. Uniform scaling only; never stretch. Real transparent alpha, no new content in the margins. No floor, cast shadow, background, external glow, halo, fog, reflections, text, labels, watermark, effects or extra views.
```

## 4. top view and framing

Input image1: exact attachment; image2: initial side reference; image3: initial rear reference.

```text
Use case: stylized-concept.
Create ONE isolated transparent PNG modeling reference of the EXACT CAR in the supplied image. This exact user-attached image is the only visual design authority. Change the camera to the requested view; retain the same layered armored grand-tourer.

IDENTITY MUST MATCH: a low wide heavy armored coupe with a blunt broad front, THREE OVERLAPPING CENTRAL HOOD ARMOR LEVELS with beveled chrome edges, including the smaller raised top plate near the windshield. These are CLOSED solid armor plates, NOT an open hood scoop. Two HOLLOW RECTANGULAR MAGENTA HEADLIGHT OUTLINES, wide low black horizontal-slat intake, substantial angular chrome corner guards. Compact tinted canopy beneath a visible raised armor roof cap; short fastback. Thick overlapping ANGULAR flank and shoulder armor plates with thin magenta seams and the distinctive angular magenta outline around side glass/rear shoulders. Do not replace this with a rectangular bolted classic-muscle door plate, plain flat hood, wedge nose, or twin hood ribs.
Glossy NEUTRAL obsidian-black/graphite body, brushed chrome edging and bolts; MAGENTA-dominant THIN lights, with small cyan turbine-center highlights. Keep large body surfaces neutral and dark. Low straight black rear wing with magenta edge accents and modest endplates, wholly within body width and not reaching over the rear pods.

PODS: exactly FOUR large chunky CYLINDRICAL metallic turbine lift pods at the four conventional wheel corners. Each is outside the body on a short visible mechanical arm, with a clearly open body-to-pod gap crossed only by the narrow mounting arm. Cylinder axes run TRANSVERSELY left-to-right like wheel axles; the round turbine faces point OUTWARD TO THE SIDES. Thus side views see circular end discs, front/rear/top see barrel profiles. Do not rotate the pod axes toward front/back/up. No rubber, tires, tread or additional pods. Keep symmetry and the same robust metal bands, magenta ring accents and small cyan turbine hubs seen in the source.
Use nominal eventual model proportions ~5.6m overall length, a low roof and total width including pods no greater than70% of length (~3.92m). Do not display dimension text.

TAIL: exactly THREE body-mounted rear-facing exhaust nozzles, one larger CENTRAL turbine and two matching smaller lateral ports in one row. They are distinct from the four side-facing lift pods. Rear styling must use the same layered black armor, chrome edges and thin magenta accents. Front weapon aperture is flush and recessed into the low intake, not firing, no protruding weapon barrel.

PRESENTATION: STRICT ORTHOGRAPHIC projection, not perspective. Neutral studio illumination revealing black surfaces. Full vehicle centered in LANDSCAPE 1536 x1024 canvas. Generous TRANSPARENT margins, complete projected car occupying only the central60–70% of canvas width/height; no edge-filling close-up and nothing cut off. Real alpha transparency. No floor, shadow, external glow, bloom, haze, fog, cloud, reflections beneath, exhaust fire, trail, text, labels, ruler, watermark or additional views.

INPUT ROLES: image1 exact attached car design, image2 same car's side elevation and authoritative front/rear axle locations, image3 authored rear with3body nozzles. Generate a STRICT ORTHOGRAPHIC TOP-DOWN PLAN, camera exactly vertically above, no pitch/yaw/foreshortening. Nose points UP, tail DOWN, perfect bilateral symmetry.
MATCH this same layered armored body: blunt broad rectangular nose and low front intake; THREE closed overlapping central hood armor tiers, small raised plate nearest windshield, raised roof armor cap over compact dark canopy, short fastback, angular layered side and shoulder plates, thin magenta outlines, body-width low rear wing. No classic open hood scoop, no rectangular door armor, no pointed interceptor nose.
Exactly FOUR transverse cylindrical lift pods OUTSIDE the body, each on a short visible arm with a real transparent gap. From above see each cylinder's metal BARREL profile with its round turbine face pointing OUTWARD LEFT/RIGHT, never four circular faces pointing up. Match their longitudinal stations to image2: front axle approximately20–22% of total nose-to-tail length behind the nose, rear axle approximately88–90% behind the nose. The rear ports are the separate three tail nozzles from image3, one central larger and two lateral smaller, pointing backward/downward and only their housings visible from above. No fifth central or roof pod. Keep the canvas LANDSCAPE 1536x1024. Zoom OUT from the input and place the complete car INCLUDING all pods within only the CENTRAL 60 TO 65 PERCENT of the frame's width and height, with very broad empty transparent margins on all four sides. This is a deliberate smaller centered object, not an edge-filling product shot. Uniform scaling only; never stretch. Real transparent alpha, no new content in the margins. No floor, cast shadow, background, external glow, halo, fog, reflections, text, labels, watermark, effects or extra views.
```

## 5. Top-view proportion correction

Input image1: initial top output from section4; image2: exact attachment; image3: final side reference from section4; image4: final rear reference from section4.

```text
Correct the proportions and pod positions of the TOP-DOWN car reference in image1. Image2 is the exact selected hero design, image3 is the correct SIDE profile and axle locations, image4 is the correct rear nozzle arrangement. Preserve this same layered armored car, materials and details.

The current top drawing is almost as wide across the outer pods as it is long, which is INCORRECT. The vehicle must be clearly LONGER than its overall width. Re-render the same car to a physical plan proportion of 5.6m nose-to-tail LENGTH and approximately 3.6–3.7m TOTAL WIDTH INCLUDING ALL PODS, never wider than3.92m. Thus the silhouette including pods should be approximately ONE AND A HALF TIMES AS LONG AS IT IS WIDE. Not a square outline. Make the main central body approximately2.4m wide. The four short-arm-mounted pods should add only about0.6m per side including the gap. Their cylindrical axial width should be around0.45m with diameter around0.85m. Keep a visible small empty gap crossed only by each short mechanical arm. Do NOT achieve this by merely moving the pods onto or inside the body.

In the image, aim for a complete car silhouette about780pixels tall and only510pixels wide, centered on a LANDSCAPE 1536x1024 transparent canvas. This is a significant narrowing of the current overly wide top projection while preserving its substantial armored coupe character. Preserve the BLUNT nose, THREE overlapping closed hood armor plates, compact tinted canopy, raised roof cap, angular flank armor, thin magenta outlines, small cyan accents and low wing strictly inside body width. No open hood scoop, no wedge redesign.

Correct the rear axle position: source top has rear pods too far forward. Place FRONT pod centers20% of total length behind the nose; place REAR pod centers90% behind the nose, matching image3. Nose is UP and tail DOWN, so move the rear pair DOWN close to the rear body. Preserve exactly FOUR total lift pods, identical, axes transverse left-right with circular faces pointing outward; top camera sees cylinder barrel profiles. Exactly THREE rear body exhaust ports (one larger center, two smaller sides) point backward at bottom, separate from the lift pods.

STRICT ORTHOGRAPHIC TOP-DOWN PLAN, zero perspective or tilt, perfectly bilaterally symmetric, same neutral lighting. Real transparent alpha, clean generous margins. One whole car, no floor, shadows, haze, bloom, external halo, trails, flames, text, labels, dimensions, arrows or watermark.
```

## 6. Top-view axle alignment

Input image1: top-view proportion correction from section5; image2: final side reference from section4. Output: packaged `top.png`.

```text
Edit IMAGE1, the top-down car plan, with only TWO precise construction corrections. IMAGE2 is the authoritative side-view axle spacing. Keep the exact same layered armored car, artwork detail, colors, perspective-free overhead camera, nose UP, existing front pod station, existing body, canopy, hood, spoiler and three exhaust ports.

1. MOVE BOTH REAR LIFT PODS BACKWARD together. In this image backward is DOWN. The rear pod centers are currently just AHEAD OF the rear wing, too far forward. Move them down by about90pixels so their centers are BEHIND the rear wing, next to the final tail armor and close to the three rear exhaust ports. Rear axle centerline must be90% of nose-to-tail length behind the nose, matching image2, not80%. Keep these two rear pods entirely separate from the body on short visible connecting arms; move the attachment points with them. The FRONT pair must remain at its existing20% station. Exactly FOUR total lift pods.

2. Reduce the OUTER width span including the pods by about5%, keeping the car length unchanged. Shorten the left-right axial extent of each pod housing slightly and keep the mounting gaps, so the overall outer pod span is about TWO THIRDS of the complete nose-to-tail length, safely under70%. Preserve the cylindrical pods' diameter, circular side faces pointing outward, and all body proportions. Do not make the body narrower or move pods into it.

The rear pair must clearly appear BELOW/BEHIND the low rear wing in this nose-up top view, not alongside the rear window. Preserve all other geometry and the three distinct exhaust housings. Keep body-width spoiler. LANDSCAPE1536x1024, no cropping or new zoom, completely transparent background, no floor/shadow/bloom/halo/effects, no text, one car, one strict orthographic top view.
```

## Modeling interpretation

These are generated orthographic-intended modeling references rather than projections from one validated 3D mesh. Use `concept.png` as the visual design authority, `side.png` for the longitudinal silhouette and axle stations, `rear.png` for the authored three-nozzle layout, and `top.png` for the four detached pods and width proportions. Image framing is not calibrated pixels per meter. Set the final 5.6m length and no-more-than3.92m total width, pod pivots and all attachment coordinates on the actual mesh in Blender. See `README.md`.

