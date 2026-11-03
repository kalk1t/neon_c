# car02 — Exact generation prompts

Generated with the built-in image-generation tool. Every image call used `transparent_background: true`. The user-approved concept is preserved byte-for-byte in `concept.png`.

The blocks below retain the exact prompt text used. Input roles are documented outside each block. Intermediate drafts are documented for provenance; only the approved concept and final four views are included in the pack. Generating the same prompts again can produce a different render.

## 1. Initial concept

Input: text only. This produced the design source subsequently reframed for approval.

```text
Generate ONE isolated PNG asset reference, NEON CIRCUIT art direction (black, chrome, magenta and cyan neon). Subject: a long, low, wedge-shaped flying interceptor coupe, sharper and sleeker than a compact sports car: a single-piece canopy, a knife-edge nose, swept-back side fins. Faceted glossy obsidian body panels, brushed chrome trim, CYAN-dominant thin light strips with small magenta accents. FOUR cylindrical anti-gravity thruster pods where the wheels would be, slimmer and more streamlined, with metallic turbine interiors; no tires, no wheels. One central rear turbine nozzle plus two rear side nozzles. Integrated front weapon aperture, not firing. Small rear spoiler. VIEW: front three-quarter, nose toward the viewer's right. Floating in midair, clean stylized 3D game-asset studio render, enough neutral light to read black surfaces, controlled neon, no bloom. Single vehicle filling 80% of the frame, 10% margin, nothing cut off. Transparent background, no floor, shadows, text or watermark. High detail, suitable as a 3D modeling reference.

Construction clarifications: exactly four separate metallic lift-pod housings total, one at each conventional wheel location, two front and two rear; bilaterally symmetric. These are turbine machinery, without rubber bands or tread. Keep the body compact at approximately 5.6-meter overall car proportions, with a low roof and elongated wedge nose. The three rear-facing exhaust ports are built into the tail, one central and a symmetric lateral pair, distinct from the four lift-pod housings. Front weapon aperture is flush and recessed, with no projecting weapon barrel. Dark body surfaces must remain predominantly neutral black/graphite; keep saturated cyan and magenta confined to narrow lighting strips and restrained turbine details. No propulsion flames or light trails, no fog, no glow halo, no ground reflection. Use a landscape composition. Output exactly one car, no additional views, no labels and no comparison-board layout.
```

## 2. Approved concept framing

Input image: the output of section 1. Output: the user-approved `concept.png`.

```text
OUTPAINT AND ZOOM OUT ONLY. Use the supplied car image as the exact design reference. Keep the identical complete car, colors, geometry and viewing angle, but CHANGE ITS SCALE IN THE FRAME substantially.

The original car fills almost the entire image width. In the NEW image, make the car MUCH SMALLER: it should occupy ONLY THE CENTRAL 60 PERCENT of the canvas width. Leave WIDE empty transparent borders, approximately 20 percent of the canvas on the left and 20 percent on the right, and generous transparent space above and below. This large change in framing is the entire purpose of the edit. Do not preserve the original tightly cropped composition. No part of the vehicle may reach the outer 15 percent border.

Preserve the same front three-quarter view and right-pointing wedge nose. Preserve black metal, chrome, thin cyan strips, little magenta accents, canopy, four metallic turbine pods and spoiler. One car. Genuinely transparent background. No floor, no shadow, no halo, no text, no watermark. Do not add anything to fill the new empty margins.
```

## 3a. Initial side view

Input image 1: approved `concept.png`. This side draft was subsequently corrected.

