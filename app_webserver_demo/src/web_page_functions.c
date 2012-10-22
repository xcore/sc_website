#include "simplefs.h"
#include "web_server.h"
#include "itoa.h"

typedef struct app_state_t {
  int counter;
} app_state_t;

static app_state_t app_state;

void init_web_state() {
  app_state.counter = 1;
  web_server_set_app_state((int) &app_state);
}

void post_page_render_increment_counter(int app_state0, int connection_state)
{
  app_state_t * app_state = (app_state_t *) app_state0;
  if (web_server_end_of_page(connection_state) &&
      web_server_get_current_file(connection_state) ==
      simplefs_get_file("index.html"))
    {
    app_state->counter++;
    }
  return;
}

int get_counter_value(int app_state0, char buf[])
{
  app_state_t * app_state = (app_state_t *) app_state0;
  int len = itoa(app_state->counter, buf, 10, 0);
  return len;
}

int get_input_param(int connection_state, char buf[]) {
  return web_server_copy_param("input",  connection_state, buf);
}
