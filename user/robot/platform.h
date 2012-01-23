#ifndef __PLATFORM_H
#define __PLATFORM_H

#include <stdint.h>
#include <lib/motor_group.h>

#define SETPOINT_MAX_DERIV  50

// Motor / encoder config
#define L_MOTOR_PORT 1
#define L_ENCODER_PORT 26
#define R_MOTOR_PORT 0
#define R_ENCODER_PORT 24

#define CANNON_MOTOR_PORT   0
#define CANNON_ENCODER_PORT 25

// Drivetrain config
#define ENCODER_TO_WHEEL_RATIO  4.166
#define WHEEL_CIRCUMFERENCE     25.6
#define WHEEL_TRACK             20.8 
#include <lib/geartrain.h>

// Gyro config
#define GYRO_PORT           23
#define LSB_US_PER_DEG      1496152

// Servo config
#define TRIGGER_SERVO_PORT  0
#define TRIGGER_LOWER_LIMIT 120
#define TRIGGER_UPPER_LIMIT 210

#define LEVER_SERVO_PORT    1
#define LEVER_LOWER_LIMIT   210
#define LEVER_UPPER_LIMIT   511

/* Exported globals */
int platform_reverse;
int platform_pause;

// Motor controllers
static MotorGroup motor_left = 1 << L_MOTOR_PORT;;
static MotorGroup motor_right = 1 << R_MOTOR_PORT;

/* Drivetrain control functions */
void setReversed(int reversed);

float getHeading(void);
void setLRMotors(int16_t l_vel, int16_t r_vel);
void pauseMovement(void);
void unpauseMovement(void);

/* Servo controls */
void triggerForward(void);
void triggerBack(void);
void setTriggerPosition(uint16_t pos);

void leverUp(void);
void leverDown(void);
void setLeverPosition(uint16_t pos);


/* Init */
void platform_init(void);

#endif