```text
Use case: stylized-concept.
Create ONE isolated transparent PNG, a faithful additional view of the EXACT approved flying car in input image 1. The supplied image is the authoritative design reference, not a framing template. Keep the same body design, proportions, materials and details; change the camera to the requested view.

Locked identity: long low wedge interceptor coupe, approximately 5.6 m length as intended eventual model size, very sharp knife-edge V nose extending noticeably ahead of the front pod axle, long uninterrupted dark tinted canopy, low roof, triangular side glazing. Glossy neutral obsidian-black faceted body with restrained chrome triangular edging. Cyan-dominant THIN light strips, tiny magenta inset accents only, dark neutral paint occupying most surfaces. Exactly FOUR cylindrical metallic turbine lift pods at the four conventional wheel corners, attached on short mechanical mounts: front left/right, rear left/right. Circular turbine openings face outward to the sides. No rubber or wheels. Small straight rear wing on swept supports with cyan outer edges. Flush recessed front weapon aperture, no barrel, not firing.

The unseen tail design must contain THREE distinct rear-facing circular nozzles integrated into the rear body: one larger central turbine and a symmetric pair of smaller side nozzles, all pointing directly backward. These rear body ports are separate from the four corner lift pods. Do not add any fifth lift pod or turbine above the canopy.

Presentation: neutral white studio illumination revealing black surfaces, controlled emissive trim, no colored light washing across body, no bloom or haze. STRICT ORTHOGRAPHIC projection, no perspective, no foreshortening, no artistic camera cant. One whole car centered in a landscape 1536 by 1024 frame, longest visible dimension about 80 percent of frame, at least 10 percent clean transparent margin at edges, no clipping. Keep a common clean presentation with the other reference views. Transparent alpha background, fully readable car. No floor, ground plane, cast shadow, propulsion flames, beams, motion trails, text, labels, watermark, rulers, contact sheet or extra views.

REQUESTED CAMERA: STRICT ORTHOGRAPHIC LEFT SIDE ELEVATION. Look squarely at the driver's LEFT side, camera exactly perpendicular to the longitudinal centerline and level, NO top surface visible. Car NOSE points to the RIGHT, tail and spoiler to the LEFT. Preserve the extremely low elongated wedge silhouette and distinctive long knife nose, not a blunt ordinary sports car. The two near-side cylindrical pods appear as circular turbine faces in profile at front and rear wheel locations; the far-side two pods are occluded, not moved onto the roof. Pod centers share the same horizontal height. Show the full nose-to-tail profile and narrow wing profile. Zero three-quarter rotation, no perspective.
```

## 3b. Initial rear view

Input image 1: approved `concept.png`. This established the three-nozzle rear arrangement.

```text
Use case: stylized-concept.
Create ONE isolated transparent PNG, a faithful additional view of the EXACT approved flying car in input image 1. The supplied image is the authoritative design reference, not a framing template. Keep the same body design, proportions, materials and details; change the camera to the requested view.

Locked identity: long low wedge interceptor coupe, approximately 5.6 m length as intended eventual model size, very sharp knife-edge V nose extending noticeably ahead of the front pod axle, long uninterrupted dark tinted canopy, low roof, triangular side glazing. Glossy neutral obsidian-black faceted body with restrained chrome triangular edging. Cyan-dominant THIN light strips, tiny magenta inset accents only, dark neutral paint occupying most surfaces. Exactly FOUR cylindrical metallic turbine lift pods at the four conventional wheel corners, attached on short mechanical mounts: front left/right, rear left/right. Circular turbine openings face outward to the sides. No rubber or wheels. Small straight rear wing on swept supports with cyan outer edges. Flush recessed front weapon aperture, no barrel, not firing.

The unseen tail design must contain THREE distinct rear-facing circular nozzles integrated into the rear body: one larger central turbine and a symmetric pair of smaller side nozzles, all pointing directly backward. These rear body ports are separate from the four corner lift pods. Do not add any fifth lift pod or turbine above the canopy.

Presentation: neutral white studio illumination revealing black surfaces, controlled emissive trim, no colored light washing across body, no bloom or haze. STRICT ORTHOGRAPHIC projection, no perspective, no foreshortening, no artistic camera cant. One whole car centered in a landscape 1536 by 1024 frame, longest visible dimension about 80 percent of frame, at least 10 percent clean transparent margin at edges, no clipping. Keep a common clean presentation with the other reference views. Transparent alpha background, fully readable car. No floor, ground plane, cast shadow, propulsion flames, beams, motion trails, text, labels, watermark, rulers, contact sheet or extra views.

REQUESTED CAMERA: STRICT ORTHOGRAPHIC STRAIGHT REAR ELEVATION. Camera exactly centered behind the longitudinal axis and level, looking forward at the TAIL, not at the nose. Bilaterally symmetric. NO visible flank, NO angled or above-rear perspective. Rear wing is a straight thin horizontal bar across the upper rear with two supports. Directly below it, clearly show EXACTLY THREE rear-facing circular exhaust nozzle openings in one horizontal row: ONE LARGE CENTRAL turbine nozzle, ONE SMALLER nozzle on the left, ONE MATCHING SMALLER nozzle on the right. All three have metallic interiors with subtle cyan turbine rims, small magenta details only. They fit in the rear body, separate from the cylindrical lift pods at the outer corners. The left and right corner lift pods appear side-on from this angle, not as extra rear-facing exhaust openings. Preserve body width and the low silhouette of the concept. No extra roof turbine. Neutral light, no exhaust fire or external glow.
```

