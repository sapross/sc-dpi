// See LICENSE.SiFive for license details.
//VCS coverage exclude_file
import "DPI-C" function void uart_tick
(
 output bit uart_rx,
 input bit  uart_tx
);

module SimUART (
                   input         clock,
                   input         reset,

                   input         enable,
                   input         init_done,

                   input         uart_tx,
                   output        uart_rx,

                   input         uart_tx_driven,

                   output [31:0] exit
                   );


   bit          r_reset;

   wire         #0.1 __uart_tx = uart_tx_driven ?
                uart_tx : 1;

   bit          __uart_tx;
   bit          __uart_rx;
   int          __exit;

   reg          init_done_sticky;

   assign #0.1 uart_rx = __uart_rx;

   assign #0.1 exit = __exit;

   always @(posedge clock) begin
      r_reset <= reset;
      if (reset || r_reset) begin
         __exit = 0;
         __uart_rx = 1;
         init_done_sticky <= 1'b0;
      end else begin
         init_done_sticky <= init_done | init_done_sticky;
         if (enable && init_done_sticky) begin
            uart_tick(__uart_rx,
                      __uart_tx);
            __exit = 0;
         end // if (enable && init_done_sticky)
      end // else: !if(reset || r_reset)
   end // always @ (posedge clock)

endmodule
