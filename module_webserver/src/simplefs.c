#include "string.h"
#include "print.h"
#include "mutual_thread_comm.h"
#include "simplefs.h"
#include "web_server_flash.h"

simplefs_state_t simplefs_state;

extern fs_dir_t *root;

#if WEB_SERVER_USE_FLASH && !WEB_SERVER_SEPARATE_FLASH_TASK
static fl_SPIPorts *flash_ports;
#endif

void simplefs_init(fl_SPIPorts *fports)
{
#if !WEB_SERVER_USE_FLASH
  return;
#else
  simplefs_state_t *st = &simplefs_state;
  st->cached_addr = -1;
  st->request_addr = -1;

#if WEB_SERVER_SEPARATE_FLASH_TASK
  mutual_comm_init_state(&st->mstate);
#else
  flash_ports = fports;
#endif

#endif
}

#if WEB_SERVER_USE_FLASH
static int data_in_cache(simplefs_addr_t addr, int len)
{
  simplefs_state_t *st = &simplefs_state;
  return (addr >= st->cached_addr &&
          (addr+len) <= (st->cached_addr+WEB_SERVER_FLASH_CACHE_SIZE));
}
#endif

char * simplefs_get_data(chanend c_flash, simplefs_addr_t addr, int len)
{
#if !WEB_SERVER_USE_FLASH
  return (char *) addr;
#else
  simplefs_state_t *st = &simplefs_state;

#if WEB_SERVER_SEPARATE_FLASH_TASK
  return &st->local_cache[addr - st->cached_addr];
#else
  if (!data_in_cache(addr, len)) {
   fl_connectToDevice(flash_ports,
                      WEB_SERVER_FLASH_DEVICES,
                      WEB_SERVER_NUM_FLASH_DEVICES);
   fl_readData(addr, WEB_SERVER_FLASH_CACHE_SIZE,
               (unsigned char *) st->local_cache);
   st->local_cache[WEB_SERVER_FLASH_CACHE_SIZE] = 0;
   st->cached_addr = addr;
   fl_disconnect();
  }

  return &st->local_cache[addr - st->cached_addr];
#endif

#endif
}

int simplefs_request_pending()
{
#if !WEB_SERVER_USE_FLASH || !WEB_SERVER_SEPARATE_FLASH_TASK
  return 0;
#else
  simplefs_state_t *st = &simplefs_state;
  return (st->request_addr != -1);
#endif;
}

int simplefs_data_available(chanend c_flash, simplefs_addr_t addr, int len)
{
#if !WEB_SERVER_USE_FLASH || !WEB_SERVER_SEPARATE_FLASH_TASK
  return 1;
#else
  return data_in_cache(addr, len);
#endif
}

void simplefs_request_data(chanend c_flash, simplefs_addr_t addr)
{
#if !WEB_SERVER_USE_FLASH || !WEB_SERVER_SEPARATE_FLASH_TASK
  return;
#else
  simplefs_state_t *st = &simplefs_state;
  mutual_comm_notify(c_flash, &st->mstate);
  st->request_addr = addr;
#endif
}


file_handle_t simplefs_get_file(const char path[]) {

  fs_dir_t *node = root;

  while (1) {
    // skip any leading slash
    if (*path == '/')
      path++;

    // First try all the files
    for(fs_file_t *file = node->files;file != NULL;file = file->next) {
      if (strcmp(path,file->name) == 0)
        // success
        return (file_handle_t) file;
    }

    // Then all the sub-directories

    fs_dir_t *dir = node->children;
    for(;dir != NULL;dir=dir->next) {
      if (strstr(path, dir->name)==path) {
        path += strlen(dir->name);
        node = dir;
        break;
      }
    }

    if (dir)
      continue;

    // Nothing found
    return NULL;
  }
}


