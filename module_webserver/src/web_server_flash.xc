#include <xs1.h>
#include <flash.h>
#include <print.h>
#include "web_server_flash.h"
#include "mutual_thread_comm.h"
#include "simplefs.h"
#ifdef __web_server_conf_h_exists__
#include "web_server_conf.h"
#endif


char web_server_flash_cache[WEB_SERVER_FLASH_CACHE_SIZE];



void web_server_flash_init(fl_SPIPorts &flash_ports)
{

}

#ifdef WEB_SERVER_USE_FLASH
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
