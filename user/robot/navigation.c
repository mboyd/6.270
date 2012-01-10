/*
 * Navigation / localization controller
 */
 
#include <joyos.h>

#include "navigation.h"

// Gyro config
#define GYRO_PORT           11
#define LSB_US_PER_DEG      1400000

// Tunable parameters
#define NAV_KP              0
#define NAV_KI              0
#define NAV_KD              0

#define NAV_THREAD_PRIORITY 100

#define NAV_POS_EPS         1.0
#define NAV_ANG_EPS         5.0

// Navigation target
float target_x;
float target_y;
float target_t;

// Navigation position
float current_x;
float current_y;
float current_t;

// Internal nav state
static uint8_t nav_thread_id;
static struct lock nav_goal_lock;
static struct lock nav_done_lock;

static struct pid_controller lm_control;
static struct pid_controller rm_control;


void setTarget(float x, float y, float t) {
    acquire(&nav_goal_lock);
    target_x = x;
    target_y = y;
    target_t = t;
    release(&nav_goal_lock);
}

void waitForArrival(void) {
    acquire(&nav_done_lock);
    release(&nav_done_lock);
}

int nav_init(void) {
    // Calibrate gyro
    printf("Calibrating gyro...");
    pause(100);
    gyro_init(GYRO_PORT, LSB_US_PER_DEG, 10000L);
    printf("done.\n");
    
    // Init vars
    target_x = 0; target_y = 0; target_t = 0;
    
    init_lock(&nav_goal_lock, "nav_goal_lock");
    init_lock(&nav_done_lock, "nav_done_lock");
    
    return 0;
}

int nav_start(void) {
    nav_thread_id = create_thread(nav_loop, 512, NAV_THREAD_PRIORITY, "nav_thread");
    return 0;
}

int nav_loop(void) {
    acquire(&nav_done_lock);
    while (1) {
        // Update position estimate
        current_t = gyro_get_degrees();
        printf("Heading: %.2f\r", current_t);
        
        // Compute target bearing
        // For now, we want constant-angle drive, and have no way of estimating
        // position.  So, we keep the target_t unchanged.
        
        // Rotate
        
        // Drive
        
        pause(1000);
    }
    return 0;
}