## 4. side view generation

Input image 1: approved concept; input image 2: initial rear view (3b); input image 3: initial side draft (3a).

```text
Use case: stylized-concept.
Create ONE isolated transparent PNG, a faithful additional view of the EXACT approved flying car in input image 1. The supplied image is the authoritative design reference, not a framing template. Keep the same body design, proportions, materials and details; change the camera to the requested view.

Locked identity: long low wedge interceptor coupe, approximately 5.6 m length as intended eventual model size, very sharp knife-edge V nose extending noticeably ahead of the front pod axle, long uninterrupted dark tinted canopy, low roof, triangular side glazing. Glossy neutral obsidian-black faceted body with restrained chrome triangular edging. Cyan-dominant THIN light strips, tiny magenta inset accents only, dark neutral paint occupying most surfaces. Exactly FOUR cylindrical metallic turbine lift pods at the four conventional wheel corners, attached on short mechanical mounts: front left/right, rear left/right. Circular turbine openings face outward to the sides. No rubber or wheels. Small straight rear wing on swept supports with cyan outer edges. Flush recessed front weapon aperture, no barrel, not firing.

The unseen tail design must contain THREE distinct rear-facing circular nozzles integrated into the rear body: one larger central turbine and a symmetric pair of smaller side nozzles, all pointing directly backward. These rear body ports are separate from the four corner lift pods. Do not add any fifth lift pod or turbine above the canopy.

Presentation: neutral white studio illumination revealing black surfaces, controlled emissive trim, no colored light washing across body, no bloom or haze. STRICT ORTHOGRAPHIC projection, no perspective, no foreshortening, no artistic camera cant. One whole car centered in a landscape 1536 by 1024 frame, longest visible dimension about 80 percent of frame, at least 10 percent clean transparent margin at edges, no clipping. Keep a common clean presentation with the other reference views. Transparent alpha background, fully readable car. No floor, ground plane, cast shadow, propulsion flames, beams, motion trails, text, labels, watermark, rulers, contact sheet or extra views.
POD GEOMETRY LOCK: all four cylindrical lift pods have their cylinder axes running TRANSVERSELY, left-to-right across the car like wheel axles. Their circular turbine end faces point outward to the LEFT and RIGHT. Therefore an exact side elevation sees two circular end faces, and front/rear/top views see cylinder barrel profiles. Do not change this orientation between views. The rear body exhaust nozzles point BACKWARD along the car's length, which is a different axis from the transverse lift pods.

Input image 1 is the exact APPROVED CAR design authority. Input image 2 is the established STRAIGHT REAR solution showing the three rear nozzles and correct transverse lift-pod orientation. Use their same black/chrome surfaces, small cyan and magenta lighting details, canopy, sharply extended nose and low rear wing.

COMPOSITION: give this isolated car noticeably broad empty transparent margins. Do not inherit the overly tight framing from the rear reference. Fit the entire projected vehicle inside the CENTRAL 70 PERCENT of the frame's width and height, centered, with roughly 15 percent or more transparent space on every side. The car must be clearly smaller than an edge-filling product photo. Keep the finished canvas landscape 1536 x 1024.
REQUESTED VIEW: a corrected, genuinely STRICT ORTHOGRAPHIC LEFT SIDE elevation, NO perspective, NO camera yaw, NO camera pitch. Nose RIGHT, tail LEFT. The camera is exactly 90 degrees lateral to the longitudinal axis and at mid-body height. Input image 3 is an earlier side draft to preserve its body silhouette, but CORRECT ITS POD PROJECTION: the two visible near-side pods must be FULL CIRCULAR turbine end discs seen square-on, centered at the front and rear wheel positions. No elongated cylinder side extending to one side of a disc; a transverse cylinder is seen end-on in this projection. The far-side pair is hidden exactly behind the near pair. No top surfaces of hood, roof or spoiler may be visible. The rear spoiler becomes a thin side profile with its support. Keep the long sharp taper well ahead of the front pod center. One full clean side elevation only.
```

## 4. front view generation

Input image 1: approved concept; input image 2: initial rear view (3b).

