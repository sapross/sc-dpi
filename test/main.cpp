// Licensed to the Apache Software Foundation (ASF) under one
// or more contributor license agreements.  See the NOTICE file
// distributed with this work for additional information
// regarding copyright ownership.  The ASF licenses this file
// to you under the Apache License, Version 2.0 (the
// "License"); you may not use this file except in compliance
// with the License.  You may obtain a copy of the License at

//   http://www.apache.org/licenses/LICENSE-2.0

// Unless required by applicable law or agreed to in writing,
// software distributed under the License is distributed on an
// "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
// KIND, either express or implied.  See the License for the
// specific language governing permissions and limitations
// under the License.

#include "Vmain.h"
#include "Vmain__Dpi.h"
#include "verilated.h"
#include "verilated_fst_c.h"
#include "verilated_vcd_c.h"

#include <chrono>
#include <ctime>
#include <getopt.h>
#include <iomanip>
#include <iostream>
#include <memory>
#include <signal.h>
#include <stdio.h>
#include <string>
#include <unistd.h>

#include "serial_console.hpp"
// This software is heavily based on Rocket Chip
// Checkout this awesome project:
// https://github.com/freechipsproject/rocket-chip/

// This is a 64-bit integer to reduce wrap over issues and
// allow modulus.  You can also use a double, if you wish.
static vluint64_t main_time = 0;

// static const char* verilog_plusargs[] = {"jtag_rbb_enable"};
static const char* verilog_plusargs[] = {"uart_enable"};

extern serial_console_t* uart;

// Called by $time in Verilog converts to double, to match what SystemC does
double sc_time_stamp() { return main_time; }

int main(int argc, char** argv)
{
    std::clock_t c_start = std::clock();
    auto         t_start = std::chrono::high_resolution_clock::now();
    bool         verbose;
    bool         perf;
    unsigned     random_seed            = (unsigned)time(NULL) ^ (unsigned)getpid();
    uint64_t     max_cycles             = -1;
    int          ret                    = 0;
    bool         print_cycles           = false;
    char**       htif_argv              = NULL;
    int          verilog_plusargs_legal = 1;

    Verilated::debug(0);
    Verilated::traceEverOn(true);
    Verilated::commandArgs(argc, argv);
    Verilated::mkdir("logs");

    VerilatedFstC* trace = new VerilatedFstC;

    uart = new serial_console_t();
    std::unique_ptr<Vmain> top(new Vmain);
    top->trace(trace, 99);
    trace->open("logs/waveform.fst");

    for (int i = 0; i < 10; i++)
    {
        top->rst_ni = 0;
        top->clk_i  = 0;
        top->rtc_i  = 0;
        top->eval();
        top->clk_i = 1;
        top->eval();
        trace->dump(main_time);
        main_time++;
    }
    top->rst_ni = 1;
    // for (int i = 0; i < 1000; i++)
    while (uart->done() < 10)
    {
        top->clk_i = !top->clk_i;
        top->eval();
        trace->dump(main_time);
        main_time++;
    }

    trace->close();

    if (uart)
        delete uart;

    std::clock_t c_end = std::clock();
    auto         t_end = std::chrono::high_resolution_clock::now();

    return ret;
}
