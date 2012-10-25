#include <xs1.h>
#include <flash.h>
#include <print.h>
#include "web_server_flash.h"
#include "mutual_thread_comm.h"
#include "simplefs.h"

#ifndef WEB_SERVER_CHECK_FLASH_SIGNATURE
#define WEB_SERVER_CHECK_FLASH_SIGNATURE 1
#endif

char web_server_flash_cache[WEB_SERVER_FLASH_CACHE_SIZE];

void web_server_check_signature(fl_SPIPorts &flash_ports);

void web_server_flash_init(fl_SPIPorts &flash_ports)
{
  #if WEB_SERVER_USE_FLASH && WEB_SERVER_SEPARATE_FLASH_TASK && WEB_SERVER_CHECK_FLASH_SIGNATURE
  web_server_check_signature(flash_ports);
  #endif
}

#if WEB_SERVER_USE_FLASH
select web_server_flash(chanend c_flash,
                        fl_SPIPorts &flash_ports)
{
 case mutual_comm_notified(c_flash): {
   simplefs_addr_t request_addr;
   c_flash :> request_addr;
   fl_connectToDevice(flash_ports,
                      WEB_SERVER_FLASH_DEVICES,
                      WEB_SERVER_NUM_FLASH_DEVICES);
   fl_readData(request_addr, WEB_SERVER_FLASH_CACHE_SIZE,
               web_server_flash_cache);
   fl_disconnect();
   mutual_comm_initiate(c_flash);
   master {
     for (int i=0;i<WEB_SERVER_FLASH_CACHE_SIZE;i++)
       c_flash <: web_server_flash_cache[i];
   }
 }
 break;
}
#endif
