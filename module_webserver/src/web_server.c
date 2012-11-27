#include <string.h>
#include "web_server.h"
#include "simplefs.h"
#include "print.h"
#include "flash.h"
#include "stdlib.h"

#ifdef __web_server_conf_h_exists__
#include "web_server_conf.h"
#endif

#ifndef WEB_SERVER_PORT
#define WEB_SERVER_PORT 80
#endif

#ifndef WEB_SERVER_NUM_CONNECTIONS
#define WEB_SERVER_NUM_CONNECTIONS 5
#endif

#ifndef WEB_SERVER_MAX_URI_LENGTH
#define WEB_SERVER_MAX_URI_LENGTH 64
#endif

#ifndef WEB_SERVER_MAX_PARAMS_LENGTH
#define WEB_SERVER_MAX_PARAMS_LENGTH 64
#endif

#ifndef WEB_SERVER_SEND_BUF_SIZE
#define WEB_SERVER_SEND_BUF_SIZE 128
#endif

#ifndef WEB_SERVER_CHECK_FLASH_SIGNATURE
#define WEB_SERVER_CHECK_FLASH_SIGNATURE 1
#endif

typedef enum {
  PARSING_METHOD,
  PARSING_URI,
  PARSING_HEADERS,
  PARSING_HEADER,
  PARSING_PARAMS,
  PARSING_IDLE
} parsing_state_t;

typedef enum {
  REQUEST_UNKNOWN,
  REQUEST_GET,
  REQUEST_POST
} request_method_t;

typedef struct connection_state_t {
  int active;
  int conn_id;
  int is_template;
  char current_data[WEB_SERVER_SEND_BUF_SIZE];
  int  current_data_len;
  simplefs_addr_t next_data;
  simplefs_addr_t end_of_data;
  char params[WEB_SERVER_MAX_PARAMS_LENGTH+1];
  int params_len;
  int content_len;
  int sending_paused;
  parsing_state_t parsing_state;
  request_method_t request_method;
  file_handle_t file;
} connection_state_t;

static connection_state_t connection_state[WEB_SERVER_NUM_CONNECTIONS];
static int app_state = 0;

#if WEB_SERVER_USE_FLASH
void web_server_check_signature(fl_SPIPorts *flash_ports)
{
  int data[1];
  fl_connectToDevice(flash_ports,
                      WEB_SERVER_FLASH_DEVICES,
                      WEB_SERVER_NUM_FLASH_DEVICES);
  fl_readData(0, 4, (unsigned char *) data);
  if (data[0] != 0x03eb517e) {
    printstrln("ERROR: Web pages have not been programmed to flash\n");
  }
  fl_disconnect();
}
#endif

void web_server_init(chanend c_xtcp, chanend c_flash, fl_SPIPorts *fports)
{
  #if WEB_SERVER_USE_FLASH && !WEB_SERVER_SEPARATE_FLASH_TASK && WEB_SERVER_CHECK_FLASH_SIGNATURE
  web_server_check_signature(fports);
  #endif
  simplefs_init(fports);
  xtcp_listen(c_xtcp, WEB_SERVER_PORT, XTCP_PROTOCOL_TCP);
  for (int i=0;i<WEB_SERVER_NUM_CONNECTIONS;i++) {
    connection_state[i].active = 0;
  }
}

void web_server_set_app_state(int st)
{
  app_state = st;
}

static char *skip_word(char *p, char *end)
{
  p += strlen(p) + 1;
  return p;
}

int web_server_is_post(int st0)
{
  connection_state_t *st = (connection_state_t *) st0;
  return (st->request_method == REQUEST_POST);
}

int web_server_end_of_page(int st0) {
  connection_state_t *st = (connection_state_t *) st0;
  return (st->next_data >= st->end_of_data);
}

file_handle_t web_server_get_current_file(int st0) {
  connection_state_t *st = (connection_state_t *) st0;
  return (st->file);
}

