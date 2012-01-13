#include <joyos.h>
#include "platform.h"
#include "navigation.h"


int usetup (void) {
    platform_init();
    nav_init();
    return 0;
    }p;

int umain (void) {
    printf("Hello, world!\n");

    motor_set_vel(0, 100);
    motor_set_vel(1, 100);
    
    go_click();
    
    motor_set_vel(0, 255);
    motor_set_vel(1, 255);
    
    nav_start();
    
    return 0;
}
