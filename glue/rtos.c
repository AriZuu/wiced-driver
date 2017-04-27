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

#include "wwd_rtos.h"
#include <stdint.h>
#include "wwd_constants.h"
#include "wwd_assert.h"
#include "RTOS/wwd_rtos_interface.h"
#include "wiced_utilities.h"

#define TMO2TICKS(t) (t == NEVER_TIMEOUT ? INFINITE : MS(t))

/*
 * Create thrad.
 */
wwd_result_t host_rtos_create_thread(host_thread_type_t* thread,
                                     void (*entryFunction)(uint32_t),
                                     const char* name,
                                     void* stack,
                                     uint32_t stackSize,
                                     uint32_t priority)
{
  return host_rtos_create_thread_with_arg(thread,
                                          entryFunction,
                                          name,
                                          stack,
                                          stackSize,
                                          priority, 
                                          0);
}

/*
 * Create thread with arg.
 */
wwd_result_t host_rtos_create_thread_with_arg(host_thread_type_t* thread,
                                              void (*entryFunction)(uint32_t),
                                              const char* name,
                                              void* stack,
                                              uint32_t stackSize,
                                              uint32_t priority,
                                              uint32_t arg)
{
  P_ASSERT("Cannot use pre-allocated thread stack.", stack == NULL);

  *thread = nosTaskCreate((POSTASKFUNC_t)entryFunction, (void*)arg, priority, stackSize, name);
  if (*thread == NULL)
    return WWD_THREAD_CREATE_FAILED;

  return WWD_SUCCESS;
}

/*
 * Terminate thread, can act only on current one.
 */
wwd_result_t host_rtos_finish_thread(host_thread_type_t* thread)
{
  P_ASSERT("Cannot delete thread other than current one.", *thread == nosTaskGetCurrent());
  nosTaskExit();
  return WWD_SUCCESS;
}


/*
 * Wait for another thead to terminate.
 * Not very sophisticated implementation, just loops and waits.
 */
wwd_result_t host_rtos_join_thread(host_thread_type_t* thread)
{
  while (!nosTaskUnused(*thread)) {

    nosTaskSleep(MS(10));
  }

  return WWD_SUCCESS;
}

/*
 * Delete terminated thread.
 * Not needed for Pico]OS.
 */
wwd_result_t host_rtos_delete_terminated_thread(host_thread_type_t* thread)
{
  return WWD_SUCCESS;
}


/*
 * Create semaphore.
 */
wwd_result_t host_rtos_init_semaphore(host_semaphore_type_t* semaphore)
{
  *semaphore = nosSemaCreate(0, 0, "wwd*");
  if (*semaphore == NULL)
    return WWD_SEMAPHORE_ERROR;

  return WWD_SUCCESS;
}


/*
 * Get (wait for) semaphore.
 */
wwd_result_t host_rtos_get_semaphore(host_semaphore_type_t* semaphore,
                                     uint32_t timeoutMS,
                                     wiced_bool_t isISR)
{
  if (nosSemaWait(*semaphore, TMO2TICKS(timeoutMS)))
    return WWD_TIMEOUT;

  return WWD_SUCCESS;
}


/*
 * Set a semaphore.
 */
wwd_result_t host_rtos_set_semaphore(host_semaphore_type_t* semaphore, wiced_bool_t fromISR)
{
  P_ASSERT("fromISR / posInInterrupt_g mismatch.", (fromISR == 0) == (posInInterrupt_g == 0));
  nosSemaSignal(*semaphore);
  return WWD_SUCCESS;
}


/*
 * Destroy semamphore.
 */
wwd_result_t host_rtos_deinit_semaphore(host_semaphore_type_t* semaphore)
{
  if (semaphore != NULL) {

    nosSemaDestroy(*semaphore);
    *semaphore = NULL;
  }

  return WWD_SUCCESS;
}


/*
 * Get time in milliseconds since RTOS start.
 */
wwd_time_t host_rtos_get_time(void)
{
  return (wwd_time_t) (jiffies * (1000 / HZ));
}


/*
 * Sleep.
 */
wwd_result_t host_rtos_delay_milliseconds(uint32_t ms)
{
  nosTaskSleep(MS(ms));
  return WWD_SUCCESS;
}

/*
 * Create queue.
 */
wwd_result_t host_rtos_init_queue(host_queue_type_t* queue,
                                  void* buffer,
                                  uint32_t bufferSize,
                                  uint32_t messageSize )
{
  *queue = uosRingCreate(messageSize, bufferSize / messageSize);
  if (*queue == NULL)
    return WWD_QUEUE_ERROR;

  return WWD_SUCCESS;
}

/*
 * Put message to queue.
 */
wwd_result_t host_rtos_push_to_queue(host_queue_type_t* queue,
                                     void* message,
                                     uint32_t timeout)
{
  if (!uosRingPut(*queue, message, TMO2TICKS(timeout)))
    return WWD_TIMEOUT;

  return WWD_SUCCESS;
}

/* 
 * Get message from queue.
 */
wwd_result_t host_rtos_pop_from_queue(host_queue_type_t* queue,
                                      void* message,
                                      uint32_t timeout)
{
  if (!uosRingGet(*queue, message, TMO2TICKS(timeout)))
    return WWD_TIMEOUT;

  return WWD_SUCCESS;
}

/*
 * Destroy queue.
 */
wwd_result_t host_rtos_deinit_queue(host_queue_type_t* queue)
{
  uosRingDestroy(*queue);
  return WWD_SUCCESS;
}
