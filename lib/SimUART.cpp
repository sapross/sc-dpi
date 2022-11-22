
#include "serial_console.hpp"
#include <cstdlib>

serial_console_t* uart;
extern "C" void   uart_tick(unsigned char* uart_rx, unsigned char uart_tx)
{
    if (!uart)
    {
        uart = new serial_console_t();
    }
    uart->tick(uart_rx, uart_tx);
}
