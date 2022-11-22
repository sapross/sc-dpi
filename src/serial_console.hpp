#ifndef SERIAL_CONSOLE_H_
#define SERIAL_CONSOLE_H_

#include <queue>
#include <stdint.h>
#include <sys/types.h>
#include <termios.h>

unsigned int get_baudrate(int baudrate);

class serial_console_t
{
  public:
    serial_console_t();
    void          tick(uint8_t* rx, uint8_t tx);
    unsigned char done() { return quit; }
    int           exit_code() { return err; }

  private:
    void          send();
    void          receive();
    int           err  = 0;
    unsigned char quit = 0;

    struct termios config;
    int            server_fd;
    int            client_fd;

    uint8_t             rx;
    uint8_t             tx;
    std::queue<uint8_t> input;
    std::queue<uint8_t> output;
};

static const uint64_t baud_rate   = 3 * 10e5;
static const uint64_t baud_period = 333; // ns
static const uint64_t clk_rate    = 50 * 10e5;
static const uint64_t clk_period  = 10; // ns
// The number of ticks required until one symbol has been send by uart.
static const uint64_t baud_ticks = clk_rate / baud_rate;

enum uart_st
{
    st_idle,
    st_start,
    st_data,
    st_stop
};

#endif // SERIAL_CONSOLE_H_
