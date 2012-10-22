#ifndef __web_page_functions_h__
#define __web_page_functions_h__

void init_web_state();
void post_page_render_increment_counter(int app_state0, int connection_state);
int get_counter_value(int app_state0, char buf[]);
int get_input_param(int connection_state, char buf[]);
int get_timer_value(char buf[], int x);

#endif // __web_page_functions_h__
