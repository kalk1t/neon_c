#!/bin/bash
# v0.3.6 Meshy models (owner OK 2026-11-08, budget 1,284 credits; ~30 each, capped 35)
gen() { mkdir -p "$1"; meshy-cli make "$2" -o "$1/" --max-credits 35 --timeout 1800 > "$1/log.txt" 2>&1; echo "$1 exit $?" >> done.txt; }
gen sedan "simple modern four-door sedan car, clean smooth body, white paint, dark tinted windows, black wheels, realistic proportions, single object, game asset" &
gen hatch "simple small city hatchback car, compact rounded body, silver paint, dark tinted windows, black wheels, single object, game asset" &
gen van "simple modern delivery van, boxy body, white paint, dark windows, black wheels, single object, game asset" &
gen taxi "simple yellow city taxi sedan car with a lit taxi sign on the roof, dark windows, black wheels, single object, game asset" &
gen bus "simple modern city bus, long body with large dark windows along the side, white and blue livery, single object, game asset" &
gen balloon "colorful hot air balloon with striped red yellow and blue envelope and a wicker basket hanging below, single object, game asset" &
gen drone "small white quadcopter delivery drone with four propellers carrying a cardboard package box underneath, single object, game asset" &
wait
echo ALL DONE >> done.txt