```text
Use case: stylized-concept.
Create ONE isolated transparent PNG, a faithful additional view of the EXACT approved flying car in input image 1. The supplied image is the authoritative design reference, not a framing template. Keep the same body design, proportions, materials and details; change the camera to the requested view.

Locked identity: long low wedge interceptor coupe, approximately 5.6 m length as intended eventual model size, very sharp knife-edge V nose extending noticeably ahead of the front pod axle, long uninterrupted dark tinted canopy, low roof, triangular side glazing. Glossy neutral obsidian-black faceted body with restrained chrome triangular edging. Cyan-dominant THIN light strips, tiny magenta inset accents only, dark neutral paint occupying most surfaces. Exactly FOUR cylindrical metallic turbine lift pods at the four conventional wheel corners, attached on short mechanical mounts: front left/right, rear left/right. Circular turbine openings face outward to the sides. No rubber or wheels. Small straight rear wing on swept supports with cyan outer edges. Flush recessed front weapon aperture, no barrel, not firing.

The unseen tail design must contain THREE distinct rear-facing circular nozzles integrated into the rear body: one larger central turbine and a symmetric pair of smaller side nozzles, all pointing directly backward. These rear body ports are separate from the four corner lift pods. Do not add any fifth lift pod or turbine above the canopy.

Presentation: neutral white studio illumination revealing black surfaces, controlled emissive trim, no colored light washing across body, no bloom or haze. STRICT ORTHOGRAPHIC projection, no perspective, no foreshortening, no artistic camera cant. One whole car centered in a landscape 1536 by 1024 frame, longest visible dimension about 80 percent of frame, at least 10 percent clean transparent margin at edges, no clipping. Keep a common clean presentation with the other reference views. Transparent alpha background, fully readable car. No floor, ground plane, cast shadow, propulsion flames, beams, motion trails, text, labels, watermark, rulers, contact sheet or extra views.
POD GEOMETRY LOCK: all four cylindrical lift pods have their cylinder axes running TRANSVERSELY, left-to-right across the car like wheel axles. Their circular turbine end faces point outward to the LEFT and RIGHT. Therefore an exact side elevation sees two circular end faces, and front/rear/top views see cylinder barrel profiles. Do not change this orientation between views. The rear body exhaust nozzles point BACKWARD along the car's length, which is a different axis from the transverse lift pods.

Input image 1 is the exact APPROVED CAR design authority. Input image 2 is the established STRAIGHT REAR solution showing the three rear nozzles and correct transverse lift-pod orientation. Use their same black/chrome surfaces, small cyan and magenta lighting details, canopy, sharply extended nose and low rear wing.

COMPOSITION: give this isolated car noticeably broad empty transparent margins. Do not inherit the overly tight framing from the rear reference. Fit the entire projected vehicle inside the CENTRAL 70 PERCENT of the frame's width and height, centered, with roughly 15 percent or more transparent space on every side. The car must be clearly smaller than an edge-filling product photo. Keep the finished canvas landscape 1536 x 1024.
REQUESTED VIEW: STRICT ORTHOGRAPHIC STRAIGHT FRONT elevation. Camera exactly centered on the front longitudinal axis, looking straight backward at the nose at mid-body height. No camera yaw, no pitch, no roll, no perspective. The vehicle is perfectly bilateral and symmetrical. Neither body flank is visible, and do not look down onto the long hood. Front lift pods show their metal BARREL PROFILES at the outer left and right; their turbine faces point sideways and are NOT large circular faces aimed at the viewer. Rear lift pods may be naturally occluded behind the front ones; do not invent extra pods for visibility. Show the low faceted triangular nose with its single flush dark aperture and small magenta accent, narrow cyan headlights, the sloping dark canopy behind it, and the thin wing line behind. Do not move the rear exhaust trio to the front. No guns, no firing, no glowing orb in the nose.
```

## 4. top view generation

Input image 1: approved concept; input image 2: initial rear view (3b).