char * web_server_get_param(const char *param,
                            int st0)
{
  connection_state_t *st = (connection_state_t *) st0;
  char *p = st->params;
  char *end = st->params + st->params_len;

  while (p < end) {
    if (strncmp(p, param, end-p)==0) {
      p = skip_word(p, end);
      return (p < end ? p : NULL);
    }
    else {
      p = skip_word(p, end);
      p = skip_word(p, end);
    }
  }

  return NULL;
}

int web_server_copy_param(const char param[], int st, char buf[])
{
  char *val = web_server_get_param(param, st);
  if (val) {
    strcpy(buf, val);
    return strlen(val);
  }

  return 0;
}



static connection_state_t * get_new_state() {
  for (int i=0;i<WEB_SERVER_NUM_CONNECTIONS;i++) {
    if (!connection_state[i].active) {
      connection_state[i].active = 1;
      connection_state[i].parsing_state = PARSING_METHOD;
      connection_state[i].request_method = REQUEST_UNKNOWN;
      connection_state[i].params_len = 0;
      connection_state[i].content_len = -1;
      connection_state[i].sending_paused = 0;
      return &connection_state[i];
    }
  }
  return NULL;
}


static void get_resource(connection_state_t *st,
                         const char *uri)
{
  fs_file_t *f;

  if ((strcmp(uri,"/")==0) || *uri==0)
    f = (fs_file_t *) simplefs_get_file("index.html");
  else
    f = (fs_file_t *) simplefs_get_file(uri);

  if (!f)
    f = (fs_file_t *) simplefs_get_file("404.html");

  if (f) {
    st->is_template = (f->ftype == FS_TYPE_TEMPLATE);
    st->next_data = f->data;
    st->end_of_data = f->data + f->length;
    st->file = (file_handle_t) f;
  }
  else {
    st->next_data = 0;
    st->end_of_data = 0;
    st->file = 0;
  }

}

static void set_content_length(connection_state_t *st)
{
  if (strncmp(st->current_data,"Content-Length",14)==0) {
    char *end = st->current_data + st->current_data_len - 1;
    char *p = skip_word(st->current_data, end)+1;
    int len;
    len = atoi(p);
    st->content_len = len;
  }
}

