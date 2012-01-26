/*
 * Navigation / localization controller
 */
#include "navigation.h"
#include "platform.h"
#include "util.h"

#include <joyos.h>
#include <lib/motion.h>
#include <math.h>

// Navigation target
float target_x;
float target_y;
float target_t;
float target_v;

// Navigation position
float current_x;
float current_y;
float current_t;

// Current drive setpoints
int16_t left_setpoint;
int16_t right_setpoint;

// Setpoint histories
rolling_buffer_f l_setpoint_hist;
rolling_buffer_f r_setpoint_hist;

// Nav system options
int heading_locked;

// Internal nav state
uint8_t nav_thread_id;
struct lock nav_data_lock;
struct lock nav_done_lock;

enum nav_state_t {ROTATE_ONLY, ROTATE, DRIVE, DONE};
enum nav_state_t nav_state;

uint32_t vps_last_update;
float last_x = 0;
float last_y = 0;

struct pid_controller rotate_pid;
struct pid_controller drive_pid;

float normalize_angle(float angle) {
    while (angle > 360) {
        angle -= 360;
    }
    
    while (angle <= 0) {
        angle += 360;
    }
    return angle;
}

float angle_difference(float a1, float a2) {
    float t = normalize_angle(a1 - a2);
    if (t > 180) {
        return 360 - t;
    } else {
        return t;
    }

}

/*
 * API calls
 */
 
/* High-level commands */

void turnToHeading(float t) {
    acquire(&nav_data_lock);
    
    setTargetDirected(current_x, current_y, t, target_v, 0);
    nav_state = ROTATE_ONLY;    // Override setTarget to prevent forward drive
    release(&nav_data_lock);
}

void turnToPoint(float x, float y) {
    acquire(&nav_data_lock);
    
    float t = atan2(y - current_y, x - current_x) * 180 / M_PI;
    setTargetDirected(current_x, current_y, t, target_v, platform_reverse);
    nav_state = ROTATE_ONLY;
    release(&nav_data_lock);
}

void moveToPoint(float x, float y, float v) {
    acquire(&nav_data_lock);
    
    float t = atan2(y - current_y, x - current_x) * 180 / M_PI;
    setTarget(x, y, t, v);
    
    release(&nav_data_lock);
}

void moveToPointDirected(float x, float y, float v, int reverse) {
    acquire(&nav_data_lock);
    
    float t = atan2(y - current_y, x - current_x) * 180 / M_PI;
    setTargetDirected(x, y, t, v, reverse);
    
    release(&nav_data_lock);
}

void setTarget(float x, float y, float t, float v) {
    acquire(&nav_data_lock);
    target_x = x;
    target_y = y;
    target_t = normalize_angle(t);
    target_v = v;
    
    if (angle_difference(target_t, current_t) > 90) {
        setReversed(!platform_reverse);
    }
    
    nav_state = ROTATE;
    
    release(&nav_data_lock);
}

void setTargetDirected(float x, float y, float t, float v, int reverse) {
    acquire(&nav_data_lock);
    target_x = x;
    target_y = y;
    target_t = normalize_angle(t);
    target_v = v;
    
    setReversed(reverse);
    
    nav_state = ROTATE;
    
    release(&nav_data_lock);
}

void setHeadingLock(int locked) {
    heading_locked = locked;
}

/* Navigation status */

void getPosition(float *x, float *y, float *t) {
    acquire(&nav_data_lock);
    *x = current_x;
    *y = current_y;
    *t = current_t;
    release(&nav_data_lock);
}

int rotationComplete(void) {
    int done;
    acquire(&nav_data_lock);
    done = (nav_state != ROTATE);
    release(&nav_data_lock);
    return done;
}

void waitForRotation(void) {
    while (!rotationComplete()) {
        yield();
    }
}

int movementComplete(void) {
    int done;
    acquire(&nav_data_lock);
    done = (nav_state == DONE);
    release(&nav_data_lock);
    return done;
}

void waitForMovement(void) {
    while (!movementComplete()) {
        yield();
    }
}


/*
 * PID control functions
 */

float rotate_pid_input(void) {
    float temp_t = fmod(current_t - target_t, 360);
    if(temp_t > 180)
    {
        temp_t -= 360;
    }
    else if(temp_t < -180)
    {
        temp_t += 360;
    }
    return temp_t;
}

void rotate_pid_output(float output) {
    if (!platform_reverse) {
        if (output > NAV_MAX_ROT) {
            output = NAV_MAX_ROT;
        } else if (output < -NAV_MAX_ROT) {
            output = -NAV_MAX_ROT;
        }
    } else {
         if (output > NAV_MAX_RVS_ROT) {
                output = NAV_MAX_RVS_ROT;
            } else if (output < -NAV_MAX_RVS_ROT) {
                output = -NAV_MAX_RVS_ROT;
            }
    }
    
    left_setpoint += output;
    right_setpoint -= output;
}

/*
 * VPS management
 */

