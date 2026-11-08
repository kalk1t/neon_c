#!/bin/bash
gen() { mkdir -p "$1"; meshy-cli make "$2" -o "$1/" --max-credits 35 --timeout 1800 > "$1/log.txt" 2>&1; echo "$1 exit $?" >> done_trees.txt; }
gen tree_oak "tall mature deciduous shade tree, straight trunk, spreading branches, full lush green leafy crown, realistic, single object, game asset" &
gen tree_birch "tall slender birch tree with white bark trunk and an airy green leafy crown, realistic, single object, game asset" &
wait
echo ALL DONE >> done_trees.txt
