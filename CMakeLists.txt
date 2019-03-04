#
# Copyright (c) 2019, Ari Suutari <ari@stonepile.fi>.
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

include_guard(GLOBAL)

set(WICED_VERSION 6.2)
set(SDK	WICED-SDK-${WICED_VERSION})
set(WICED_SDK ${CMAKE_CURRENT_SOURCE_DIR}/${SDK} PARENT_SCOPE)

#
# Pico]OS wiced stuff
#
set(SRC
    glue/rtos.c
    glue/resources.c
    glue/low_level_init.c
    utils.c
    powersave.c)

#
# WWD sources
#
list(APPEND SRC
     ${SDK}/WICED/WWD/internal/wwd_management.c
     ${SDK}/WICED/WWD/internal/wwd_debug.c
     ${SDK}/WICED/WWD/internal/wwd_internal.c
     ${SDK}/WICED/WWD/internal/wwd_eapol.c
     ${SDK}/WICED/WWD/internal/wwd_sdpcm.c
     ${SDK}/WICED/WWD/internal/wwd_thread.c
     ${SDK}/WICED/WWD/internal/wwd_thread_internal.c
     ${SDK}/WICED/WWD/internal/wwd_logging.c
     ${SDK}/WICED/WWD/internal/wwd_ap_common.c
     ${SDK}/WICED/WWD/internal/wwd_wifi.c
     ${SDK}/WICED/WWD/internal/wwd_wifi_sleep.c
     ${SDK}/WICED/WWD/internal/wwd_wifi_chip_common.c
     ${SDK}/WICED/WWD/internal/chips/${WICED_CHIP}/wwd_ap.c
     ${SDK}/WICED/WWD/internal/chips/${WICED_CHIP}/wwd_chip_specific_functions.c)
#
# bus protocols
#
list(APPEND SRC
     ${SDK}/WICED/WWD/internal/bus_protocols/wwd_bus_common.c
     ${SDK}/WICED/WWD/internal/bus_protocols/${WICED_BUS}/wwd_bus_protocol.c
     ${SDK}/WICED/platform/MCU/${WICED_MCU}/WWD/wwd_${WICED_BUS}.c
     ${SDK}/WICED/platform/MCU/${WICED_MCU}/WWD/wwd_platform.c)

#
# WWD lwip support
#
list(APPEND SRC
     glue/buffer.c
     glue/wlan_if.c)

# platform
list(APPEND SRC
     ${SDK}/WICED/platform/MCU/wwd_platform_separate_mcu.c
     ${SDK}/WICED/platform/MCU/${WICED_MCU}/peripherals/platform_gpio.c
     ${SDK}/WICED/platform/MCU/${WICED_MCU}/peripherals/platform_watchdog.c
     ${SDK}/WICED/platform/MCU/${WICED_MCU}/platform_init.c
     ${SDK}/platforms/${WICED_PLATFORM}/platform.c)


add_peer_directory(${PICOOS_DIR})
add_peer_directory(../picoos-lwip)
add_peer_directory(../picoos-micro)

add_library(wiced-driver STATIC ${SRC})

target_include_directories(wiced-driver
  PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(wiced-driver picoos-lwip picoos-micro picoos)
target_compile_options(wiced-driver PRIVATE -Wno-cast-align -Wno-missing-field-initializers)

set(INC
    ${SDK}/WICED/WWD/include
    ${SDK}/WICED/WWD/
    ${SDK}/WICED/WWD/include/network
    ${SDK}/WICED/WWD/internal/chips/${WICED_CHIP}
    ${SDK}/WICED/WWD/internal/bus_protocols/${WICED_BUS}
    ${SDK}/WICED/platform/include
    ${SDK}/include
    ${SDK}/platforms/${WICED_PLATFORM}
    glue glue/ports/${PORT} ${CMAKE_CURRENT_SOURCE_DIR})

if(CORTEX STREQUAL "m3")
list(APPEND INC ${SDK}/WICED/platform/ARM_CM3)
endif()

if(CORTEX STREQUAL "m4")
list(APPEND INC ${SDK}/WICED/platform/ARM_CM4)
endif()

target_include_directories(wiced-driver
  PRIVATE
    ${SDK}/libraries/utilities/TLV
    ${SDK}/libraries/utilities/ring_buffer
    ${SDK}/WICED/security/BESL/include
    ${SDK}/WICED/platform/GCC
    ${SDK}/WICED/platform/MCU/${WICED_MCU}
    ${SDK}/WICED/platform/MCU/
    ${SDK}/WICED/platform/MCU/${WICED_MCU}/peripherals
    ${SDK}/WICED/platform/MCU/${WICED_MCU}/WAF
    ${SDK}/WICED/WWD/include/RTOS
    ${SDK}/WICED)

#
# Provide public headers via lwipcore library, because
# it needs them also.
#
target_include_directories(lwipcore PUBLIC ${INC})

target_compile_definitions(wiced-driver
  PRIVATE
    PICOOS_WORKAROUNDS
    FIRMWARE_WITH_PMK_CALC_SUPPORT
    WWD_STARTUP_DELAY=10
    MAX_WATCHDOG_TIMEOUT_SECONDS=22
  PUBLIC
    WICED_DISABLE_BOOTLOADER
    WICED_DISABLE_STDIO
    WICED_DISABLE_WATCHDOG
    NETWORK_LwIP=1
    WICED_SDK_VERSION=\"${WICED_VERSION}\")

if(WICED_PLATFORM STREQUAL "EMW3165")
target_compile_definitions(wiced-driver PRIVATE GPIO_LED_NOT_SUPPORTED)
endif()

