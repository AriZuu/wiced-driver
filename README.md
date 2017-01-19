LwIP driver for Broadcom WICED WIFI
===================================

This library contains a LwIP driver (for picoos-lwip library). It works with
stock LwIP by using PBUF_LINK_ENCAPSULATION_HLEN to reserve space for Wiced SDK
headers that exist before ethernet header. Two-byte padding in beginning
of ethernet frame is used, but the padding word overlaps with last two bytes
of Wiced SDK headers. A little bit odd, but as LwIP never really touches the contents
of the padding word this was an easy way to get correctly aligned packet (DMA processing
in Wiced SDIO layer requires that Wiced headers begin on 32-bit boundary - at least
on STM32F2xx).

I have tested this using MXCHIP EMW3162 module + EMB-380-S2 development board (both are
available at least from seeedstudio.com). It works also with WifiMCU, which uses EMW3165.

To compile this, you'll need:

- WICED SDK package from Broadcom (free, but registration required, redistribution not allowed)
- MXCHIP patches from github (https://github.com/MXCHIP/MXCHIP-for-WICED)
  Just follow instructions there.

After SDK is extracted and patched, move it to WICED-SDK-3.7.0-7 subdirectory of this
library. Apply patch to make it work with Pico]OS:

cd WICED-SDK-3.7.0-7; patch -p1 < ../wiced.patch

The patch modifies wwd_buffer.c & wwd_network.c so that they work with stock LwIP.

If the patch doesn't apply cleanly, the problem might be the dos-style line endings
in SDK files. Issue [#1][1] contains steps the fix them.

Library doesn't use Wiced SDK Makefiles (Pico]OS Makefile system is used instead).
Also, neither LwIP nor RTOS inside SDK is used (LwIP comes from picoos-lwip
library and RTOS is, well, of course Pico]OS)

Wifi chip firmware is loaded from /firmware/43362A2.bin, it's up to the
application to provide the filesystem (romfs from picoos-micro library for example).

[1]: https://github.com/AriZuu/wiced-driver/issues/1
