#include <joyos.h>
#include "platform.h"
#include "navigation.h"

extern volatile uint8_t robot_id;

int usetup(void) {
    robot_id = 7;
    
    platform_init();
    nav_init();
    return 0;
}

int umain(void) {
    printf("Hello, world!\n");
    
    nav_start();
    
    //turnToHeading(0);
    //waitForMovement();
    //
    //float x, y, t;
    //getPosition(&x, &y, &t);
    //    
    //moveToPoint(x+30, y, 200);
    //waitForMovement();
    //
    //return 0;
    //
    //moveToPoint(30, 30, 200);
    //waitForMovement();
    //
    //moveToPoint(00, 30, 200);
    //waitForMovement();
    //
    //moveToPoint(00, 00, 200);
    //waitForMovement();
    
    while (1) {
        copy_objects();
        int16_t t_x = objects[2].x;
        int16_t t_y = objects[2].y;
        
        float x = ((float) t_x) / VPS_PER_CM;
        float y = ((float) t_y) / VPS_PER_CM;
        
        uint32_t start_time = get_time_us();
        
        moveToPoint(x, y, 150);
        
        while (!movementComplete() && (t_x == objects[2].x && t_y == objects[2].y)) {
            pause(500);
            if (get_time_us() - start_time > 10000000) {
                pauseMovement();
                pause(300);
                gyro_sync();
                unpauseMovement();
            }
            copy_objects();
        }        
        pause(300);
        gyro_sync();
    }
    
    return 0;
}
