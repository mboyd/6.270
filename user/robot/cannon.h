#ifndef CANNON_H
#include <joyos.h>

#define CANNON_RPM_KP   -0.2
#define CANNON_RPM_KD   0
#define CANNON_RPM_KI   -0.15

#define CANNON_RPM_EPS  50

#define CANNON_THREAD_PRIORITY      20

struct lock cannon_empty;

void cannon_set_distance(float distance);
void cannon_set_rpm(float rpm);
float cannon_get_rpm(void);

void setTargetReady(int targetReady);

int cannon_ready(void);
void cannon_wait(void);

void cannon_fire_wait(void);

int cannon_init(void);
int cannon_start(void);
int cannon_loop(void);
int cannon_trigger_loop(void);


#endif