#ifndef _WEB_SERVER_H_
#define _WEB_SERVER_H_
#include <xccompat.h>
#include "xtcp_client.h"
#ifdef __web_server_conf_h_exists__
#include "web_server_conf.h"
#endif
#include "web_server_gen.h"

void web_server_init(chanend c_xtcp, NULLABLE_RESOURCE(chanend, c_flash));

void web_server_handle_event(chanend c_xtcp,
                             NULLABLE_RESOURCE(chanend, c_flash),
                             REFERENCE_PARAM(xtcp_connection_t, conn));

#ifndef __XC__
char * web_server_get_param(const char *param,
                            int connection_state);
#endif

int web_server_copy_param(const char param[], int connection_state, char buf[]);

int web_server_is_post(int connection_state);

void web_server_set_app_state(int st);

#ifdef __XC__
#pragma select handler
void web_server_cache_request(chanend c_flash);
void web_server_cache_handler(chanend c_flash, chanend c_xtcp);
#endif

#endif
