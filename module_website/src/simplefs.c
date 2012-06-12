#include "string.h"
#include "print.h"
#include "mutual_thread_comm.h"
#include "simplefs.h"

simplefs_state_t simplefs_state;

extern fs_dir_t *root;

void simplefs_init()
{
#ifndef WEB_SERVER_USE_FLASH
  return;
#else
  simplefs_state_t *st = &simplefs_state;
  st->cached_addr = -1;
  st->request_addr = -1;
  mutual_comm_init_state(&st->mstate);
#endif
}

char * simplefs_get_data(chanend c_flash, simplefs_addr_t addr, int len)
{
#ifndef WEB_SERVER_USE_FLASH
  return (char *) addr;
#else
  simplefs_state_t *st = &simplefs_state;
  return &st->local_cache[addr - st->cached_addr];
#endif
}

int simplefs_request_pending()
{
#ifndef WEB_SERVER_USE_FLASH
  return 0;
#else
  simplefs_state_t *st = &simplefs_state;
  return (st->request_addr != -1);
#endif;
}

int simplefs_data_available(chanend c_flash, simplefs_addr_t addr, int len)
{
#ifndef WEB_SERVER_USE_FLASH
  return 1;
#else
  simplefs_state_t *st = &simplefs_state;
  return (addr >= st->cached_addr &&
          (addr+len) <= (st->cached_addr+WEB_SERVER_FLASH_CACHE_SIZE));
#endif
}

void simplefs_request_data(chanend c_flash, simplefs_addr_t addr)
{
#ifndef WEB_SERVER_USE_FLASH
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


