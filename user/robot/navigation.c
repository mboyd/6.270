/*
 * Navigation / localization controller
 */
#include "navigation.h"
#include "platform.h"
#include "gameboard.h"

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

// Nav system options
int heading_locked;     // Do not recalculate heading to target point when driving;
                        // useful for approaches more sensitive to angular error than
                        // lateral offset.

int fast_drive;         // Do not attempt to come to a full stop at the target point;
                        // useful when target specifies a waypoint along a smooth path
                        // and we can drive through it without stopping.

int recovery_enabled;   // Enable the auto-recovery thread

// Internal nav state
uint8_t nav_thread_id;
struct lock *nav_data_lock;
struct lock *nav_done_lock;

enum nav_state_t nav_state;

uint32_t vps_last_update;

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
    acquire(nav_data_lock);
    
    setTargetDirected(current_x, current_y, t, target_v, 0);
    nav_state = ROTATE_ONLY;    // Override setTarget to prevent forward drive
    release(nav_data_lock);
}

void turnToPoint(float x, float y) {
    acquire(nav_data_lock);
    
    float t = atan2(y - current_y, x - current_x) * 180 / M_PI;
    setTargetDirected(current_x, current_y, t, target_v, platform_reverse);
    nav_state = ROTATE_ONLY;
    release(nav_data_lock);
}

void moveToPoint(float x, float y, float v) {
    acquire(nav_data_lock);
    
    float t = atan2(y - current_y, x - current_x) * 180 / M_PI;
    setTarget(x, y, t, v);
    
    release(nav_data_lock);
}

void moveToPointDirected(float x, float y, float v, int reverse) {
    acquire(nav_data_lock);
    
    float t = atan2(y - current_y, x - current_x) * 180 / M_PI;
    setTargetDirected(x, y, t, v, reverse);
    
    release(nav_data_lock);
}

void setTarget(float x, float y, float t, float v) {
    acquire(nav_data_lock);
    target_x = x;
    target_y = y;
    target_t = normalize_angle(t);
    target_v = v;
    
    float dist = sqrt(square(current_x - target_x) + square(current_y - target_y));
    
    if (angle_difference(target_t, current_t) > 90) {
        setReversed(!platform_reverse);
    }
    
    nav_state = ROTATE;
    
    release(nav_data_lock);
}

void setTargetDirected(float x, float y, float t, float v, int reverse) {
    acquire(nav_data_lock);
    target_x = x;
    target_y = y;
    target_t = normalize_angle(t);
    target_v = v;
    
    setReversed(reverse);
    
    nav_state = ROTATE;
    
    release(nav_data_lock);
}

/* Nav options */

void setHeadingLock(int locked) {
    heading_locked = locked;
}

void setFastDrive(int fastDrive) {
    fast_drive = fastDrive;
}

void setRecoveryEnabled(int recoveryEnabled) {
    recovery_enabled = recoveryEnabled;
}

/* Navigation status */

void getPosition(float *x, float *y, float *t) {
    acquire(nav_data_lock);
    *x = current_x;
    *y = current_y;
    *t = current_t;
    release(nav_data_lock);
}

int rotationComplete(void) {
    int done;
    acquire(nav_data_lock);
    done = (nav_state != ROTATE);
    release(nav_data_lock);
    return done;
}

void waitForRotation(void) {
    while (!rotationComplete()) {
        yield();
    }
}

int movementComplete(void) {
    int done;
    acquire(nav_done_lock);
    done = (nav_state == DONE);
    release(nav_done_lock);
    return done;
}

void waitForMovement(void) {
    while (!movementComplete()) {
        pause(500);
    }
}

struct nav_system_state_t getNavSystemState(void) {
    struct nav_system_state_t s;
    acquire(nav_data_lock);
    s.target_x = target_x;
    s.target_y = target_y;
    s.target_t = target_t;
    s.target_v = target_v;
    s.nav_state = nav_state;
    s.nav_done_lock = nav_done_lock;
    
    nav_done_lock = malloc(sizeof(struct lock));
    init_lock(nav_done_lock, "nav_done_lock");
    
