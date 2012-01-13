#ifndef __NAVIGATION_H
#define __NAVIGATION_H

/* High-level commands */

void turnToHeading(float t);

void turnToPoint(float x, float y);

void moveToPoint(float x, float y, float v);

/* Navigation status */

void getPosition(float *x, float *y, float *t);

int isMovementComplete(void);

void waitForMovementComplete(void);


/* Initialization */

int nav_init(void);

int nav_start(void);

/* Internal */

int nav_loop(void);

#endif