static void parse_http_request(chanend c_xtcp,
                               xtcp_connection_t *conn,
                               connection_state_t *st,
                               char *buf,
                               int len)
{
  // We make the assumption that the uri  is contained in the fisrt
  // chunk of data we get so we don't have to maintain a partial uri string
  // per connection
  char *end = buf+len;
  char uri[WEB_SERVER_MAX_URI_LENGTH+1];
  int uri_len = 0;

  while (buf < end) {
    switch (st->parsing_state)
      {
      case PARSING_METHOD:
        if (strncmp(buf,"GET ",4)==0) {
          st->request_method = REQUEST_GET;
          buf += 4;
          st->parsing_state = PARSING_URI;
        } else if (strncmp(buf,"POST ",5)==0) {
          st->request_method = REQUEST_POST;
          buf += 5;
          st->parsing_state = PARSING_URI;
        } else {
          st->parsing_state = PARSING_IDLE;
          xtcp_abort(c_xtcp, conn);
          return;
        }
        break;
      case PARSING_URI:
        switch (*buf) {
        case ' ':
          uri[uri_len] = 0;
          get_resource(st, uri);
          st->parsing_state = PARSING_HEADERS;
          break;
        case '?':
          uri[uri_len] = 0;
          get_resource(st, uri);
          st->parsing_state = PARSING_PARAMS;
          break;
        default:
          if (uri_len < WEB_SERVER_MAX_URI_LENGTH) {
            uri[uri_len] = *buf;
            uri_len++;
          }
          break;
        }
        buf++;
        break;
      case PARSING_HEADERS:
        switch (*buf)
          {
          case 13:
            buf++;
          case 10:
            buf++;
            if (st->request_method == REQUEST_POST)
              st->parsing_state = PARSING_PARAMS;
            else {
              xtcp_init_send(c_xtcp, conn);
              st->parsing_state = PARSING_IDLE;
            }
            break;
          default:
            // This code reuses the current_data array to store
            // the header names
            st->current_data[0] = *buf;
            buf++;
            st->parsing_state = PARSING_HEADER;
            st->current_data_len = 1;
            break;
          }
        break;
      case PARSING_HEADER:
        switch (*buf)
          {
          case 13:
            buf++;
          case 10:
            buf++;
            st->current_data[st->current_data_len] = 0;
            set_content_length(st);
            st->parsing_state = PARSING_HEADERS;
            break;
          default:
            if (st->current_data_len < WEB_SERVER_SEND_BUF_SIZE-1) {
              if (*buf==':')
                st->current_data[st->current_data_len] = 0;
              else
                st->current_data[st->current_data_len] = *buf;
              st->current_data_len++;
            }
            buf++;
            break;
          }
        break;
      case PARSING_PARAMS:
        switch (*buf)
          {
          case ' ':
            st->params[st->params_len] = 0;
            st->params_len++;
            st->parsing_state = PARSING_HEADERS;
            break;
          case 13:
          case 10:
            st->params[st->params_len] = 0;
            st->params_len++;
            xtcp_init_send(c_xtcp, conn);
            st->parsing_state = PARSING_IDLE;
            break;
          case '&':
          case '=':
            if (st->params_len < WEB_SERVER_MAX_PARAMS_LENGTH) {
              st->params[st->params_len] = 0;
              st->params_len++;
            }
            break;
          default:
            if (st->params_len < WEB_SERVER_MAX_PARAMS_LENGTH) {
              st->params[st->params_len] = *buf;
              st->params_len++;
            }
            break;
          }
        if (st->content_len > 0) {
          st->content_len--;
        }
        buf++;
        break;
      case PARSING_IDLE:
        // Nothing to do
        return;
      }
  }
  if (st->parsing_state == PARSING_PARAMS &&
      st->content_len == 0) {
    st->params[st->params_len] = 0;
    xtcp_init_send(c_xtcp, conn);
    st->parsing_state = PARSING_IDLE;
  }
}

extern int web_server_dyn_expr(int exp,
                               char *buf,
                               int app_state,
                               int connection_state);

void web_server_unpause_senders(chanend c_flash, chanend c_xtcp)
{
  for (int i=0;i<WEB_SERVER_NUM_CONNECTIONS;i++) {
    connection_state_t *st = &connection_state[i];
    if (st->active &&
        st->sending_paused &&
        simplefs_data_available(c_flash,
                                st->next_data,
                                WEB_SERVER_SEND_BUF_SIZE))
      {
        xtcp_connection_t conn;
        conn.id = st->conn_id;
        xtcp_init_send(c_xtcp, &conn);
        st->sending_paused = 0;
      }
  }
}

static void update_data_cache(chanend c_flash)
{
  simplefs_addr_t addr=-1;

  if (simplefs_request_pending())
    return;

  for (int i=0;i<WEB_SERVER_NUM_CONNECTIONS;i++) {
    connection_state_t *st = &connection_state[i];
    if (st->active) {
      if (simplefs_data_available(c_flash,
                                  st->next_data,
                                  WEB_SERVER_SEND_BUF_SIZE))
        // Data is available that could be used
        return;
      if (st->next_data < st->end_of_data)
        addr = st->next_data;
    }
  }
  // Nothing is available, lets ask for some more data
  if (addr != -1)
    simplefs_request_data(c_flash, addr);
  return;
}

