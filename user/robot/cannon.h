#ifndef CANNON_H
#include <joyos.h>

#define CANNON_RPM_KP   -0.2
#define CANNON_RPM_KD   0
#define CANNON_RPM_KI   -0.15

#define CANNON_RPM_EPS  20

#define CANNON_THREAD_PRIORITY      20

void cannon_set_rpm(float rpm);

float cannon_get_rpm(void);

int cannon_init(void);
int cannon_start(void);
int cannon_loop(void);


#endif