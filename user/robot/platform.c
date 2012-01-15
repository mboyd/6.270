#include <joyos.h>
#include <stdint.h>

#include "platform.h"

int platform_reverse;
int platform_pause;

void platform_init(void) {
    platform_reverse = 0;
    platform_pause = 0;
    
    // Calibrate gyro
    printf("Calibrating gyro...");
    pause(100);
    gyro_init(GYRO_PORT, LSB_US_PER_DEG, 1000L);
    printf("done.\n");
}

void pauseMovement() {
    platform_pause = 1;
    motor_brake(L_MOTOR_PORT);
    motor_brake(R_MOTOR_PORT);
}

void unpauseMovement() {
    platform_pause = 0;
}

void setLRMotors(int16_t l_vel, int16_t r_vel) {
    if (platform_pause) {
        return;
    }
    
    if (l_vel > 255) {
        l_vel = 255;
    } else if (l_vel < -255) {
        l_vel = -255;
    }
    
    if (r_vel > 255) {
        r_vel = 255;
    } else if (r_vel < -255) {
        r_vel = -255;
    }
    
    if (platform_reverse) {
        motor_group_set_vel(motor_left, -r_vel);
        motor_group_set_vel(motor_right, -l_vel);
    } else {
        motor_group_set_vel(motor_left, l_vel);
        motor_group_set_vel(motor_right, r_vel);
    }
}