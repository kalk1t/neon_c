#!/bin/bash
gen() { mkdir -p "$1"; meshy-cli make "$2" -o "$1/" --max-credits 35 --timeout 1500 > "$1/log.txt" 2>&1; echo "$1 exit $?" >> done.txt; }
gen drone "futuristic cyberpunk courier drone with four ducted fans, sleek dark gunmetal body, glowing cyan light strips, single object, game asset" &
gen blimp "sleek futuristic advertising airship blimp, dark glossy hull, large flat billboard screen panel on its side, magenta neon trim lights, game asset" &
gen taxi "futuristic flying sky taxi hover car, rounded dark body, glowing cyan headlights, magenta underglow, no wheels, game asset" &
gen island "floating rock sky island with a slim futuristic neon spire tower on top, cyberpunk, glowing cyan windows, game asset" &
gen antenna "cyberpunk rooftop communication antenna mast with satellite dishes, cables and red beacon lights, dark metal, game asset" &
gen koi "giant stylized koi fish sculpture, chrome body with glowing magenta and cyan fins, futuristic art installation, game asset" &
wait
echo ALL DONE >> done.txt
