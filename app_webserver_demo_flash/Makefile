# The TARGET variable determines what target system the application is
# compiled for. It either refers to an XN file in the source directories
# or a valid argument for the --target option when compiling
TARGET = SLICEKIT-L2

# The APP_NAME variable determines the name of the final .xe file. It should
# not include the .xe postfix. If left blank the name will default to
# the project name
APP_NAME =

# The USED_MODULES variable lists other module used by the application.
USED_MODULES = module_webserver module_ethernet_board_support module_slicekit_support

# The flags passed to xcc when building the application
# You can also set the following to override flags for a particular language:
# XCC_XC_FLAGS, XCC_C_FLAGS, XCC_ASM_FLAGS, XCC_CPP_FLAGS
# If the variable XCC_MAP_FLAGS is set it overrides the flags passed to
# xcc for the final link (mapping) stage.

XCC_FLAGS_flash_in_tcp_handler = -g -O2  -DSEPARATE_FLASH_TASK=0 -lflash
XCC_FLAGS_separate_flash_task = -g -O2 -DSEPARATE_FLASH_TASK=1 -lflash

# The VERBOSE variable, if set to 1, enables verbose output from the make
# system.
VERBOSE = 0

.PHONY: flash

flash:
	xflash --boot-partition-size 0x10000 bin/flash_in_tcp_handler/app_webserver_demo_flash_flash_in_tcp_handler.xe --data bin/flash_in_tcp_handler/web_data.bin


#=============================================================================
# The following part of the Makefile includes the common build infrastructure
# for compiling XMOS applications. You should not need to edit below here.

XMOS_MAKE_PATH ?= ../..
include $(XMOS_MAKE_PATH)/xcommon/module_xcommon/build/Makefile.common
