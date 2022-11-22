// Copyright 2018 ETH Zurich and University of Bologna.
// Copyright and related rights are licensed under the Solderpad Hardware
// License, Version 0.51 (the "License"); you may not use this file except in
// compliance with the License.  You may obtain a copy of the License at
// http://solderpad.org/licenses/SHL-0.51. Unless required by applicable law
// or agreed to in writing, software, hardware and materials distributed under
// this License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
// CONDITIONS OF ANY KIND, either express or implied. See the License for the
// specific language governing permissions and limitations under the License.
//
// Author: Florian Zaruba, ETH Zurich
// Date: 19.03.2017
// Description: Test-harness for Ariane
//              Instantiates an AXI-Bus and memories

module main #(
              parameter int unsigned CLK_RATE = 50*10**6,
              parameter int unsigned BAUD_RATE = 3*10**6
              ) (
                 input logic         clk_i,
                 input logic         rtc_i,
                 input logic         rst_ni,
                 output logic [31:0] exit_o
                 );

   int                               uart_enable = 1;
   logic                             uart_rx ;
   logic                             uart_tx;
   logic                             uart_tx_driven = 1;
   int                               uart_exit;
   assign exit_o = uart_exit;

   SimUART i_SimUART (
                      .clock                ( clk_i                ),
                      .reset                ( ~rst_ni              ),
                      .enable               ( uart_enable[0]       ),
                      .init_done            ( 1'b1                 ),
                      .uart_rx              ( uart_rx              ),
                      .uart_tx              ( uart_tx              ),
                      .uart_tx_driven       ( uart_tx_driven       ),
                      .exit                 ( uart_exit            )
                      );

   assign uart_tx = uart_rx;


endmodule
