Embedded Webserver Demo (No SPI Flash) Quickstart Guide
=======================================================
This application demonstrates an embedded webserver running on an XMOS device using the Ethernet sliceCARD ``XA-SK-E100``. The web pages hosted by this application is stored in the program memory.

Host computer setup
-------------------
A computer with:

* Internet browser (Internet Explorer, Chrome, Firefox, etc...)
* An Ethernet port or connected to a network router with spare Ethernet port.
* Download and install the xTIMEcomposer studio from XMOS xTIMEcomposer downloads webpage.

Hardware setup
--------------
Required sliceKIT units:

* XP-SKC-L2 sliceKIT L2 core board
* XA-SK-E100 Ethernet sliceCARD
* xTAG-2 and XA-SK-XTAG2 adapter

Setup:

* Connect the ``XA-SK-XTAG2`` adapter to the core board.
* Connect ``XTAG2`` to ``XSYS`` side (``J1``) of the ``XA-SK-XTAG2`` adapter.
* Connect the ``XTAG2`` to your computer using a USB cable.
* Connect the ``XA-SK-E100`` Ethernet sliceCARD to the ``XP-SKC-L2`` core board's ``CIRCLE`` (indicated by a white color circle (or) ``J6``) slot.
* Using an Ethernet cable, connect the other side of ``XA-SK-E100`` Ethernet sliceCARD to your computer's Ethernet port (or) to a spare Ethernet port of the router.
* Connect the 12V power supply to the core board and switch it ON.

.. figure:: images/hardware_setup.*

   Hardware setup
   
Import and build the application
--------------------------------
Importing the Webserver demo application:

* Open the xTIMEcomposer studio and ensure that it is operating in online mode. 
* Open the *Edit* perspective (Window -> Open Perspective -> XMOS Edit).
* Open the *xSOFTip* view from (Window -> Show View -> xSOFTip). An *xSOFTip* window appears on the bottom-left.
* Search for *Embedded Webserver Demo (No SPI Flash)*.
* Click and drag it into the *Project Explorer* window. Doing this will open a *Import xTIMEcomposer Software* window. Click on *Finish* to download and complete the import.
* This will also automatically import dependencies for this application.
* The application is called as *app_webserver_demo* in the *Project Explorer* window.

Building the Webserver demo application:

* Click on the *app_webserver_demo* item in the *Project Explorer* window.
* Click on the *Build* (indicated by a 'Hammer' picture) icon.
* Check the *Console* window to verify that the application has built successfully.

Run the application
-------------------

To run the application using xTIMEcomposer studio:

* In the *Project Explorer* window, locate the *app_webserver_demo.xe* in the (app_webserver_demo -> Binaries).
* Right click on *app_webserver_demo.xe* and click on (Run As -> xCORE Application).
* A *Select Device* window appears.
* Select *XMOS XTAG-2 connected to L1* and click OK.

Demo:

* The following message appears in the *Console* window of the xTIMEcomposer studio::

   Address: 0.0.0.0
   Gateway: 0.0.0.0
   Netmask: 0.0.0.0

* At this point, the XMOS device is trying to acquire an IP address in the network. Wait for some time (approximately 10 seconds) for the following message to appear in the *Console* window. Note, the IP address may be different based on your network::

   ipv4ll: 169.254.10.130
   
* Open a web browser (Firefox, etc...) in your host computer and enter the above IP address in the address bar of the browser. It opens a web page as hosted by the simple webserver running on the XMOS device.

.. figure:: images/webpage.*

   Page hosted by webserver running on XMOS device

Next steps
----------

* The (app_webserver_demo -> web) contains web pages that the webserver hosts.
* Look at the ``Embedded Webserver Function Library`` - the underlying module for this application; for details on adding a website to your application. This includes adding dynamic content in the website.
