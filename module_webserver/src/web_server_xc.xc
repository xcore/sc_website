#include <xs1.h>
#include "web_server.h"
#include "simplefs.h"

extern simplefs_state_t simplefs_state;

static int is_data_request;

void web_server_unpause_senders(chanend c_flash, chanend c_xtcp);

void web_server_flash_response(chanend c_flash) {
  mutual_comm_transaction(c_flash,
                          is_data_request,
                          simplefs_state.mstate);
}

void web_server_flash_handler(chanend c_flash, chanend c_xtcp)
{
  if (is_data_request) {
    c_flash <: simplefs_state.request_addr;
    mutual_comm_complete_transaction(c_flash, is_data_request,
                                     simplefs_state.mstate);
  }
  else {
    slave {
      for (int i=0;i<WEB_SERVER_FLASH_CACHE_SIZE;i++)
         c_flash :> simplefs_state.local_cache[i];
    }
    simplefs_state.cached_addr = simplefs_state.request_addr;
    simplefs_state.request_addr = -1;
    mutual_comm_complete_transaction(c_flash, is_data_request,
                                     simplefs_state.mstate);
    web_server_unpause_senders(c_flash, c_xtcp);
  }
}
