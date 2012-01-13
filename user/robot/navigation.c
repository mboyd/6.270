/*
 * Navigation / localization controller
 */
#include "navigation.h"
#include "platform.h"

#include <joyos.h>
#include <lib/motion.h>
#include <math.h>

// Tunable parameters
#define NAV_ROT_KP              0
#define NAV_ROT_KI              0
#define NAV_ROT_KD              0

#define NAV_DRV_KP              0
#define NAV_DRV_KI              0
#define NAV_DRV_KD              0

#define NAV_FWD_GAIN            50

// "Close-enough" angle and distance
// POS_EPS is a distance squared
#define NAV_POS_EPS             4.0
#define NAV_ANG_EPS             5.0

// Maximum angular error before switching from front drive
// to in-place pivot (degrees)
#define NAV_ANG_DRV_LMT         20.0

#define NAV_THREAD_PRIORITY     100

// Navigation target
float target_x;
float target_y;
float target_t;
float target_v;

// Navigation position
float current_x;
float current_y;
float current_t;

// Drive setpoints
int16_t left_setpoint;
int16_t right_setpoint;

// Internal nav state
uint8_t nav_thread_id;
struct lock nav_data_lock;
struct lock nav_done_lock;

enum nav_state_t {ROTATE, DRIVE};
enum nav_state_t nav_state; 

struct pid_controller rotate_pid;
struct pid_controller drive_pid;

/*
 * API calls
 */
 
/* High-level commands */

void turnToHeading(float t) {
    acquire(&nav_data_lock);
    target_x = current_x;
    target_y = current_y;
    
    target_t = t;
    release(&nav_data_lock);
}

void turnToPoint(float x, float y) {
    acquire(&nav_data_lock);
    
    float t = atan2(x - current_x, y - current_y);
    turnToHeading(t);
    release(&nav_data_lock);
}

void moveToPoint(float x, float y, float v) {
    acquire(&nav_data_lock);
    target_x = x;
    target_y = y;
    target_t = atan2(x - current_x, y - current_y);
    target_v = v;
    release(&nav_data_lock);
}

void setTarget(float x, float y, float t, float v) {
    acquire(&nav_data_lock);
    target_x = x;
    target_y = y;
    target_t = t;
    target_v = v;
    release(&nav_data_lock);
}

/* Navigation status */

void getPosition(float *x, float *y, float *t) {
    acquire(&nav_data_lock);
    *x = current_x;
    *y = current_y;
    *t = current_t;
    release(&nav_data_lock);
}

int isMovementComplete(void) {
    return !is_held(&nav_done_lock);
}

void waitForMovementComplete(void) {
    acquire(&nav_done_lock);
    release(&nav_done_lock);
}


/*
 * PID control functions
 */

float rotate_pid_input(void) {
    // TODO: sign errors
    return current_t - target_t;
}

void rotate_pid_output(float output) {
    // TODO: sign errors
    left_setpoint += output;
    right_setpoint -= output;
}

float drive_pid_input(void) {
    // TODO: sign errors
    uint16_t l_enc = encoder_read(L_ENCODER_PORT);
    encoder_reset(L_ENCODER_PORT);
    
    uint16_t r_enc = encoder_read(R_ENCODER_PORT);
    encoder_reset(R_ENCODER_PORT);
    
    return (float) (l_enc - r_enc);
}

void drive_pid_output(float output) {
    // TOOD: sign errors
    left_setpoint += output;
    right_setpoint -= output;
}




/*
 * Nav thread initialization and main loop
 */

int nav_init(void) {
    
    target_x = 0; target_y = 0; target_t = 0;
    
    init_lock(&nav_data_lock, "nav_data_lock");
    init_lock(&nav_done_lock, "nav_done_lock");
    
    init_pid(&rotate_pid, NAV_ROT_KP, NAV_ROT_KI, NAV_ROT_KD,
                rotate_pid_input, rotate_pid_output);
    init_pid(&rotate_pid, NAV_DRV_KP, NAV_DRV_KI, NAV_DRV_KD,
                drive_pid_input, drive_pid_output);
    
    nav_state = ROTATE;
    
    return 0;
}

int nav_start(void) {
    nav_thread_id = create_thread(nav_loop, 
                STACK_DEFAULT, NAV_THREAD_PRIORITY, "nav_loop");
    return 0;
}

int nav_loop(void) {
    acquire(&nav_done_lock);
    while (1) {
        acquire(&nav_data_lock);
        if (!is_held(&nav_done_lock)) {
            acquire(&nav_done_lock);
        }
        
        left_setpoint = 0;
        right_setpoint = 0;
        
        // Update position estimate
        int16_t l_enc = encoder_read(L_ENCODER_PORT);
        int16_t r_enc = encoder_read(R_ENCODER_PORT);
        float enc_dist = (l_enc + r_enc) / (2.0 * TICKS_PER_CM);
        
        current_x += enc_dist * cos(current_t); // Use old heading
        current_y += enc_dist * sin(current_t);
        
        current_t = gyro_get_degrees();
        printf("X: %.2f \tY: %.2f \tHeading: %.2f \t\t\r", current_x, current_y, current_t);
        
        if (nav_state == ROTATE) {
            if (fmod(fabs(current_t - target_t), 360) <= NAV_ANG_EPS) {
                nav_state = DRIVE;
                continue;
            }
            
            update_pid(&rotate_pid);
        
        } else if (nav_state == DRIVE) {
            float dist = square(current_x - target_x) + 
                        square(current_y - target_y);
            
            if (dist <= NAV_POS_EPS) {
                // Done
                release(&nav_done_lock);
                continue;
            }
            
            if (fmod(fabs(current_t - target_t), 360) <= NAV_ANG_DRV_LMT) {
                nav_state = ROTATE;
                continue;
            }
            
            float forward_vel = fmax(target_v, dist * NAV_FWD_GAIN);
            
            left_setpoint = forward_vel;
            right_setpoint = forward_vel;
            
            update_pid(&rotate_pid);
            update_pid(&drive_pid);
            
        }
        
        release(&nav_data_lock);
        
        setLRMotors(left_setpoint, right_setpoint);
        
        pause(50);
    }
    return 0;
}

