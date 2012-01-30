#ifndef __GAMEBOARD_H
#define __GAMEBOARD_H

#include "joyos.h"

typedef struct point {
    float x;
    float y;
} point_t;

static point_t centers[6] = { {1024, 0}, \
                              {768, 887}, \
                              {-768, 887}, \
                              {-1024, 0}, \
                              {-768, -887}, \
                              {768, -887} };

static point_t outer_vertices[6] = { {2047, 0}, \
                                     {1024, 1773}, \
                                     {-1024, 1773}, \
                                     {-2047, 0}, \
                                     {-1024, -1773}, \
                                     {1024, -1773} };

static point_t inner_vertices[6] = { {443, 256}, \
                                     {0, 512}, \
                                     {-443, 256}, \
                                     {-443, -256}, \
                                     {0, -512}, \
                                     {443, -256} };

static point_t targets[2] = { {-128, 221.5}, \
                              {128, -221.4} };

typedef struct territory {
    point_t gearbox;
    point_t lever;
} territory_t;

static territory_t territories[6] = { { {1791, 443},   {1791, -443}  }, \
                                      { {512, 1773},   {1280, 1330}  }, \
                                      { {-1280, 1330}, {-512, 1773}  }, \
                                      { {-1791, -443},  {-1791, 443}  }, \
                                      { {-512, -1773}, {-1280, -1330} }, \
                                      { {1280, -1330}, {512, -1773}   }  };

float distanceTo(point_t p);

uint8_t opponentPosition(void);

point_t closestGearbox(void);
point_t closestLever(void);

point_t gearboxOffset(float nOffset, float pOffset, uint8_t territory);
point_t leverOffset(float nOffset, float pOffset, uint8_t territory);
point_t leverTargetOffset(float nOffset, float pOffset, uint8_t territory);


#endif