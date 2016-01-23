#
# Copyright (c) 2015, Ari Suutari <ari@stonepile.fi>.
# All rights reserved. 
# 
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
# 
#  1. Redistributions of source code must retain the above copyright
#     notice, this list of conditions and the following disclaimer.
#  2. Redistributions in binary form must reproduce the above copyright
#     notice, this list of conditions and the following disclaimer in the
#     documentation and/or other materials provided with the distribution.
#  3. The name of the author may not be used to endorse or promote
#     products derived from this software without specific prior written
#     permission. 
# 
# THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS
# OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
# WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
# ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT,
# INDIRECT,  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
# (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
# SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
# HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
# STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
# ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
# OF THE POSSIBILITY OF SUCH DAMAGE.

#
# Compile uIP & drivers using Pico]OS library Makefile
#

RELROOT = ../picoos/
PORT ?= cortex-m
BUILD ?= RELEASE

include $(RELROOT)make/common.mak

TARGET = wiced-driver

WICED_CHIP	?= 43362A2
WICED_MCU	?= STM32F2xx
WICED_PLATFORM  ?= EMW3162
WICED_BUS	?= SDIO

SDK	= WICED-SDK-3.3.1

EXTRA_CFLAGS = -Wno-cast-align

ifeq '$(WICED_PLATFORM)' 'EMW3165'
CDEFINES += GPIO_LED_NOT_SUPPORTED
endif

#
# Pico]OS wiced stuff
#
SRC_TXT +=	picoos/WWD/wwd_rtos.c picoos/WWD/resources.c picoos/WWD/low_level_init.c

#
# WWD sources
#
SRC_TXT +=	$(SDK)/WICED/WWD/internal/wwd_management.c \
		$(SDK)/WICED/WWD/internal/wwd_internal.c \
		$(SDK)/WICED/WWD/internal/wwd_crypto.c \
		$(SDK)/WICED/WWD/internal/wwd_eapol.c \
		$(SDK)/WICED/WWD/internal/wwd_sdpcm.c \
		$(SDK)/WICED/WWD/internal/wwd_thread.c \
		$(SDK)/WICED/WWD/internal/wwd_logging.c \
		$(SDK)/WICED/WWD/internal/wwd_wifi.c \
		$(SDK)/WICED/WWD/internal/chips/$(WICED_CHIP)/wwd_ap.c
#
# bus protocols
#
SRC_TXT +=	$(SDK)/WICED/WWD/internal/bus_protocols/wwd_bus_common.c \
		$(SDK)/WICED/WWD/internal/bus_protocols/$(WICED_BUS)/wwd_bus_protocol.c \
		$(SDK)/WICED/platform/MCU/$(WICED_MCU)/WWD/wwd_$(WICED_BUS).c \
		$(SDK)/WICED/platform/MCU/$(WICED_MCU)/WWD/wwd_platform.c

#
# WWD lwip support
#
SRC_TXT +=	$(SDK)/WICED/network/LwIP/WWD/wwd_buffer.c \
		$(SDK)/WICED/network/LwIP/WWD/wwd_network.c

# platform
SRC_TXT +=	$(SDK)/WICED/platform/MCU/wwd_platform_separate_mcu.c \
		$(SDK)/WICED/platform/MCU/$(WICED_MCU)/peripherals/platform_mcu_powersave.c \
		$(SDK)/WICED/platform/MCU/$(WICED_MCU)/peripherals/platform_gpio.c \
		$(SDK)/WICED/platform/MCU/$(WICED_MCU)/peripherals/platform_rtc.c \
		$(SDK)/WICED/platform/MCU/$(WICED_MCU)/peripherals/platform_watchdog.c \
		$(SDK)/WICED/platform/MCU/STM32F2xx/platform_init.c \
		$(SDK)/platforms/$(WICED_PLATFORM)/platform.c


SRC_HDR =	$(SDK)/generated_mac_address.txt

SRC_OBJ =
CDEFINES  += 	WICED_DISABLE_BOOTLOADER \
		WICED_DISABLE_STDIO \
		WICED_DISABLE_WATCHDOG \
		FIRMWARE_WITH_PMK_CALC_SUPPORT \
		WWD_STARTUP_DELAY=10 \
		MAX_WATCHDOG_TIMEOUT_SECONDS=22 \
		NETWORK_LwIP=1 \
		MINIMAL_PLATFORM 

DIR_USRINC +=	$(SDK)/libraries/utilities/TLV \
		$(SDK)/libraries/utilities/ring_buffer \
		$(SDK)/WICED/platform/GCC

DIR_USRINC +=	$(SDK)/WICED/platform/MCU/$(WICED_MCU) \
		$(SDK)/WICED/platform/MCU/ \
		$(SDK)/WICED/platform/include \
		$(SDK)/WICED/platform/ARM_CM3 \
		$(SDK)/WICED/platform/MCU/$(WICED_MCU)/peripherals \
		$(SDK)/WICED/platform/MCU/$(WICED_MCU)/WAF \
		$(SDK)/WICED/WWD \
		$(SDK)/WICED/WWD/include \
		$(SDK)/WICED/WWD/include/network \
		$(SDK)/WICED/WWD/internal/bus_protocols/$(WICED_BUS) \
		$(SDK)/WICED/WWD/internal/chips/$(WICED_CHIP) \
		$(SDK)/WICED/network/LwIP/WWD \
		picoos/WWD ports/$(PORT) \
		$(SDK)/WICED \
		$(SDK)/platforms/$(WICED_PLATFORM) \
		$(SDK)/include

MODULES  += ../picoos-micro ../picoos-lwip

ifeq '$(strip $(DIR_OUTPUT))' ''
DIR_OUTPUT = $(CURRENTDIR)/bin
endif

include $(MAKE_LIB)

$(SDK)/generated_mac_address.txt:	$(SDK)/tools/mac_generator/mac_generator.pl
	perl $<  > $@
