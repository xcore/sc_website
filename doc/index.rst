Embedded Webserver Library Programming Guide
============================================

This software block allows you to generate a webserver that
communicates using the XMOS TCP/IP server component. It has the
following core features:

 * Automatically package a file tree of web pages into data that be
   accessed on the device
 * Store web pages in either program memory or on an attached SPI
   flash
 * Call C/XC functions from within web page templates to access
   dynamic content
 * Handle GET and POST HTTP requests
 * Separate the handling of TCP traffic and the access of flash into
   different tasks passing data over XC channels. Allowing you to
   integrate the webserver in other applications that already handle
   TCP or access flash.

.. toctree::

   getting_started
   programming_guide
   api
