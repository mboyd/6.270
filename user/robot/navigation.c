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

// Navigation target
float target_x;
float target_y;
float target_t;

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
    printf("\nGo for gyro cal:");
    go_click();
    pause(100);
    gyro_init(GYRO_PORT, LSB_US_PER_DEG, 500L);
    
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
    while (1) {
        // Navigate, dawg
        printf("Navigating, heading is %.2f...\r", gyro_get_degrees());
        pause(1000);
    }
    return 0;
}

