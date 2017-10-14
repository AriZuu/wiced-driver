/*
 * Copyright (c) 2015, Ari Suutari <ari@stonepile.fi>.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT
 * SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT
 * OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY
 * OF SUCH DAMAGE.
 */

#include "wifi_nvram_image.h"
#include "platform/wwd_resource_interface.h"
#include "wiced_resource.h"
#include "wwd_assert.h"
#include "wiced_result.h"
#include "platform_dct.h"
#include "wiced_waf_common.h"
#include "lwip/opt.h"

#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>

#if defined(WWD_DYNAMIC_NVRAM)

#define NVRAM_SIZE             dynamic_nvram_size
#define NVRAM_IMAGE_VARIABLE   dynamic_nvram_image
uint32_t dynamic_nvram_size  = sizeof(wifi_nvram_image);
void*    dynamic_nvram_image = &wifi_nvram_image;

#else

#define NVRAM_SIZE             sizeof(wifi_nvram_image)
#define NVRAM_IMAGE_VARIABLE   wifi_nvram_image

#endif

#ifndef WDCFG_FIRMWARE
#define WDCFG_FIRMWARE "43362A2.bin"
#endif

#ifndef WDCFG_FIRMWARE_PATH
#define WDCFG_FIRMWARE_PATH "/firmware"
#endif

const char* firmwarePath[] = { WDCFG_FIRMWARE_PATH };

wwd_result_t host_platform_resource_size(wwd_resource_t resource, uint32_t* size_out)
{
  if (resource == WWD_RESOURCE_WLAN_FIRMWARE) {

    struct stat st;
    unsigned int i;
/*
 * Get size of firmware from filesystem.
 */
    for (i = 0; i < sizeof(firmwarePath) / sizeof(firmwarePath[0]); i++) {

      char buf[80];
      sprintf(buf, "%s/%s", firmwarePath[i], WDCFG_FIRMWARE);
      if (stat(buf, &st) != -1) {

        *size_out = st.st_size;
        return WWD_SUCCESS;
      }
    }

    return RESOURCE_UNSUPPORTED;
  }
  else
  {
      *size_out = NVRAM_SIZE;
  }

  return WWD_SUCCESS;
}

wwd_result_t host_platform_resource_read_indirect(wwd_resource_t resource,
                                                  uint32_t offset,
                                                  void* buffer,
                                                  uint32_t buffer_size,
                                                  uint32_t* size_out)
{
  if (resource == WWD_RESOURCE_WLAN_FIRMWARE) {

    int fd;
    unsigned int i;

    for (i = 0; i < sizeof(firmwarePath) / sizeof(firmwarePath[0]); i++) {

      char buf[80];
      sprintf(buf, "%s/%s", firmwarePath[i], WDCFG_FIRMWARE);
      fd = open(buf, O_RDONLY);
      if (fd != -1) {

        if (lseek(fd, offset, SEEK_SET) == -1) {

           *size_out = 0;
        }
        else {

           int len;

           len = read(fd, buffer, buffer_size);
           if (len == -1) {

             close(fd);
             return RESOURCE_UNSUPPORTED;
           }

           *size_out = len;
        }

        close(fd);
        return WWD_SUCCESS;
      }
    }

    return RESOURCE_UNSUPPORTED;
  }
  else {

     *size_out = MIN(buffer_size, NVRAM_SIZE - offset);
     memcpy(buffer, &NVRAM_IMAGE_VARIABLE[ offset ], *size_out);
     return WWD_SUCCESS;
  }
}
