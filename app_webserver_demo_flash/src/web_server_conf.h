#define WEB_SERVER_USE_FLASH 1

#define WEB_SERVER_FLASH_DEVICES flash_devices

#define WEB_SERVER_NUM_FLASH_DEVICES 1

#if SEPARATE_FLASH_TASK
#define WEB_SERVER_SEPARATE_FLASH_TASK 1
#endif

// Set the function to run after rendering a page
#define WEB_SERVER_POST_RENDER_FUNCTION post_page_render_increment_counter


#include "web_page_functions.h"
