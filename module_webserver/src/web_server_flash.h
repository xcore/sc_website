#ifndef __web_server_h__
#define __web_server_h__
#include <xccompat.h>
#include <flash.h>
#ifdef __web_server_conf_h_exists__
#include "web_server_conf.h"
#endif


/** This define controls whether the web server gets the pages from SPI
 *  flash or not. Set to 1 to use flash and 0 not to use flash.
 */
#if !defined(WEB_SERVER_USE_FLASH) || __DOXYGEN__
#define WEB_SERVER_USE_FLASH 0
#endif

/** This define sets whether the flash access should be done within the web
 *  server functions or offloaded via a channel to another task.
 */
#if !defined(WEB_SERVER_SEPARATE_FLASH_TASK) || __DOXYGEN__
#define WEB_SERVER_SEPARATE_FLASH_TASK 0
#endif


/** This define should be set to the name of a global variable that defines
 *  the array of possible flash devices that the web server connects to.
 */
#if !defined(WEB_SERVER_FLASH_DEVICES) || __DOXYGEN__
#define WEB_SERVER_FLASH_DEVICES flash_devices
#endif

/** This define sets the size of the array of possible flash devices defined
 *  by the variable set by.
 */
#if !defined(WEB_SERVER_NUM_FLASH_DEVICES) || __DOXYGEN__
#define WEB_SERVER_NUM_FLASH_DEVICES 1
#endif


/** This function intializes the separate flash task access to the web server
 *  data.
 *
 *  \param flash_ports   The ports to access SPI flash (see libflash)
 *
 */
void web_server_flash_init(REFERENCE_PARAM(fl_SPIPorts, flash_ports));


/** This function handles the request from the tcp handling task to get
 *  some data from flash.
 *
 *  \param c_flash       Chanend connected to the tcp handling functions of the
 *                       webserver
 *  \param flash_ports   The ports to access SPI flash (see libflash)
 *
 */
#ifdef __XC__
select web_server_flash(chanend c_flash,
                        fl_SPIPorts &flash_ports);
#endif

extern fl_DeviceSpec WEB_SERVER_FLASH_DEVICES [];


#endif // __web_server_h__


