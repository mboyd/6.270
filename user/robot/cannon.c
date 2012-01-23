#include "cannon.h"
#include "platform.h"
#include <joyos.h>
#include <lib/pid.h>

float cannon_current_rpm;
float cannon_target_rpm;

float cannon_motor_setpoint;

uint32_t last_update;

struct pid_controller cannon_controller;

int cannon_thread_id;
struct lock cannon_data_lock;

void cannon_set_distance(float dist) {
    // Linear fit to calibration curve; see spreadsheet
    float rpm = 792.515 + 6.88129 * dist;
    cannon_set_rpm(rpm);
}

void cannon_set_rpm(float rpm) {
    acquire(&cannon_data_lock);
    cannon_target_rpm = rpm;
    release(&cannon_data_lock);
}

float cannon_get_rpm(void) {
    float rpm;
    acquire(&cannon_data_lock);
    rpm = cannon_current_rpm;
    release(&cannon_data_lock);
    return rpm;
}

void cannon_wait(void) {
    int done = 0;
    while (!done) {
        acquire(&cannon_data_lock);
        done = (fabs(cannon_current_rpm - cannon_target_rpm) < CANNON_RPM_EPS);
        release(&cannon_data_lock);
        pause(50);
    }
}

float cannon_pid_input(void) {
    float err = cannon_target_rpm - cannon_current_rpm;
    return err;
}

void cannon_pid_output(float out) {
    if (out > 255) {
        out = 255;
    }
    
    //cannon_motor_setpoint += out;
    cannon_motor_setpoint = out;
    
    printf("Cannon at %.2f RPM, motor setpoint %.3i\n", cannon_current_rpm, (int16_t) out);
    
    motor_set_vel(CANNON_MOTOR_PORT, (int16_t) cannon_motor_setpoint);   // FIXME: droop
}

int cannon_init(void) {
    cannon_target_rpm = 0;
    cannon_motor_setpoint = 0;
    
    init_pid(&cannon_controller, CANNON_RPM_KP, CANNON_RPM_KI, CANNON_RPM_KD,
                cannon_pid_input, cannon_pid_output);
    cannon_controller.goal = 0;
    cannon_controller.enabled = 1;
    
    return 0;
}

int cannon_start(void) {
    last_update = get_time_us();
    
    cannon_thread_id = create_thread(cannon_loop, 
                STACK_DEFAULT, CANNON_THREAD_PRIORITY, "cannon_loop");
    return 0;
}

int cannon_loop(void) {
    /*
     * Under typical conditions, the cannon behavior is well modeled by
     * (Top wheel RPM) = 2.53 * (Motor setpoint) - 113.76; or
     * (Motor setpoint) = 0.40 * (Top wheel RPM) + 45.03
     * However, we don't use this, instead driving with a PID controller.
     */
    
    
    while (1) {
        acquire(&cannon_data_lock);
        
        uint16_t revs = encoder_read(CANNON_ENCODER_PORT);
        uint32_t ellapsed = (get_time_us() - last_update);
        cannon_current_rpm = (60.0 * 1000000 / 6) * ((float) revs) / (ellapsed);
        
        update_pid(&cannon_controller);
        
        encoder_reset(CANNON_ENCODER_PORT);
        last_update = get_time_us();
        
        release(&cannon_data_lock);
        
        //pause(50 * (ellapsed / 1000) / ((float) revs));      // Wait for ~50 clicks
        pause(300);
    }
    
    return 0;
}
