extern int get_timer_value(char buf[], int);

#if SEPARATE_FLASH_TASK
#define WEB_SERVER_FLASH_THREAD
#endif
