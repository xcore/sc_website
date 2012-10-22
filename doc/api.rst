Webserver Module API
====================

Configuration Defines
---------------------

These defines can be set in a file called ``web_server_conf.h`` within your
application source tree.

.. doxygendefine:: WEB_SERVER_PORT
.. doxygendefine:: WEB_SERVER_USE_FLASH
.. doxygendefine:: WEB_SERVER_SEPARATE_FLASH_TASK
.. doxygendefine:: WEB_SERVER_FLASH_DEVICES
.. doxygendefine:: WEB_SERVER_NUM_FLASH_DEVICES

.. c:macro:: WEB_SERVER_POST_RENDER_FUNCTION

   This define can be set to a function that will be run after every
   rendering of part of a web page. The function gets passed the
   application and connection state and must have the type::

     void render(int app_state, int connection_state)



Web Server Functions
--------------------

The following functions can be uses in code that has a channel
connected to an XTCP server task. The functions handle all the
communication with the tcp stack.

.. doxygenfunction:: web_server_init
.. doxygenfunction:: web_server_handle_event
.. doxygenfunction:: web_server_set_app_state

Separate Flash Task Functions
-----------------------------

If ``WEB_SERVER_SEPARATE_FLASH_TASK`` is enabled then a task
with access to flash must use these functions. This task will need to
follow a pattern similar to::

    void flash_handler(chanend c_flash) {
      web_server_flash_init(flash_ports);
      while (1) {
        select {
          // Do other flash handling stuff here
          case web_server_flash(c_flash, flash_ports);
        }
      }
    }


.. doxygenfunction:: web_server_flash_init
.. doxygenfunction:: web_server_flash

In addition the task handling the xtcp connnections needs to respond
to cache events from the flash handling task. This task will need to
follow a pattern similar to::

    void tcp_handler(chanend c_xtcp, chanend c_flash) {
      xtcp_connection_t conn;
      web_server_init(c_xtcp, c_flash, null);
      while (1) {
        select
          {
          case xtcp_event(c_xtcp, conn):
            // handle non web related tcp events here
            web_server_handle_event(c_xtcp, c_flash, null, conn);
            break;
          case web_server_flash_response(c_flash):
            web_server_flash_handler(c_flash, c_xtcp);
            break;
          }
      }
    }

.. doxygenfunction:: web_server_flash_response
.. doxygenfunction:: web_server_flash_handler


.. _module_webserver_dynamic_content_api:

Functions that can be called during page rendering
--------------------------------------------------

When functions are called during page rendering (either via the ``{% ... %}``
template escaping in the webpages or via the
``WEB_SERVER_POST_RENDER_FUNCTION`` function), the following utility functions
can be called.

.. doxygenfunction:: web_server_get_param
.. doxygenfunction:: web_server_copy_param
.. doxygenfunction:: web_server_is_post
.. doxygenfunction:: web_server_get_current_file
.. doxygenfunction:: web_server_end_of_page



