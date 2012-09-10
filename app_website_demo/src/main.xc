// Copyright (c) 2011, XMOS Ltd, All rights reserved
// This software is freely distributable under a derivative of the
// University of Illinois/NCSA Open Source License posted in
// LICENSE.txt and at <http://github.xcore.com/>

#include <platform.h>
#include <xs1.h>
#include "uip_server.h"
#include "getmac.h"
#include <print.h>
#include <xscope.h>
#include "uip_single_server.h"
#include "xtcp_client.h"
#include "web_server.h"
#include <stdio.h>
#include "itoa.h"

#define ETH_CORE 0

// Ethernet Ports
on stdcore[ETH_CORE]: struct otp_ports otp_ports =
{
  XS1_PORT_32B,
  XS1_PORT_16C,
  XS1_PORT_16D
};


#define PORT_ETH_FAKE on stdcore[ETH_CORE]: XS1_PORT_8C

on stdcore[ETH_CORE]: mii_interface_t mii =
{
	XS1_CLKBLK_2,
	XS1_CLKBLK_3,

	PORT_ETH_RXCLK_1,
	PORT_ETH_ERR_1,
	PORT_ETH_RXD_1,
	PORT_ETH_RXDV_1,

	PORT_ETH_TXCLK_1,
	PORT_ETH_TXEN_1,
	PORT_ETH_TXD_1,
        PORT_ETH_FAKE
};

#define PORT_ETH_RST_N XS1_PORT_8D

on stdcore[ETH_CORE]: smi_interface_t smi = {0, PORT_ETH_MDIO_1, PORT_ETH_MDC_1};

// IP Config - change this to suit your network.  Leave with all
// 0 values to use DHCP
xtcp_ipconfig_t ipconfig = {
  //{ 0, 0, 0, 0 }, // ip address (eg 192,168,0,2)
                { 192, 168, 1, 178 }, // ip address (eg 192,168,0,2)
		{ 0, 0, 0, 0 }, // netmask (eg 255,255,255,0)
		{ 0, 0, 0, 0 } // gateway (eg 192,168,0,1)
};

on stdcore[0] : fl_SPIPorts flash_ports =
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

void tcp_handler(chanend c_xtcp) {
  xtcp_connection_t conn;
  web_server_init(c_xtcp, null, flash_ports);
  while (1) {
    select
      {
      case xtcp_event(c_xtcp,conn):
        web_server_handle_event(c_xtcp, null, conn);
        break;
      }
  }
}

// Program entry point
int main(void) {
	chan c_xtcp[1];

	par
	{

        on stdcore[ETH_CORE]:
        {
            char mac_address[6];
            ethernet_getmac_otp(otp_ports, mac_address);
            // Start server
            uip_single_server(null, smi, mii, c_xtcp, 1,
                              ipconfig, mac_address);
        }

        on stdcore[ETH_CORE]: tcp_handler(c_xtcp[0]);

	}
	return 0;
}
