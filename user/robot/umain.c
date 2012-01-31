#include <joyos.h>
#include "platform.h"
#include "cannon.h"
#include "navigation.h"
#include "gameboard.h"
#include <math.h>

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
    point_t p1 = gearboxOffset(570, -10, territory);
    point_t p2 = gearboxOffset(365, -30, territory);
    
    uint8_t failure_count = 0;
    
    moveToPoint(p1.x, p1.y, 150);
    waitForMovement();
    
    engageGear:
    
    setReversed(0);
    turnToPoint(p2.x, p2.y);
    waitForMovement();
    
    setHeadingLock(1);  // Lock in the initial heading to target
    setRecoveryEnabled(0);
    moveToPointDirected(p2.x, p2.y, 110, 0);

    if (team == RED) {
        motor_set_vel(GEAR_MOTOR_PORT, -255);
    } else {
        motor_set_vel(GEAR_MOTOR_PORT, 255);
    }
    
    uint32_t start_time = get_time_us();
    
    uint32_t timeout = 2200000;
    
    if (game.territories[territory].owner) {
        timeout = 3200000;
    }
    
    while (game.territories[territory].owner != robot_id && \
        (get_time_us() - start_time) < timeout) {
            
            copy_objects();
            
    }
    
    motor_set_vel(GEAR_MOTOR_PORT, 0);
    setRecoveryEnabled(1);
    setHeadingLock(0);
    
    pauseMovement();    // Cut out the nav controller
    motor_set_vel(L_MOTOR_PORT, -150);  // And back straight out for a bit
    motor_set_vel(R_MOTOR_PORT, -150);
    pause(500);
    unpauseMovement();
    
    if (game.territories[territory].owner != robot_id) {
        // Failed to capture
        if ((++failure_count) == 3) {
            // Failed 3 times, bail
            return;
        }
        
        moveToPointDirected(p1.x, p1.y, 110, 0);
        waitForMovement();
        
        pauseMovement();
        pause(300);
        gyro_sync();
        unpauseMovement();
        goto engageGear;
    }
}

void mine_territory_fallback(uint8_t territory) {
    uint8_t failure_count = 0;
    
    point_t p1 = leverOffset(550, 0, territory);
    point_t p2 = leverOffset(247, 0, territory);
    
    moveToPoint(p1.x, p1.y, 150);                   // Move to staging point
    waitForMovement();
    
    engageLever_fallback:
    
    setReversed(1);                                 // Turn to the lever
    turnToPoint(p2.x, p2.y);
    waitForMovement();
    
    setHeadingLock(1);                              // Back into the lever
    moveToPointDirected(p2.x, p2.y, 100, 1);
    waitForMovement();
    
    setRecoveryEnabled(0);
    pauseMovement();
    
    copy_objects();
            
    uint8_t initial_count = game.territories[territory].remaining;
    uint8_t balls_retrieved = 0;
    
    copy_objects();
    
    while (game.territories[territory].remaining) {
        leverDown();        // Pull the lever
        pause(400);
        leverUp();
        pause(600);         // Wait for ball to drop
        balls_retrieved++;
        
        copy_objects();
        
        if (game.territories[territory].remaining != (initial_count - balls_retrieved)) {
            
            if (game.territories[territory].owner != robot_id) {
                // This territory has been captured out from under us, bail
                goto cleanup_fallback;
            }
            
            // We're missing the lever, back out and retry with the fallback points
            failure_count++;
            
            if (failure_count == 2) {
                // We've already failed once, bail
                goto cleanup_fallback;
            }
            
            cannon_fire_wait();
            
            unpauseMovement();
            setHeadingLock(0);
            moveToPointDirected(p1.x, p1.y, 110, 0);
            waitForMovement();
            
            goto engageLever_fallback;
        }
        
        if (balls_retrieved == 3) {
            unpauseMovement();
            moveToPointDirected(p1.x, p1.y, 100, 0);
            waitForMovement();
            setReversed(0);
            turnToPoint(targets[0].x, targets[0].y);
            waitForMovement();
            
            pauseMovement();
            setTargetReady(1);
            cannon_fire_wait();
            
            unpauseMovement();
            setTargetReady(0);
            
            moveToPointDirected(p2.x, p2.y, 100, 1);
            waitForMovement();
        }
    }
    
    unpauseMovement();
    moveToPointDirected(p1.x, p1.y, 100, 0);
    waitForMovement();
    setReversed(0);
    turnToPoint(targets[0].x, targets[0].y);
    waitForMovement();
    
    pauseMovement();
    setTargetReady(1);
    
    cannon_fire_wait();
    
    unpauseMovement();
    
    setTargetReady(0);
    
    copy_objects();
    gyro_sync();
    
    setRecoveryEnabled(1);
    setHeadingLock(0);
    return;
        
    cleanup_fallback:
    
    setTargetReady(0);
    setRecoveryEnabled(1);
    unpauseMovement();

    setHeadingLock(0);
    moveToPointDirected(p1.x, p1.y, 110, 0);
    waitForMovement();
    
}

