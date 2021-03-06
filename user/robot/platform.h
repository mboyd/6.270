#ifndef __PLATFORM_H
#define __PLATFORM_H

#include <stdint.h>
#include <lib/motor_group.h>

#define SETPOINT_MAX_DERIV  100
#define SETPOINT_MIN_VEL    38

// Motor config
#define L_MOTOR_PORT 1
#define R_MOTOR_PORT 0

#define GEAR_MOTOR_PORT     2

#define CANNON_MOTOR_PORT   3
#define CANNON_ENCODER_PORT 24

// Gyro config
#define GYRO_PORT           8
#define LSB_US_PER_DEG      1496152

// Servo config
#define TRIGGER_SERVO_PORT  2
// Forward
#define TRIGGER_LOWER_LIMIT 227
// Back
#define TRIGGER_UPPER_LIMIT 368

#define LEVER_SERVO_PORT    0
// Up
#define LEVER_LOWER_LIMIT   220
// Down
#define LEVER_UPPER_LIMIT   511

#define CANNON_BREAKBEAM_PORT   12

/* Exported globals */
int platform_reverse;
int platform_pause;

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