void vps_update(void) {
    float x = ((float) game.coords[0].x);
    float y = ((float) game.coords[0].y);
    float t = ((float) game.coords[0].theta) * 360.0 / 4096.0;
    
    // VPS angular correction
    // 443.4units/ft 129in alt
    float vps_r = sqrt(x*x + y*y);
    float vps_corr = (443.4) * (vps_r / 4766.55);
    
    float vps_t = atan2(y, x);
    
    x -= cos(vps_t) * vps_corr;
    y -= sin(vps_t) * vps_corr;
    
    if (t < 0) {
        t += 360;
    }
    
    float vps_dist = sqrt(square(x - last_x) + square(y - last_y));
        
    last_x = x;
    last_y = y;
    
    //printf("VPS correction dX: %.2f\tdY: %.2f\tdT: %.2f\n", x-current_x, y-current_y, t-current_t);
    
    current_x = x;
    current_y = y;
    current_t = t;
    
    vps_last_update = position_microtime;
}



/*
 * Nav thread initialization and main loop
 */

int nav_init(void) {
    
    target_x = 0; target_y = 0; target_t = 0;
    
    rolling_buffer_f_new(&l_setpoint_hist, 3);
    rolling_buffer_f_new(&r_setpoint_hist, 3);
    
    heading_locked = 0;
    
    init_lock(&nav_data_lock, "nav_data_lock");
    init_lock(&nav_done_lock, "nav_done_lock");
    
    init_pid(&rotate_pid, NAV_ROT_KP, NAV_ROT_KI, NAV_ROT_KD,
                rotate_pid_input, rotate_pid_output);
    rotate_pid.goal = 0;
    rotate_pid.enabled = 1;
    
    nav_state = ROTATE;
    
    vps_last_update = position_microtime;
    uint32_t start_time = get_time_us();
    while (vps_last_update == position_microtime) {  // Wait for VPS update
        copy_objects();  
        if (get_time_us() - start_time > 1000000) {     // Timeout after 1 sec
            break;
        }
    }
    
    if (vps_last_update != position_microtime) {
        vps_update();
        gyro_set_degrees(current_t);
    } 
    return 0;
}

void gyro_sync(void) {
    acquire(&nav_data_lock);
    copy_objects();
    vps_update();
    gyro_set_degrees(current_t);
    release(&nav_data_lock);
}

int nav_start(void) {
    nav_thread_id = create_thread(nav_loop, 
                STACK_DEFAULT, NAV_THREAD_PRIORITY, "nav_loop");
    return 0;
}

void update_position() {
    
    if (nav_state == DRIVE && !heading_locked) {
        target_t = atan2((target_y-current_y),(target_x-current_x)) * 180 / M_PI;
    }
        
    // Check for new VPS fix
    copy_objects();
    if (position_microtime != vps_last_update) {
        //printf("New VPS fix: dt %u usec.\n", position_microtime - vps_last_update);
        vps_update();
    }
    
    current_t = normalize_angle(getHeading());
    
    //printf("X: %.2f \tY: %.2f \tT: %.2f \t\t\n", current_x, current_y, current_t);
    //printf("Heading: %.2f\r", current_t);
}

int nav_loop(void) {
    acquire(&nav_done_lock);
    while (1) {
        acquire(&nav_data_lock);
        if (!is_held(&nav_done_lock)) {
            acquire(&nav_done_lock);
        }
        
        //printf("Target x: %.2f\tTarget y: %.2f\tTarget t: %.2f\n", target_x, target_y, target_t);
                
        left_setpoint = 0;
        right_setpoint = 0;
        
        // Update position estimate
        update_position();
        
        float dist = sqrt(square(current_x - target_x) + 
                    square(current_y - target_y));
                    
        if (nav_state == DRIVE && dist <= NAV_POS_EPS) {
            release(&nav_done_lock);
            nav_state = DONE;
        }
        
        // Change states if necessary

        if (nav_state == ROTATE_ONLY) {

            if (angle_difference(current_t, target_t) <= NAV_ANG_EPS) {
                nav_state = DONE;
            }
            
        } else if (nav_state == ROTATE) {
            
            if (angle_difference(current_t, target_t) <= NAV_ANG_DRV_LMT) {         
                nav_state = DRIVE;
            }
        
        } else if (nav_state == DRIVE) {
                        
            //printf("Nav deviation: %.2f\n", angle_difference(current_t, target_t));
            
            if (angle_difference(current_t, target_t) >= NAV_ANG_DRV_LMT) {
                nav_state = ROTATE;
                //printf("Nav angle deviation exceeded\n");
            }
            
        }
        
        // Execute
        
        if (nav_state == ROTATE || nav_state == ROTATE_ONLY) {
            update_pid(&rotate_pid);
            //printf("Rotating\n");
            
        } else if (nav_state == DRIVE) {
            //printf("Driving, %.2f cm to target\n", dist);
            
            float forward_vel = fmin(target_v, dist * NAV_FWD_GAIN);
    	    left_setpoint = forward_vel;
            right_setpoint = forward_vel;
	        //printf("forward vel is %.2f \t targetv is %.2f \n", forward_vel, target_v);
	        
            update_pid(&rotate_pid);
    	    //update_pid(&drive_pid);
        }
        
        release(&nav_data_lock);
        
        rolling_buffer_f_add(&l_setpoint_hist, left_setpoint);
        rolling_buffer_f_add(&r_setpoint_hist, right_setpoint);
        
        float l = rolling_buffer_f_avg(&l_setpoint_hist);
        float r = rolling_buffer_f_avg(&r_setpoint_hist);
        
        //printf("L/R Setpoints: %i / %i\n", l, r);

        setLRMotors(left_setpoint, right_setpoint);
	
    }
    return 0;
}

