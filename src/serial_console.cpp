#include "serial_console.hpp"
#include <assert.h>
#include <cstdint>
#include <iostream>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

#include <errno.h>
#include <fcntl.h>
#include <pty.h>
#include <queue>
#include <stdint.h>
#include <strings.h>
#include <sys/types.h>
#include <termios.h>
#include <unistd.h>

serial_console_t::serial_console_t(): err(0), quit(0), server_fd(0), client_fd(0)
{
    auto bdcode = get_baudrate(baud_rate);
    if (bdcode == 0)
    {
        fprintf(stderr, "serial_console failed. Invalid Baudrate:(%ld)\n", baud_rate);
        abort();
    }

    bzero(&config, sizeof(config));
    cfmakeraw(&config);
    config.c_cflag |= bdcode;
    // Read will return immediatly, whether data is available or not.
    config.c_cc[VMIN]  = 0;
    config.c_cc[VTIME] = 0;
    char name[64];
    if (openpty(&server_fd, &client_fd, name, &config, NULL) < 0)
    {
        fprintf(stderr, "serial_console failed openpty: (%d)\n", errno);
        abort();
    }
    else
    {
        fprintf(stdout, "serial_console opened pty: %s\n", name);
    };

    fcntl(server_fd, F_SETFL, O_NONBLOCK);

    // tcflush(server_fd, TCIOFLUSH);
    // tcsetattr(server_fd, TCSANOW, &config);
    this->tx = 1;
    this->rx = 1;
    err      = 1;
    quit     = 0;
}

void serial_console_t::send()
{
    // The number of ticks since the last state change.
    // Each state transition resets this value to prevent integer overflow.
    static uint64_t num_ticks = 0;

    static enum uart_st state = st_idle;
    // Current byte/word being sent.
    ssize_t        len     = 0;
    static uint8_t current = 0;
    // Index of the current bit being sent.
    static size_t data_index = 0;
    uint8_t       l_tx       = tx;
    switch (state)
    {
    case (st_idle):
        l_tx = 1;
        len  = read(server_fd, &current, 1);
        if (len > 0)
        {
            state = st_start;
        }
        break;
    case (st_start):
        num_ticks++;
        l_tx = 0;
        if (num_ticks >= baud_ticks)
        {
            num_ticks = 0;
            state     = st_data;
        }
        break;
    case (st_data):
        num_ticks++;
        l_tx = (current >> (data_index)) & 1u;
        if (num_ticks >= baud_ticks)
        {
            num_ticks = 0;
            // std::cout << (int)l_tx;
            state = st_data;
            data_index++;
            if (data_index >= 8)
            {
                // std::cout << std::endl;
                data_index = 0;
                state      = st_stop;
            }
        }
        break;
    case (st_stop):
        num_ticks++;
        l_tx = 1;
        if (num_ticks >= baud_ticks)
        {
            num_ticks = 0;
            state     = st_idle;
        }
        break;
    }
    tx = l_tx;
}

void serial_console_t::receive()
{

    // The number of ticks since the last state change.
    // Each state transition resets this value to prevent integer overflow.
    static uint64_t num_ticks = 0;

    static enum uart_st state = st_idle;
    // Current byte/word being sent.
    static uint8_t current = 0;
    // Index of the current bit being sent.
    static size_t data_index = 0;
    // Value of rx of the last tick. Required to detect falling edge.
    static uint8_t rx_prev = 1;
    uint8_t        l_rx    = rx;
    switch (state)
    {
    case (st_idle):
        if (rx_prev == 1 && l_rx == 0)
        {
            state = st_start;
        }
        break;
    case (st_start):
        num_ticks++;
        // Wait for 1/2 the baudrate to sample at the center of the baud period.
        if (num_ticks >= baud_ticks / 2)
        {
            num_ticks = 0;
            current   = 0;
            state     = st_data;
        }
        break;
    case (st_data):
        num_ticks++;
        if (num_ticks >= baud_ticks)
        {
            num_ticks = 0;
            current |= l_rx << data_index;
            data_index++;
            // std::cout << (int)current;
            // std::cout << std::endl;
            if (data_index >= 8)
            {
                data_index = 0;
                output.push(current);
                state = st_stop;
            }
        }
        break;
    case (st_stop):
        num_ticks++;
        if (num_ticks >= baud_ticks / 2)
        {
            num_ticks = 0;
            state     = st_idle;
        }
        break;
    }
    rx_prev = l_rx;
}
void serial_console_t::tick(uint8_t* uart_rx, uint8_t uart_tx)
{
    uint8_t data;
    // Attempt to read a byte from the serial driver.
    // Termios is configured to immediately return, whether data
    // is available or not. This prevents the simulation from stopping or
    // slowing down unless data is available to read.

    // std::cout << "Tick" << std::endl;

    // Get new data from TTY
    // ssize_t len = read(server_fd, &data, 1);
    // if (len == 1)
    // {
    //     // std::cout << "R:" << (int)(data) << std::endl;
    //     input.push(data);
    // }
    // Send data over simulated UART.
    send();
    // Receive data over simulated UART
    // TX input is connected to console l_rx.
    this->rx = uart_tx;
    receive();
    // Put received data to TTY.
    if (output.size() > 0)
    {
        data = output.front();
        // std::cout << "S: " << (int)(data) << std::endl;
        output.pop();
        write(server_fd, &data, 1);
        tcdrain(server_fd);
    }

    // Due to the function of the serial_console, tx and l_rx are swapped in the
    // simulatio model.
    *uart_rx = this->tx;
}

unsigned int get_baudrate(int bd)
{
    if (bd == 300)
    {
        return B300;
    }
    if (bd == 600)
    {
        return B600;
    }
    if (bd == 1200)
    {
        return B1200;
    }
    if (bd == 2400)
    {
        return B2400;
    }
    if (bd == 4800)
    {
        return B4800;
    }
    if (bd == 9600)
    {
        return B9600;
    }
    if (bd == 19200)
    {
        return B19200;
    }
    if (bd == 38400)
    {
        return B38400;
    }
    if (bd == 57600)
    {
        return B57600;
    }
    if (bd == 115200)
    {
        return B115200;
    }
    if (bd == 230400)
    {
        return B230400;
    }
    if (bd == 460800)
    {
        return B460800;
    }
    if (bd == 576000)
    {
        return B57600;
    }
    if (bd == 921600)
    {
        return B921600;
    }
    if (bd == 1000000)
    {
        return B1000000;
    }
    if (bd == 2000000)
    {
        return B2000000;
    }
    if (bd == 3000000)
    {
        return B3000000;
    }
    return 0;
}
