#define WEB_SERVER_USE_FLASH 1

#define WEB_SERVER_FLASH_DEVICES flash_devices

#define WEB_SERVER_NUM_FLASH_DEVICES 1

#if SEPARATE_FLASH_TASK
#define WEB_SERVER_FLASH_THREAD 1
#endif

#include "web_page_functions.h"
