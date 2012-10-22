#include "web_page_functions.h"

#if SEPARATE_FLASH_TASK
#define WEB_SERVER_FLASH_THREAD
#endif

// Set the function to run after rendering a page
#define WEB_SERVER_POST_RENDER_FUNCTION post_page_render_increment_counter
