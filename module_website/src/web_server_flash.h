#ifndef __web_server_h__
#define __web_server_h__
#include <flash.h>

void web_server_flash_init(fl_SPIPorts &flash_prots);

select web_server_flash(chanend c_flash,
                        fl_SPIPorts &flash_ports);

#endif // __web_server_h__
