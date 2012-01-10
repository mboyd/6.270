#include <joyos.h>
#include "navigation.c"


int usetup (void) {
    return 0;
}

int umain (void) {

    // YOUR CODE GOES HERE
    
    printf("Hello, world!\n");
    
    nav_init();
    
    return 0;
}
