// Copyright (c) 2011, XMOS Ltd, All rights reserved
// This software is freely distributable under a derivative of the
// University of Illinois/NCSA Open Source License posted in
// LICENSE.txt and at <http://github.xcore.com/>

#include <platform.h>
#include <xs1.h>
#include <print.h>
#include <xscope.h>
#include <stdio.h>
#include <flash.h>
#include "xtcp.h"
#include "web_server.h"
#include "web_server_flash.h"
#include "web_page_functions.h"
#include "ethernet_board_support.h"
#include "itoa.h"

// These intializers are taken from the ethernet_board_support.h header for
// XMOS dev boards. If you are using a different board you will need to
// supply explicit port structure intializers for these values
ethernet_xtcp_ports_t xtcp_ports =
    {on ETHERNET_DEFAULT_TILE:  OTP_PORTS_INITIALIZER,
     ETHERNET_DEFAULT_SMI_INIT,
     ETHERNET_DEFAULT_MII_INIT_lite,
     ETHERNET_DEFAULT_RESET_INTERFACE_INIT};

// IP Config - change this to suit your network.  Leave with all
// 0 values to use DHCP
xtcp_ipconfig_t ipconfig = {
  { 0, 0, 0, 0 }, // ip address (eg 192,168,0,2)
  { 0, 0, 0, 0 }, // netmask (eg 255,255,255,0)
  { 0, 0, 0, 0 } // gateway (eg 192,168,0,1)
};

on tile[0] : fl_SPIPorts flash_ports =
{ PORT_SPI_MISO,
  PORT_SPI_SS,
  PORT_SPI_CLK,
  PORT_SPI_MOSI,
  XS1_CLKBLK_1
};

fl_DeviceSpec flash_devices[] =
  {
    FL_DEVICE_NUMONYX_M25P16,
  };


int get_timer_value(char buf[], int x)
{
  timer tmr;
  unsigned time;
  int len;

  tmr :> time;
  len = itoa(time, buf, 10, 0);
  return len;
}

void tcp_handler(chanend c_xtcp, chanend ?c_flash, fl_SPIPorts &?flash_ports) {
  xtcp_connection_t conn;
  web_server_init(c_xtcp, c_flash, flash_ports);
  init_web_state();
  while (1) {
    select
      {
      case xtcp_event(c_xtcp, conn):
        web_server_handle_event(c_xtcp, c_flash, flash_ports, conn);
        break;
#if SEPARATE_FLASH_TASK
      case web_server_cache_request(c_flash):
        web_server_cache_handler(c_flash, c_xtcp);
        break;
#endif
      }
  }
}

#if SEPARATE_FLASH_TASK
void flash_handler(chanend c_flash) {
  web_server_flash_init(flash_ports);
  while (1) {
    select {
    case web_server_flash(c_flash, flash_ports);
    }
  }
}
#endif


// Program entry point
int main(void) {
	chan c_xtcp[1];
         #if SEPARATE_FLASH_TASK
        chan c_flash;
        #endif


	par
	{

          on ETHERNET_DEFAULT_TILE: {
            ethernet_xtcp_server(xtcp_ports,
                                 ipconfig,
                                 c_xtcp,
                                 1);
          }

          on tile[0]: {
            tcp_handler(c_xtcp[0],
#if SEPARATE_FLASH_TASK
                        c_flash,
                        null
#else
                        null,
                        flash_ports
#endif
                        );
          }

        #if SEPARATE_FLASH_TASK
        on tile[0]: flash_handler(c_flash);
        #endif

	}
	return 0;
}
