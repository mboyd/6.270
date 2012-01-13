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
    
    return 0;
}
