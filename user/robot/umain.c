#include <joyos.h>
#include "platform.h"
#include "navigation.h"

extern volatile uint8_t robot_id;

int usetup (void) {
    robot_id = 7;
    
    platform_init();
    nav_init();
    return 0;
}

int umain (void) {
    printf("Hello, world!\n");
    
    nav_start();
    
    printf("Nav started, setting coords\n");
    //moveToPoint(100, 0, 150);
    turnToHeading(45);
    printf("Coords set\n");
    
    return 0;
}
