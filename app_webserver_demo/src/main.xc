// Copyright (c) 2011, XMOS Ltd, All rights reserved
// This software is freely distributable under a derivative of the
// University of Illinois/NCSA Open Source License posted in
// LICENSE.txt and at <http://github.xcore.com/>

#include <platform.h>
#include <xs1.h>
#include <print.h>
#include <xscope.h>
#include "xtcp.h"
#include "web_server.h"
#include "ethernet_board_support.h"
#include <stdio.h>
#include "itoa.h"

// These intializers are taken from the ethernet_board_support.h header for
// XMOS dev boards. If you are using a different board you will need to
// supply explicit port structure intializers for these values
ethernet_xtcp_ports_t xtcp_ports =
  {    on ETHERNET_DEFAULT_TILE:  OTP_PORTS_INITIALIZER,
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


int get_timer_value(char buf[], int x)
{
  timer tmr;
  unsigned time;
  int len;
  tmr :> time;
  len = itoa(time, buf, 10, 0);
  return len;
}

void tcp_handler(chanend c_xtcp) {
  xtcp_connection_t conn;
  web_server_init(c_xtcp, null, null);
  init_web_state();
  while (1) {
    select
      {
      case xtcp_event(c_xtcp,conn):
        web_server_handle_event(c_xtcp, null, null, conn);
        break;
      }
  }
}

// Program entry point
int main(void) {
	chan c_xtcp[1];

	par
	{

          on ETHERNET_DEFAULT_TILE: ethernet_xtcp_server(xtcp_ports,
                                                         ipconfig,
                                                         c_xtcp,
                                                         1);

          on tile[0]: tcp_handler(c_xtcp[0]);

	}
	return 0;
}
