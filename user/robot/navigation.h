#ifndef __NAVIGATION_H
#define __NAVIGATION_H

// Tunable parameters
#define NAV_ROT_KP              -3.8
#define NAV_ROT_KI              0
#define NAV_ROT_KD              0.00

#define NAV_FWD_GAIN            0.30

// Fastest / slowest allowed rotation (left/right setpoint delta)
#define NAV_MAX_ROT             140
#define NAV_MIN_ROT             50
// Same thing, but in reverse mode
#define NAV_MAX_RVS_ROT         105

// "Close-enough" angle and distance
// VPS units
#define NAV_POS_EPS             70
// degrees
#define NAV_ANG_EPS             4.0

// Maximum angular error before switching from front drive
// to in-place pivot (degrees)
#define NAV_ANG_DRV_LMT         20.0

#define NAV_THREAD_PRIORITY     10

/* Types */

enum nav_state_t {ROTATE_ONLY, ROTATE, DRIVE, DONE};

struct nav_system_state_t {
    float target_x;
    float target_y;
    float target_t;
    float target_v;
    enum nav_state_t nav_state;
    struct lock *nav_done_lock;
};

/* High-level commands */

void turnToHeading(float t);

void turnToPoint(float x, float y);

void moveToPoint(float x, float y, float v);

void moveToPointDirected(float x, float y, float v, int reverse);

void pause();

void unpause();

struct nav_system_state_t getNavSystemState(void);
void setNavSystemState(struct nav_system_state_t s);

/* Options */
void setHeadingLock(int locked);
void setFastDrive(int fastDrive);
void setRecoveryEnabled(int recoveryEnabled);

/* Navigation status */

void getPosition(float *x, float *y, float *t);

int movementComplete(void);
void waitForMovement(void);

int rotationComplete(void);
void waitForRotation(void);


/* Initialization */

int nav_init(void);

int nav_start(void);

/* Internal */

void vps_update(void);

void gyro_sync(void);

int nav_loop(void);
int recovery_loop(void);

void setTarget(float x, float y, float t, float v);
void setTargetDirected(float x, float y, float t, float v, int reverse);

#endif