void mine_territory(uint8_t territory) {
    uint8_t failure_count = 0;
    
    point_t offset;
    
    if (team == BLUE) {
        offset = blue_lever_offsets[territory];
    } else {
        offset = red_lever_offsets[territory];
    }
    
    point_t p1 = leverTargetOffset(offset.x + 300, offset.y, territory); // Staging point
    point_t p2 = leverTargetOffset(offset.x, offset.y, territory); // Target point
    
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
    
    setTargetReady(1);
    setRecoveryEnabled(0);
    pauseMovement();
    
    copy_objects();
    
    //pause(game.territories[territory].rate_limit * 1000);
        
    uint8_t initial_count = game.territories[territory].remaining;
    uint8_t balls_retrieved = 0;
    
    copy_objects();
    
    while (game.territories[territory].remaining) {
        leverDown();        // Pull the lever
        pause(400);
        leverUp();
        pause(600);         // Wait for ball to drop
        balls_retrieved++;
        
        copy_objects();
        
        if (game.territories[territory].remaining != (initial_count - balls_retrieved)) {
            
            if (game.territories[territory].owner != robot_id) {
                // This territory has been captured out from under us, bail
                goto cleanup;
            }
            
            // We're missing the lever, back out and retry with the fallback points
            failure_count++;
            
            cannon_fire_wait();
            
            unpauseMovement();
            setHeadingLock(0);
            setTargetReady(0);
            setRecoveryEnabled(1);
            
            mine_territory_fallback(territory);
            return;
        }
        
        if (balls_retrieved == 2) {
            pause(700);     // Let the cannon catch up
        }
    }
    
    
    cannon_fire_wait();
    
    copy_objects();
    gyro_sync();
    
    uint8_t obtained = initial_count - game.territories[territory].remaining;
    
    cleanup:
    
    setTargetReady(0);
    setRecoveryEnabled(1);
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
    }*/
    
    
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
        
    uint8_t current_territory;
    
    if (team == BLUE) {
        current_territory = 0;
    } else {
        current_territory = 3;
    }
    
    uint32_t start_time = get_time_us();
    int dir = 1;
    
    while ((get_time_us() - start_time) < 8000000) {
        uint8_t op = opponentPosition();
        
        if (op == current_territory + dir) {
            dir = -dir;
        }
        
        current_territory = (current_territory + 6 + dir) % 6;
        
        moveToPointDirected(centers[current_territory].x, centers[current_territory].y, 180, 0);
        waitForMovement();   
    }
            
    capture_territory(current_territory);
    
    if (game.territories[current_territory].owner == robot_id) {
        mine_territory(current_territory);
    }
    
    while (1) {
        copy_objects();
        // In general, we prefer moving clockwise,
        // as that's the orientation of the gearbox -> lever path
        
        uint8_t a2 = (current_territory + 1) % 6;
        uint8_t a1 = (current_territory + 5) % 6;
        
        uint8_t op = opponentPosition();
        
        uint8_t dest = a1;      // Set default, just in case
        
        territory_data t1 = game.territories[a1];
        territory_data t2 = game.territories[a2];
        
        // Avoid the opponent above all else
        // (don't want to get stuck or broken)
        if (op == a1) {
            dest = a2;
        
        } else if (op == a2) {
            dest = a1;
        
        // If we can make an easy buck...
        } else if (t1.owner == robot_id && 
                (t1.remaining > 2)) {
        
            dest = a1;
        
        
        } else if (t2.owner == robot_id && 
                (t2.remaining > 2)) {
        
            dest = a2;
        
        // Else, go for the opponent's territories (that they're not in)
        } else if (t1.owner != robot_id && t1.owner != 0) {
            
            dest = a1;
        
        } else if (t2.owner != robot_id && t2.owner != 0) {
            
            dest = a2;
        }
        
        if (game.territories[dest].owner != robot_id) {
            
            capture_territory(dest);
        }
        
        uint8_t pos = closestTerritory();
                
        if ((pos+2)%6 == dest) {
            moveToPointDirected(centers[(pos+1)%6].x, centers[(pos+1)%6].y, 180, 0);
            waitForMovement();
        } else if ((pos+4)%6 == dest) {
            moveToPointDirected(centers[(pos+5)%6].x, centers[(pos+5)%6].y, 180, 0);
            waitForMovement();
        }
        
        if (game.territories[dest].owner == robot_id && 
            (game.territories[dest].remaining)) {
            
            mine_territory(dest);
        
        }   // If we didn't capture successfully, just go on
        
        // If we couldn't do anything (forced by opponent), be sure and move
        if (game.territories[dest].owner == robot_id && 
            (!game.territories[dest].remaining)) {
            
            moveToPointDirected(centers[dest].x, centers[dest].y, 180, 0);
            waitForMovement();
        }
        
        current_territory = dest;
        
    }
    
    return 0;
}

int uend(void) {
    turnToHeading(0);
    waitForMovement();
    
    return 0;
}