static void prepare_data(chanend c_flash, connection_state_t *st)
{
  int getlen = st->end_of_data - st->next_data;
  if (getlen > WEB_SERVER_SEND_BUF_SIZE)
    getlen = WEB_SERVER_SEND_BUF_SIZE;

  char *src0 = simplefs_get_data(c_flash, st->next_data, getlen);
  char *eod = src0 + getlen;
  char *dst = st->current_data;
  char *src = src0;
  int len = 0;

  if (!st->is_template) {
    memcpy(dst, src, getlen);
    st->current_data_len = getlen;
    st->next_data += getlen;
    return;
  }


  int finished = 0;
  while (!finished) {
    switch (*src) {
    case 255: {
      int dyn_expr_id = *(src+1);
      char buf[WEB_SERVER_SEND_BUF_SIZE];
      int dyn_expr_len = web_server_dyn_expr(dyn_expr_id, buf,
                                             app_state, (int) st);

      if (dyn_expr_len > WEB_SERVER_SEND_BUF_SIZE) {
        // Can't possibly transmit this, so skip it
        src += 2;
        finished = (src >= eod);
      }
      else if (len + dyn_expr_len > WEB_SERVER_SEND_BUF_SIZE) {
        finished = 1;
      }
      else {
        memcpy(dst, buf, dyn_expr_len);
        src += 2;
        dst += dyn_expr_len;
        len += dyn_expr_len;
        finished = (len >= WEB_SERVER_SEND_BUF_SIZE || src >= eod);
      }
      }
      break;
    default:
      *dst = *src;
      dst++;
      src++;
      len++;
      finished = (len >= WEB_SERVER_SEND_BUF_SIZE || src >= eod);
      break;
    }
  }

  st->current_data_len = len;
  st->next_data += (src - src0);
}

void web_server_handle_event(chanend c_xtcp,
                             chanend c_flash,
                             fl_SPIPorts *flash_ports,
                             xtcp_connection_t *conn)
{
  char inbuf[XTCP_MAX_RECEIVE_SIZE];

  switch (conn->event) {
    case XTCP_IFUP:
    case XTCP_IFDOWN:
    case XTCP_ALREADY_HANDLED:
      return;
    default:
      break;
  }

  if (conn->local_port == WEB_SERVER_PORT) {
    connection_state_t *st = (connection_state_t *) conn->appstate;
    switch (conn->event)
      {
      case XTCP_NEW_CONNECTION:
        st = get_new_state();
        if (st) {
          st->conn_id = conn->id;
          xtcp_set_connection_appstate(c_xtcp, conn, (unsigned) st);
        }
        else
          xtcp_abort(c_xtcp, conn);
        break;
      case XTCP_RECV_DATA: {
        int len = xtcp_recv(c_xtcp, inbuf);
        if (st)
          parse_http_request(c_xtcp, conn, st, inbuf, len);
        }
        break;
      case XTCP_REQUEST_DATA:
      case XTCP_SENT_DATA:
        if (!st || !st->active) {
          xtcp_complete_send(c_xtcp);
        }
        else if (st->next_data >= st->end_of_data) {
          xtcp_complete_send(c_xtcp);
          xtcp_close(c_xtcp, conn);
          st->active = 0;
        }
        else {
          if (simplefs_data_available(c_flash,
                                      st->next_data,
                                      WEB_SERVER_SEND_BUF_SIZE)) {
            prepare_data(c_flash, st);
            xtcp_send(c_xtcp, st->current_data, st->current_data_len);
            #ifdef WEB_SERVER_POST_RENDER_FUNCTION
            WEB_SERVER_POST_RENDER_FUNCTION((int) app_state, (int) st);
            #endif
            update_data_cache(c_flash);
          }
          else {
            st->sending_paused = 1;
            xtcp_complete_send(c_xtcp);
            update_data_cache(c_flash);
          }

        }
        break;
      case XTCP_RESEND_DATA:
        xtcp_send(c_xtcp, st->current_data, st->current_data_len);
        break;
      case XTCP_CLOSED:
      case XTCP_ABORTED:
      case XTCP_TIMED_OUT:
        if (st) {
          st->active = 0;
          update_data_cache(c_flash);
        }
        break;
      default:
        break;
      }
    conn->event = XTCP_ALREADY_HANDLED;
  }
  return;
}


