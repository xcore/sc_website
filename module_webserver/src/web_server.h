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

/** Set the application state of the web server.
 *
 *  This sets a single integer (which could be cast from a pointer in C) to be
 *  the application state of the server. This state variable can be accessed by
 *  the dynamic content of the server.
 */
void web_server_set_app_state(int st);

#ifdef __XC__
/** Select handler to react to a cache response from the
 *
 *  This select handler can be used in a select case to handle the response
 *  from the separate flash task with some data. It should be followed by a
 *  call to web_server_flash_handler().
 *
 *  \param c_flash    chanend connected to separate flash task
 *
 */
#pragma select handler
void web_server_flash_response(chanend c_flash);

/** Handle incoming data from flash thread.
 *
 *  This function should be called in the body of a select case that responds
 *  to the flash task via web_server_flash_reponse().
 *
 *  \param c_flash   chanend connected to separate flash task
 *  \param c_xtcp    chanend connected to the xtcp server
 */
void web_server_flash_handler(chanend c_flash, chanend c_xtcp);
#endif

/** Get a web page parameter.
 *
 *  This function looks up a parameter that has been passed to the current page
 *  request via either a GET or POST request.
 *
 *  \param    param             The name of the parameter to look up
 *  \param    connection_state  The connection state of the page being served
 *
 *  \returns  a pointer to the parameters value. If the parameter was not
 *            passed in as part of the HTTP request then NULL is returned.
 **/
#if !defined(__XC__) || __DOXYGEN__
char * web_server_get_param(const char *param,
                            int connection_state);
#endif

/** Copy a web page parameter.
 *
 *  This function looks up a parameter that has been passed to the current page
 *  request via either a GET or POST request and copies it into a supplied
 *  buffer
 *
 *  \param    param             The name of the parameter to look up
 *  \param    connection_state  The connection state of the page being served
 *  \param    buf               The buffer to copy the parameter into
 *
 *  \returns  the length of the copied parameter value. If the parameter was not
 *            passed in as part of the HTTP request then 0 is returned.
 **/
int web_server_copy_param(const char param[], int connection_state, char buf[]);

/** Determine whether the current page request was a POST request.
 *
 * \param connection_state  The connection state of the page being served
 *
 * \returns non-zero if the page request is a POST request. Zero otherwise.
 **/
int web_server_is_post(int connection_state);

/** Return the file handle of the current page.
 *
 * \param connection_state  The connection state of the page being served
 *
 * \returns A file handle for the current page. See ``simplefs.h`` for details.
 **/
file_handle_t web_server_get_current_file(int connection_state);

/** Have we served the entire page
 *
 *  This function is usually used in the ``WEB_SERVER_POST_RENDER`` function
 *  to determine whether the whole page has been fully served
 *  (for example to update a page visit counter).
 *
 *  \returns non-zero if the page has been fully served. Zero otherwise.
 */
int web_server_end_of_page(int connection_state);
#endif
