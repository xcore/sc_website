#ifndef __web_server_h__
#define __web_server_h__
#include <xccompat.h>
#include <flash.h>

void web_server_flash_init(REFERENCE_PARAM(fl_SPIPorts, flash_pors));

#ifdef __XC__
select web_server_flash(chanend c_flash,
                        fl_SPIPorts &flash_ports);
#endif

#ifndef WEB_SERVER_FLASH_DEVICES
#define WEB_SERVER_FLASH_DEVICES flash_devices
#endif

#ifndef WEB_SERVER_NUM_FLASH_DEVICES
#define WEB_SERVER_NUM_FLASH_DEVICES 1
#endif

extern fl_DeviceSpec WEB_SERVER_FLASH_DEVICES [];


#endif // __web_server_h__
