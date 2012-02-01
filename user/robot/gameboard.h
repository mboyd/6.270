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
                              
static point_t blue_lever_offsets[6] = { {256, 5}, \
                                         {242, -22}, \
                                         {240, 35}, \
                                         {250, 28}, \
                                         {256, 30}, \
                                         {260, 0} };
                                         
 static point_t red_lever_offsets[6] = { {260, -10}, \
                                         {242, -30}, \
                                         {235, 15}, \
                                         {240, 44}, \
                                         {256, 10}, \
                                         {250, 0} };

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

uint8_t closestTerritory(void);
point_t closestGearbox(void);
point_t closestLever(void);

point_t gearboxOffset(float nOffset, float pOffset, uint8_t territory);
point_t leverOffset(float nOffset, float pOffset, uint8_t territory);
point_t leverTargetOffset(float nOffset, float pOffset, uint8_t territory);

void decomposeLeverTarget(float x, float y);

#endif