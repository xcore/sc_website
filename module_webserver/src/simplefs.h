#ifndef _SIMPLEFS_H_
#define _SIMPLEFS_H_
#include <xccompat.h>
#include "flash.h"
#include "web_server_flash.h"
#include "mutual_thread_comm.h"

typedef enum fstype_t {
  FS_TYPE_TEMPLATE=0,
  FS_TYPE_BINARY=1
} fstype_t;


typedef unsigned simplefs_addr_t;

#ifndef __XC__

typedef struct fs_file_t {
  struct fs_file_t *next;
  int ftype;
  int length;
  simplefs_addr_t data;
  char name[];
} fs_file_t;

typedef struct fs_dir_t {
  struct fs_dir_t *next;
  struct fs_dir_t *children;
  struct fs_file_t *files;
  char name[];
} fs_dir_t;

#endif

#define INVALID_FILE (0)

typedef unsigned file_handle_t;

file_handle_t simplefs_get_file(const char path[]);

#ifndef __XC__
char * simplefs_get_data(chanend c_flash, simplefs_addr_t addr, int len);
#endif

int  simplefs_data_available(chanend c_flash, simplefs_addr_t addr, int len);
void simplefs_request_data(chanend c_flash, simplefs_addr_t addr);
int simplefs_request_pending();
void simplefs_init(NULLABLE_REFERENCE_PARAM(fl_SPIPorts, flash_ports));

#ifndef WEB_SERVER_FLASH_CACHE_SIZE
#ifndef WEB_SERVER_SEND_BUF_SIZE
#define WEB_SERVER_FLASH_CACHE_SIZE 128
#else
#define WEB_SERVER_FLASH_CACHE_SIZE WEB_SERVER_SEND_BUF_SIZE
#endif
#endif

typedef struct simplefs_state_t {
  char local_cache[WEB_SERVER_FLASH_CACHE_SIZE+1];
  simplefs_addr_t cached_addr;
  simplefs_addr_t request_addr;
  mutual_comm_state_t mstate;
} simplefs_state_t;


#endif //_SIMPLEFS_H_
