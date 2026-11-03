#include <stdio.h>
#include <string.h>
#include "raylib.h"

_Static_assert(RAYLIB_VERSION_MAJOR == 6 && RAYLIB_VERSION_MINOR == 2,
               "The dependency check requires the pinned raylib 6.2 headers.");

int main(void)
{
    const char *name = GetFileName("assets/models/car.glb");
    if (name == NULL || strcmp(name, "car.glb") != 0) {
        fputs("raylib filename call failed\n", stderr);
        return 1;
    }
    SetRandomSeed(42);
    const int sample = GetRandomValue(1, 10);
    if (sample < 1 || sample > 10) {
        fputs("raylib random call failed\n", stderr);
        return 1;
    }
    printf("raylib %s: C17 compile, static link, and non-graphics calls passed.\n", RAYLIB_VERSION);
    puts("This check does not initialize a window, GPU renderer, or game.");
    return 0;
}
