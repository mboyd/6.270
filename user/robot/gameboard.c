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

point_t gearboxOffset(float offset, uint8_t territory) {
    point_t v1 = outer_vertices[territory];
    point_t v2 = outer_vertices[(territory+1)%6];
    
    point_t normal;
    normal.x = v1.y - v2.y;
    normal.y = v2.x - v1.x;
    
    float mag = sqrt(normal.x*normal.x + normal.y*normal.y);
    
    point_t gearbox = territories[territory].gearbox;
    gearbox.x += offset * normal.x / mag;
    gearbox.y += offset * normal.y / mag;
    
    return gearbox;
}

point_t leverOffset(float offset, uint8_t territory) {
    point_t v1 = outer_vertices[(territory+5)%6];
    point_t v2 = outer_vertices[territory];
    
    point_t normal;
    normal.x = v1.y - v2.y;
    normal.y = v2.x - v1.x;
    
    float mag = sqrt(normal.x*normal.x + normal.y*normal.y);
    
    point_t lever = territories[territory].lever;
    lever.x += offset * normal.x / mag;
    lever.y += offset * normal.y / mag;
    
    return lever;
}

point_t leverTargetOffset(float offset, uint8_t territory) {
    point_t v1 = territories[territory].lever;
    point_t v2 = targets[0];
    
    point_t v;
    v.x = v2.x - v1.x;
    v.y = v2.y - v1.y;
    
    float mag = sqrt(v.x*v.x + v.y*v.y);
    point_t lever = territories[territory].lever;
    lever.x += offset * v.x / mag;
    lever.y += offset * v.y / mag;
    
    return lever;
}