```text
Use case: stylized-concept.
Create ONE isolated transparent PNG, a faithful additional view of the EXACT approved flying car in input image 1. The supplied image is the authoritative design reference, not a framing template. Keep the same body design, proportions, materials and details; change the camera to the requested view.

Locked identity: long low wedge interceptor coupe, approximately 5.6 m length as intended eventual model size, very sharp knife-edge V nose extending noticeably ahead of the front pod axle, long uninterrupted dark tinted canopy, low roof, triangular side glazing. Glossy neutral obsidian-black faceted body with restrained chrome triangular edging. Cyan-dominant THIN light strips, tiny magenta inset accents only, dark neutral paint occupying most surfaces. Exactly FOUR cylindrical metallic turbine lift pods at the four conventional wheel corners, attached on short mechanical mounts: front left/right, rear left/right. Circular turbine openings face outward to the sides. No rubber or wheels. Small straight rear wing on swept supports with cyan outer edges. Flush recessed front weapon aperture, no barrel, not firing.

The unseen tail design must contain THREE distinct rear-facing circular nozzles integrated into the rear body: one larger central turbine and a symmetric pair of smaller side nozzles, all pointing directly backward. These rear body ports are separate from the four corner lift pods. Do not add any fifth lift pod or turbine above the canopy.

Presentation: neutral white studio illumination revealing black surfaces, controlled emissive trim, no colored light washing across body, no bloom or haze. STRICT ORTHOGRAPHIC projection, no perspective, no foreshortening, no artistic camera cant. One whole car centered in a landscape 1536 by 1024 frame, longest visible dimension about 80 percent of frame, at least 10 percent clean transparent margin at edges, no clipping. Keep a common clean presentation with the other reference views. Transparent alpha background, fully readable car. No floor, ground plane, cast shadow, propulsion flames, beams, motion trails, text, labels, watermark, rulers, contact sheet or extra views.
POD GEOMETRY LOCK: all four cylindrical lift pods have their cylinder axes running TRANSVERSELY, left-to-right across the car like wheel axles. Their circular turbine end faces point outward to the LEFT and RIGHT. Therefore an exact side elevation sees two circular end faces, and front/rear/top views see cylinder barrel profiles. Do not change this orientation between views. The rear body exhaust nozzles point BACKWARD along the car's length, which is a different axis from the transverse lift pods.

Input image 1 is the exact APPROVED CAR design authority. Input image 2 is the established STRAIGHT REAR solution showing the three rear nozzles and correct transverse lift-pod orientation. Use their same black/chrome surfaces, small cyan and magenta lighting details, canopy, sharply extended nose and low rear wing.

COMPOSITION: give this isolated car noticeably broad empty transparent margins. Do not inherit the overly tight framing from the rear reference. Fit the entire projected vehicle inside the CENTRAL 70 PERCENT of the frame's width and height, centered, with roughly 15 percent or more transparent space on every side. The car must be clearly smaller than an edge-filling product photo. Keep the finished canvas landscape 1536 x 1024.
REQUESTED VIEW: STRICT ORTHOGRAPHIC TOP-DOWN PLAN view. Camera exactly vertically above the car, looking straight down, NO perspective, no tilt or foreshortening. Nose points straight UP toward the top edge; tail points straight DOWN. Bilateral symmetry across the vertical image centerline. Preserve the very elongated low wedge shape with a pointed V nose projecting ahead of the front pod pair, central dark continuous canopy, cyan shoulder lines and small rear wing. EXACTLY FOUR lift pods in the conventional four wheel-corner positions: upper left and upper right front pair; lower left and lower right rear pair. Each pod is a transverse cylinder with rectangular/ribbed barrel silhouette from above and a narrow outer rim; do NOT draw four circular turbine faces pointing upward. The three rear exhaust openings face backward/downward, NOT upward toward the top camera; from above only their housings and short rear lips can be visible beneath the wing. No fifth pod on the roof or centerline. Entire vehicle within the stated broad margins.
```

## 5. Final front alignment and framing

Input image: the front output from section 4. Output: `front.png`.

```text
Edit this isolated car reference PNG. Preserve this EXACT car, its design, its current front camera view, geometric proportions, neutral lighting, black/chrome materials, thin cyan light strips and small magenta accents. This is a framing adjustment, not a redesign.

OUTPAINT / ZOOM OUT: the original object is too close to the image edges. Make the car drawing visibly SMALLER within the frame by increasing empty transparent space around it. The complete left-to-right extent of the whole vehicle should occupy about 78 percent of the canvas WIDTH, centered horizontally, with at least 11 percent empty transparent margin on both sides. Keep the final canvas LANDSCAPE 1536 by 1024 pixels, identical to the input canvas. Do not inherit the tight edge-filling crop. No clipping. Keep the whole vehicle centered. The extra space is completely transparent.

Keep the projection strictly orthographic, with no new camera tilt, perspective, cant or rotation. Preserve all individual parts and correct part counts. One car, one view only. Real alpha transparency, no floor or ground shadow, no halo, no bloom, no exhaust trails, no text, no labels, no border and no watermark. Do not fill the added space with any new object or scenery.
```

## 5. Final side alignment and framing

