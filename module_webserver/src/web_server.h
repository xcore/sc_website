#ifndef _WEB_SERVER_H_
#define _WEB_SERVER_H_
#include <xccompat.h>
#include "xtcp_client.h"
#ifdef __web_server_conf_h_exists__
#include "web_server_conf.h"
#endif
#include "web_server_gen.h"
#include "flash.h"
#include "simplefs.h"

/** This define controls what port the server listens on */
#ifndef WEB_SERVER_PORT
#define WEB_SERVER_PORT 80
#endif

/** Initialize the web server.
 *
 *  This function initializes the webserver.
 *
 *  \param c_xtcp         chanend connected to the xtcp server
 *  \param c_flash        If the webserver is configured to use flash where
 *                        flash access is in a separate task. Then this chanend
 *                        parameter needs to connect to that task. Otherwise it
 *                        should be set to null.
 *  \param flash_ports    If the webserver is configured to use flash and flash
 *                        access is not in a separate task. Then this parameter
 *                        should supply the ports to access the flash. Otherwise
 *                        it should be set to null.
 */
void web_server_init(chanend c_xtcp,
                     NULLABLE_RESOURCE(chanend, c_flash),
                     NULLABLE_REFERENCE_PARAM(fl_SPIPorts, flash_ports));


/** Handle a webserver tcp event.
 *
 *  This function should be called when a TCP event is signalled from the xtcp
 *  server. If the event is for the webserver port, the function will handle
 *  the event.
 *
 *  \param c_xtcp      chanend connected to the xtcp server
 *  \param c_flash     If the webserver is configured to use flash where
 *                     flash access is in a separate task. Then this chanend
 *                     parameter needs to connect to that task. Otherwise it
 *                     should be set to null.
 *  \param flash_ports    If the webserver is configured to use flash and flash
 *                        access is not in a separate task. Then this parameter
 *                        should supply the ports to access the flash. Otherwise
 *                        it should be set to null.
 *
 *  \param conn        The tcp connection structure containing the new event
 */
void web_server_handle_event(chanend c_xtcp,
                             NULLABLE_RESOURCE(chanend, c_flash),
                             NULLABLE_REFERENCE_PARAM(fl_SPIPorts, flash_ports),
                             REFERENCE_PARAM(xtcp_connection_t, conn));


void web_server_set_app_state(int st);

#ifdef __XC__
#pragma select handler
void web_server_cache_request(chanend c_flash);
void web_server_cache_handler(chanend c_flash, chanend c_xtcp);
#endif


#ifndef __XC__
char * web_server_get_param(const char *param,
                            int connection_state);
#endif

int web_server_copy_param(const char param[], int connection_state, char buf[]);

int web_server_is_post(int connection_state);
file_handle_t web_server_get_current_file(int connection_state);
int web_server_end_of_page(int connection_state);
#endif
