#include <stdio.h>

#include "serial_console.hpp"

int main(int argc, char** argv)
{

    serial_console_t uart;
    uint8_t          rx = 1;
    uint8_t          tx = 1;

    while (true)
    {
        uart.tick(&rx, tx);
        tx = rx;
    }

    return 0;
}