    release(nav_data_lock);
    return s;
}
void setNavSystemState(struct nav_system_state_t s) {
    acquire(nav_data_lock);
    target_x = s.target_x;
    target_y = s.target_y;
    target_t = s.target_t;
    target_v = s.target_v;
    nav_state = s.nav_state;
    
    free(nav_done_lock);
    
    nav_done_lock = s.nav_done_lock;
    release(nav_data_lock);
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
    
    if (abs(output) < NAV_MIN_ROT && (nav_state == ROTATE || nav_state == ROTATE_ONLY)) {
        if (output > 0) {
            output = NAV_MIN_ROT;
        } else {
            output = -NAV_MIN_ROT;
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
    float vps_corr = 1 - ((443.4 * 35 / 38) / (4766.55));
        
    x *= vps_corr;
    y *= vps_corr;
        
    if (t < 0) {
        t += 360;
    }
    
    current_x = x;
    current_y = y;
    current_t = t;
    
    //decomposeLeverTarget(x, y);
    
    vps_last_update = position_microtime;
}



/*
 * Nav thread initialization and main loop
 */

int nav_init(void) {
    
    target_x = 0; target_y = 0; target_t = 0;
    
    heading_locked = 0;
    fast_drive = 0;
    recovery_enabled = 1;
    
    nav_data_lock = malloc(sizeof(struct lock));
    nav_done_lock = malloc(sizeof(struct lock));
    
    init_lock(nav_data_lock, "nav_data_lock");
    init_lock(nav_done_lock, "nav_done_lock");
    
    init_pid(&rotate_pid, NAV_ROT_KP, NAV_ROT_KI, NAV_ROT_KD,
                rotate_pid_input, rotate_pid_output);
    rotate_pid.goal = 0;
    rotate_pid.enabled = 1;
    
    nav_state = DONE;
    
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
    acquire(nav_data_lock);
    copy_objects();
    vps_update();
    gyro_set_degrees(current_t);
    release(nav_data_lock);
}

int nav_start(void) {
    nav_thread_id = create_thread(nav_loop, 
                STACK_DEFAULT, NAV_THREAD_PRIORITY, "nav_loop");
                
    create_thread(recovery_loop, 
                STACK_DEFAULT, 20, "recovery_loop");
    
    return 0;
}

void update_position() {
    
    if (nav_state == DRIVE && !heading_locked) {
        target_t = atan2((target_y-current_y),(target_x-current_x)) * 180 / M_PI;
    }
        
    // Check for new VPS fix
    copy_objects();
    if (position_microtime != vps_last_update) {
        vps_update();
    }
    
    current_t = normalize_angle(getHeading());
}

int nav_loop(void) {
    acquire(nav_done_lock);
    while (1) {
        acquire(nav_data_lock);
                        
        left_setpoint = 0;
        right_setpoint = 0;
        
        // Update position estimate
        update_position();
        
        float dist = sqrt(square(current_x - target_x) + 
                    square(current_y - target_y));
                    
        if (nav_state == DRIVE && dist <= NAV_POS_EPS) {  
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
                                    
            if (angle_difference(current_t, target_t) >= NAV_ANG_DRV_LMT) {
                nav_state = ROTATE;
            }
            
        }
        
        // Execute
        
        if (nav_state == ROTATE || nav_state == ROTATE_ONLY) {
            update_pid(&rotate_pid);
            
        } else if (nav_state == DRIVE) {
            
            float forward_vel;
            if (!fast_drive) {
                forward_vel = fmin(target_v, dist * NAV_FWD_GAIN);
            } else {
                forward_vel = target_v;
            }
    	    
    	    left_setpoint = forward_vel;
            right_setpoint = forward_vel;
	        
            update_pid(&rotate_pid);
        }
        
        if (nav_state == DONE) {
            if (is_held(nav_done_lock)) {
                release(nav_done_lock);
            }
        } else {
            if (!is_held(nav_done_lock)) {
                acquire(nav_done_lock);
            }
        }
        
        release(nav_data_lock);
        
        setLRMotors(left_setpoint, right_setpoint);
	
    }
    return 0;
}

int recovery_loop(void) {
    float last_x = 0;
    float last_y = 0;
    float last_t = 0;
    
    while (1) {
        acquire(nav_data_lock);
        float dist = sqrt(square(current_x - last_x) + square(current_y - last_y));
        
        if (recovery_enabled && dist < NAV_POS_EPS && (current_t - last_t) < NAV_ANG_EPS*7 && nav_state != DONE) {
            // Stuck, hit a wall, etc
            pauseMovement();    // Stop what we're doing

            int vel = -200;
            if (platform_reverse) {
                vel = 200;
            }
            motor_set_vel(L_MOTOR_PORT, vel);
            motor_set_vel(R_MOTOR_PORT, vel);
            
            pause(1000);    // Wait a tick
            
            motor_set_vel(L_MOTOR_PORT, 0);
            motor_set_vel(R_MOTOR_PORT, 0);
            
            pause(300);
            gyro_sync();
            
            unpauseMovement();      // Resume normal movement
            
        }
        
        last_x = current_x;
        last_y = current_y;
        last_t = current_t;
        
        release(nav_data_lock);
        pause(4500);
    }
    return 0;
}

