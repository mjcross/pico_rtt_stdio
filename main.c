#include <stdio.h>
#include "pico/stdlib.h"

// Using SEGGER RTT allows you to see stdio over SWD in debug builds
// without the need for a UART or USB connection (see README)

int main()
{
    stdio_init_all();

    while (true) {
        printf("Hello, world!\n");
        sleep_ms(1000);
    }
}
