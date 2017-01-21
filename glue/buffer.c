/*
 * Copyright (c) 2017, Ari Suutari <ari@stonepile.fi>.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 *  1. Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *  2. Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *  3. The name of the author may not be used to endorse or promote
 *     products derived from this software without specific prior written
 *     permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT,
 * INDIRECT,  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <picoos.h>
#include <string.h>

#include "lwip/netbuf.h"
#include "lwip/memp.h"

#include "network/wwd_buffer_interface.h"
#include "platform/wwd_bus_interface.h"
#include "RTOS/wwd_rtos_interface.h"
#include "wiced_utilities.h"

wwd_result_t wwd_buffer_init(void* arg)
{
  return WWD_SUCCESS;
}

wwd_result_t host_buffer_get(wiced_buffer_t* buffer,
                             wwd_buffer_dir_t direction,
                             unsigned short size,
                             wiced_bool_t wait)
{
  P_ASSERT("bufsize valid", size != 0);
  
  *buffer = NULL;
  if (size > WICED_LINK_MTU)
    return WWD_BUFFER_UNAVAILABLE_PERMANENT;

  do {
    
    *buffer = pbuf_alloc(PBUF_RAW, size, PBUF_POOL);
    if (wait && *buffer == NULL)
      posTaskSleep(MS(1));

  } while (wait && *buffer == NULL);

  if (*buffer == NULL)
    return WWD_BUFFER_UNAVAILABLE_TEMPORARY;
    
  return WWD_SUCCESS;
}

wwd_result_t internal_host_buffer_get(wiced_buffer_t* buffer,
                                      wwd_buffer_dir_t direction,
                                      unsigned short size,
                                      unsigned long timeout)
{
  P_ASSERT("bufsize valid", size != 0);

  *buffer = NULL;
  if (size > WICED_LINK_MTU)
    return WWD_BUFFER_UNAVAILABLE_PERMANENT;

  do {

    *buffer = pbuf_alloc(PBUF_RAW, size, PBUF_POOL);
    if (timeout && *buffer == NULL)
      posTaskSleep(MS(1));

  } while (timeout-- && *buffer == NULL);

  if (*buffer == NULL)
    return WWD_BUFFER_UNAVAILABLE_TEMPORARY;

  return WWD_SUCCESS;
}

void host_buffer_release(wiced_buffer_t buffer, wwd_buffer_dir_t direction)
{
  P_ASSERT("pbuf valid", buffer != NULL);
  pbuf_free(buffer);
}

uint8_t* host_buffer_get_current_piece_data_pointer(wiced_buffer_t buffer)
{
  P_ASSERT("pbuf valid", buffer != NULL);
  return (uint8_t*)buffer->payload;
}

uint16_t host_buffer_get_current_piece_size(wiced_buffer_t buffer)
{
  P_ASSERT("pbuf valid", buffer != NULL);
  return (uint16_t)buffer->len;
}

wiced_buffer_t host_buffer_get_next_piece(wiced_buffer_t buffer)
{
  P_ASSERT("pbuf valid", buffer != NULL);
  return buffer->next;
}

wwd_result_t host_buffer_add_remove_at_front(wiced_buffer_t* buffer, int32_t amount)
{
  P_ASSERT("pbuf valid", buffer != NULL);
  if (pbuf_header(*buffer, -amount) != 0)
    return WWD_BUFFER_POINTER_MOVE_ERROR;

  return WWD_SUCCESS;
}

wwd_result_t host_buffer_set_size(wiced_buffer_t buffer, unsigned short size)
{
  if (size > WICED_LINK_MTU)
    return WWD_BUFFER_SIZE_SET_ERROR;

  buffer->tot_len = size;
  buffer->len     = size;

  return WWD_SUCCESS;
}
