#include <joyos.h>
#include <stdint.h>

#include "platform.h"

void platform_init(void) {
    // Calibrate gyro
    printf("Calibrating gyro...");
    pause(100);
    gyro_init(GYRO_PORT, LSB_US_PER_DEG, 10000L);
    printf("done.\n");
}

void setLRMotors(int16_t l_vel, int16_t r_vel) {
    motor_group_set_vel(motor_left, l_vel);
    motor_group_set_vel(motor_right, r_vel);
}