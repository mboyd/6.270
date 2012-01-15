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
        
    moveToPoint(30, 0, 200);
    //turnToHeading(90);
    waitForMovementComplete();
    
    //while (1) {}
    
    moveToPoint(30, 30, 200);
   pause(50);
    waitForMovementComplete();

  
    
    moveToPoint(00, 30, 200);
   pause(50);
     waitForMovementComplete();

    
    moveToPoint(00, 00, 200);
    waitForMovementComplete();
    
    //while (1) {
    //    copy_objects();
    //    moveToPoint(objects[2].x, objects[2].y, 200);
    //    waitForMovementComplete();
    //}
    
    return 0;
}
