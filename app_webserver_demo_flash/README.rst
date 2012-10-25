Embedded Webserver Demo (SPI Flash)
======================================

:scope: Example
:description: A simple demo of the embedded website component
:keywords: ethernet, http, tcp, ip, webserver

This is a simple demonstration of the emebedded wesbite component.
It uses the mode of the component where the website is stored on SPI flash.

 * The program is set up by default to target the sliceKIT Core Board
   (L2) with an ethernet slice in the CIRCLE slot
 * The TCP stack is configured to use DHCP to gain an IP address. Once
   this happens the address will be printed. Visit that address with a
   web browser connected to the same network as the ethernet slice to
   see the app running.
 * This program requires you to flash the data onto the SPI flash
   before running it. See
   :ref:`the webserver library programming guide <module_webserver_flashing_pages>` for details.
