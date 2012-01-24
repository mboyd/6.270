#include <joyos.h>
#include "platform.h"
#include "cannon.h"
#include "navigation.h"
#include "gameboard.h"

extern volatile uint8_t robot_id;

int usetup(void) {
    robot_id = 7;
    
    platform_init();
    cannon_init();
    nav_init();
    return 0;
}

int umain(void) {
    printf("Hello, world!\n");
    
    /*
    cannon_start();

    while (1) {
        int16_t dist = frob_read_range(0,255);
        
        printf("Distance: %.3i cm\n", dist);
        
        cannon_set_distance(dist);
        pause(300);
    }
    
    while (1) {
        int16_t val = frob_read_range(0, 255);
        motor_set_vel(0, val);
    }
    */
    
    nav_start();
    
    int point = 1;
    
    while (1) {
        copy_objects();
        int16_t t_x = game.coords[2].x;
        int16_t t_y = game.coords[2].y;
        
        float x = ((float) t_x) / VPS_PER_CM;
        float y = ((float) t_y) / VPS_PER_CM;
        
        uint32_t start_time = get_time_us();
        
        moveToPoint(x, y, 180);
        
        while (!movementComplete() && (t_x == game.coords[2].x && t_y == game.coords[2].y)) {
            pause(50);
            if (get_time_us() - start_time > 10000000) {
                pauseMovement();
                pause(300);
                gyro_sync();
                unpauseMovement();
            }
            copy_objects();
        }
        
        if ((point++) % 30 == 0) {
            pauseMovement();
            pause(300);
            gyro_sync();
            unpauseMovement();
        }
    }
    
    return 0;
}
