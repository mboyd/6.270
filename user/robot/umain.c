#include <joyos.h>
#include "platform.h"
#include "cannon.h"
#include "navigation.h"
#include "gameboard.h"

extern volatile uint8_t robot_id;
enum team_t {RED, BLUE} team_t;
enum team_t team;

int usetup(void) {
    robot_id = 7;
        
    platform_init();
    cannon_init();
    nav_init();
    
    if (game.coords[0].x < 0) {
        team = RED;
        point_t temp = targets[0];
        targets[0] = targets[1];
        targets[1] = temp;
    } else {
        team = BLUE;
    }
    
    return 0;
}

void capture_territory(uint8_t territory) {
    point_t p1 = gearboxOffset(550, -70, territory);
    point_t p2 = gearboxOffset(365, -90, territory);
    
    uint8_t failure_count = 0;
    
    moveToPoint(p1.x, p1.y, 150);
    waitForMovement();
    
    
    engageGear:
    
    setReversed(0);
    turnToPoint(p2.x, p2.y);
    waitForMovement();
    
    setHeadingLock(1);  // Lock in the initial heading to target
    moveToPointDirected(p2.x, p2.y, 110, 0);

    if (team == RED) {
        motor_set_vel(GEAR_MOTOR_PORT, -255);
    } else {
        motor_set_vel(GEAR_MOTOR_PORT, 255);
    }
    
    uint32_t start_time = get_time_us();
    
    while (game.territories[territory].owner != robot_id && \
        (get_time_us() - start_time) < 2200000) {
            
            copy_objects();
            
    }
    
    motor_set_vel(GEAR_MOTOR_PORT, 0);
    setHeadingLock(0);
    
    pauseMovement();    // Cut out the nav controller
    motor_set_vel(L_MOTOR_PORT, -150);  // And back straight out for a bit
    motor_set_vel(R_MOTOR_PORT, -150);
    pause(500);
    unpauseMovement();
    
    if (game.territories[territory].owner != robot_id) {
        // Failed to capture
        /*if ((++failure_count) == 3) {
            // Failed 3 times, bail
            return;
        }*/
        
        moveToPoint(p1.x, p1.y, 110);
        waitForMovement();
        
        pauseMovement();
        pause(300);
        gyro_sync();
        unpauseMovement();
        goto engageGear;
    }
}

void mine_territory(uint8_t territory) {
    uint8_t failure_count = 0;
    
    point_t p1 = leverTargetOffset(550, territory); // Staging point
    point_t p2 = leverTargetOffset(260, territory); // Target point
    
    point_t p1_fallback = leverOffset(550, territory);
    point_t p2_fallback = leverOffset(260, territory);
    
    moveToPoint(p1.x, p1.y, 150);                   // Move to staging point
    waitForMovement();
    
    engageLever:
    
    setReversed(1);                                 // Turn to the lever
    turnToPoint(p2.x, p2.y);
    waitForMovement();
        
    setHeadingLock(1);                              // Back into the lever
    moveToPointDirected(p2.x, p2.y, 100, 1);
    waitForMovement();
    
    setReversed(0);
    turnToPoint(targets[0].x, targets[0].y);        // Turn to the target
    waitForMovement();
    
    pauseMovement();
        
    uint8_t initial_count = game.territories[territory].remaining;
    uint8_t balls_retrieved = 0;
    
    while (game.territories[territory].remaining) {
    //for (int i = 0; i < 5; i++) {
        leverDown();        // Pull the lever
        pause(400);
        leverUp();
        pause(600);         // Wait for ball to drop
        balls_retrieved++;
        
        if (game.territories[territory].remaining != (initial_count - balls_retrieved)) {
            // We're missing the lever, back out and retry with the fallback points
            failure_count++;
            
            if (failure_count == 2) {
                // We've already failed once, bail
                goto cleanup;
            }
            
            p1 = p1_fallback;
            p2 = p2_fallback;
            
            cannon_fire_wait();
            
            unpauseMovement();
            setHeadingLock(0);
            moveToPointDirected(p1.x, p1.y, 110, 0);
            waitForMovement();
            
            goto engageLever;
        }
        
        if (balls_retrieved == 2) {
            pause(700);     // Let the cannon catch up
        }
    }
    
    pause(400);
    
    cannon_fire_wait();
    
    copy_objects();
    gyro_sync();
    
    uint8_t obtained = initial_count - game.territories[territory].remaining;
    
    cleanup:
    
    unpauseMovement();

    setHeadingLock(0);
    moveToPointDirected(p1.x, p1.y, 110, 0);
    waitForMovement();
    
}

int umain(void) {
    printf("Hello, world!\n");
    
    /*  Servo calibration
    while (1) {
        int16_t i = frob_read_range(0, 511);
        servo_set_pos(TRIGGER_SERVO_PORT, i);
        printf("%.3i\r", i);
    }
    */
    
    /*  Cannon calibration
    cannon_start();

    for (int rpm = 800; rpm <= 2500; rpm += 100) {
        printf("Cannon setpoint: %.4i rpm.\r", rpm);
        while (1) {
            if (go_press()) {
                triggerForward();
                pause(300);
                triggerBack();
            } else if (stop_press()) {
                break;
            }
        } 
    }
    */
    
    nav_start();
    cannon_start();

    for (int i = 1; i < 5; i++) {
        moveToPoint(centers[i].x, centers[i].y, 200);
        waitForMovement();
    }
        
    for (int i = 5; i >= 0; i--) {
        capture_territory(i);
        mine_territory(i);
    }
    
    return 0;
}
