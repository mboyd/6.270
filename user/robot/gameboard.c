#include "gameboard.h"

#include <math.h>

float distanceTo(point_t p) {
    return sqrt(square(p.x - game.coords[0].x) + square(p.y - game.coords[0].y));
}

point_t closestGearbox(void) {
    int min = 0;
    float minDist = distanceTo(territories[0].gearbox);
    for (int i = 1; i < 6; i++) {
        float d = distanceTo(territories[i].gearbox);
        if (d < minDist) {
            min = i;
            minDist = d;
        }
    }
    return territories[min].gearbox;
}

point_t closestLever(void) {
    int min = 0;
    float minDist = distanceTo(territories[0].lever);
    for (int i = 1; i < 6; i++) {
        float d = distanceTo(territories[i].lever);
        if (d < minDist) {
            min = i;
            minDist = d;
        }
    }
    return territories[min].lever;
}
