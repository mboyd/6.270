#include "gameboard.h"

#include <math.h>

float distanceTo(point_t p) {
    return sqrt(square(p.x - game.coords[0].x) + square(p.y - game.coords[0].y));
}

uint8_t opponentPosition(void) {
    int min = 0;
    float minDist = square(game.coords[1].x - centers[0].x) + 
                        square(game.coords[1].y - centers[0].y);
    for (int i = 1; i < 6; i++) {
        float d = square(game.coords[1].x - centers[i].x) + 
                            square(game.coords[1].y - centers[i].y);
        if (d < minDist) {
            min = i;
            minDist = d;
        }
    }
    return min;
}

uint8_t closestTerritory(void) {
    uint8_t min = 0;
    float minDist = distanceTo(centers[0]);
    for (int i = 1; i < 6; i++) {
        float d = distanceTo(centers[i]);
        if (d < minDist) {
            min = i;
            minDist = d;
        }
    }
    return min;
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

point_t gearboxOffset(float nOffset, float pOffset, uint8_t territory) {
    point_t v1 = outer_vertices[territory];
    point_t v2 = outer_vertices[(territory+1)%6];
    
    point_t normal;
    normal.x = v1.y - v2.y;
    normal.y = v2.x - v1.x;
    
    point_t edge;
    edge.x = v2.x - v1.x;
    edge.y = v2.y - v1.y;
    
    float mag = sqrt(normal.x*normal.x + normal.y*normal.y);
    
    point_t gearbox = territories[territory].gearbox;
    gearbox.x += nOffset * normal.x / mag;
    gearbox.y += nOffset * normal.y / mag;
    
    gearbox.x += pOffset * edge.x / mag;
    gearbox.y += pOffset * edge.y / mag;
    
    return gearbox;
}

point_t leverOffset(float nOffset, float pOffset, uint8_t territory) {
    point_t v1 = outer_vertices[(territory+5)%6];
    point_t v2 = outer_vertices[territory];
    
    point_t normal;
    normal.x = v1.y - v2.y;
    normal.y = v2.x - v1.x;
    
    point_t edge;
    edge.x = v2.x - v1.x;
    edge.y = v2.y - v1.y;
    
    float mag = sqrt(normal.x*normal.x + normal.y*normal.y);
    
    point_t lever = territories[territory].lever;
    lever.x += nOffset * normal.x / mag;
    lever.y += nOffset * normal.y / mag;
    
    lever.x += pOffset * edge.x / mag;
    lever.y += pOffset * edge.y / mag;
    
    return lever;
}

point_t leverTargetOffset(float nOffset, float pOffset, uint8_t territory) {
    point_t v1 = territories[territory].lever;
    point_t v2 = targets[0];
    
    point_t v;
    v.x = v2.x - v1.x;
    v.y = v2.y - v1.y;
    
    point_t u;
    u.x = v2.y - v1.y;
    u.y = v1.x - v2.x;
    
    float mag = sqrt(v.x*v.x + v.y*v.y);
    point_t lever = territories[territory].lever;
    lever.x += nOffset * v.x / mag;
    lever.y += nOffset * v.y / mag;
    
    lever.x += pOffset * u.x / mag;
    lever.y += pOffset * u.y / mag;
    
    return lever;
}

void decomposeLeverTarget(float x, float y) {
    uint8_t territory = closestTerritory();
    
    point_t v1 = territories[territory].lever;
    point_t v2 = targets[0];
    
    point_t v;
    v.x = v2.x - v1.x;
    v.y = v2.y - v1.y;
    
    point_t u;
    u.x = v2.y - v1.y;
    u.y = v1.x - v2.x;
    
    float mag = sqrt(v.x*v.x + v.y*v.y);
    
    point_t dis;
    dis.x = x - v1.x;
    dis.y = y - v1.y;
    
    // nOffset = dis . v
    float nOffset = (dis.x * v.x + dis.y * v.y) / mag;
    // pOffset = dis . u
    float pOffset = (dis.x * u.x + dis.y * u.y) / mag;
    
    printf("nOffset: %.2f  pOffset: %.2f   \n", nOffset, pOffset);
}