Input image: the side output from section 4. Output: `side.png`.

```text
Edit this isolated car reference PNG. Preserve this EXACT car, its design, its current side camera view, geometric proportions, neutral lighting, black/chrome materials, thin cyan light strips and small magenta accents. This is a framing adjustment, not a redesign.

OUTPAINT / ZOOM OUT: the original object is too close to the image edges. Make the car drawing visibly SMALLER within the frame by increasing empty transparent space around it. The complete left-to-right extent of the whole vehicle should occupy about 78 percent of the canvas WIDTH, centered horizontally, with at least 11 percent empty transparent margin on both sides. Keep the final canvas LANDSCAPE 1536 by 1024 pixels, identical to the input canvas. Do not inherit the tight edge-filling crop. No clipping. Keep the whole vehicle centered. The extra space is completely transparent.

Keep the projection strictly orthographic, with no new camera tilt, perspective, cant or rotation. Preserve all individual parts and correct part counts. One car, one view only. Real alpha transparency, no floor or ground shadow, no halo, no bloom, no exhaust trails, no text, no labels, no border and no watermark. Do not fill the added space with any new object or scenery.
```

## 5. Final rear alignment and framing

Input image: the initial rear view (3b). Output: `rear.png`.

```text
Edit this isolated car reference PNG. Preserve this EXACT car, its design, its current rear camera view, geometric proportions, neutral lighting, black/chrome materials, thin cyan light strips and small magenta accents. This is a framing adjustment, not a redesign.

OUTPAINT / ZOOM OUT: the original object is too close to the image edges. Make the car drawing visibly SMALLER within the frame by increasing empty transparent space around it. The complete left-to-right extent of the whole vehicle should occupy about 78 percent of the canvas WIDTH, centered horizontally, with at least 11 percent empty transparent margin on both sides. Keep the final canvas LANDSCAPE 1536 by 1024 pixels, identical to the input canvas. Do not inherit the tight edge-filling crop. No clipping. Keep the whole vehicle centered. The extra space is completely transparent.

Keep the projection strictly orthographic, with no new camera tilt, perspective, cant or rotation. Preserve all individual parts and correct part counts. One car, one view only. Real alpha transparency, no floor or ground shadow, no halo, no bloom, no exhaust trails, no text, no labels, no border and no watermark. Do not fill the added space with any new object or scenery.
```

## 5. Final top alignment and framing

Input image: the top output from section 4. Output: `top.png`.

```text
Edit this isolated car reference PNG. Preserve this EXACT car, its design, its current top camera view, geometric proportions, neutral lighting, black/chrome materials, thin cyan light strips and small magenta accents. This is a framing adjustment, not a redesign.

OUTPAINT / ZOOM OUT: the original object is too close to the image edges. Make the car drawing visibly SMALLER within the frame by increasing empty transparent space around it. The complete nose-to-tail length should occupy about 78 percent of the canvas HEIGHT, centered vertically, with at least 11 percent empty transparent margin above and below. Preserve the top-down orientation, nose UP. Keep the final canvas LANDSCAPE 1536 by 1024 pixels, identical to the input canvas. Do not inherit the tight edge-filling crop. No clipping. Keep the whole vehicle centered. The extra space is completely transparent.

ONE PRECISE ALIGNMENT CORRECTION: Move BOTH front lift pods rearward together by about 9 percent of total vehicle length, i.e. DOWNWARD in this top view, so their axle line is approximately ONE THIRD of the full car length back from the pointed nose. The current pair is too close to the tip. Keep the rear pod pair exactly at its current body-relative station. Preserve exactly four total lift pods. Keep all body, canopy, wing and rear-nozzle geometry unchanged. After this single pod-station correction, perform the reframe above. Do not move the front pods outward or inward except to maintain their attachment to the unchanged body.
Keep the projection strictly orthographic, with no new camera tilt, perspective, cant or rotation. Preserve all individual parts and correct part counts. One car, one view only. Real alpha transparency, no floor or ground shadow, no halo, no bloom, no exhaust trails, no text, no labels, no border and no watermark. Do not fill the added space with any new object or scenery.
```

## Reference interpretation

The additional views are generated orthographic-intended references, not renders of one reconstructed 3D mesh. `concept.png` is the visual design authority. `rear.png` establishes the authored three-nozzle solution. The final top correction aligns its front pod station with the side view. Set exact physical scale, pivots and attachment coordinates on the resulting model in Blender; see `README.md`.

