#include <joyos.h>
#include "platform.h"
#include "navigation.h"


int usetup (void) {
    platform_init();
    nav_init();
    return 0;
}

int umain (void) {
    printf("Hello, world!\n");
    
    nav_start();
    
    printf("Nav started, setting coords\n");
    moveToPoint(10, 0, 200);
    //turnToHeading(45);
    printf("Coords set\n");
    
    return 0;
}
