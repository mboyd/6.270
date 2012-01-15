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
        float x = ((float) objects[2].x) / VPS_PER_CM;
        float y = ((float) objects[2].y) / VPS_PER_CM;
        moveToPoint(x, y, 200);
        waitForMovement();
    }
    
    return 0;
}
