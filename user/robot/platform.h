#ifndef __PLATFORM_H
#define __PLATFORM_H

#include <stdint.h>
#include <lib/motor_group.h>

// Motor / encoder config
#define L_MOTOR_PORT 0
#define L_ENCODER_PORT 0
#define R_MOTOR_PORT 1
#define R_ENCODER_PORT 1

// Drivetrain config
#define ENCODER_TO_WHEEL_RATIO  3
#define WHEEL_CIRCUMFERENCE     10.0
#define WHEEL_TRACK             20.0
#include <lib/geartrain.h>

// Gyro config
#define GYRO_PORT           23
#define LSB_US_PER_DEG      1496152

/* Exported globals */

// Motor controllers
static MotorGroup motor_left = 1 << L_MOTOR_PORT;;
static MotorGroup motor_right = 1 << R_MOTOR_PORT;

/* Control functions */
void setLRMotors(int16_t l_vel, int16_t r_vel);

/* Init */
void platform_init(void);

#endif