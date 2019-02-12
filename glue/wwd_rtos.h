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

#ifndef INCLUDED_WWD_RTOS_H_
#define INCLUDED_WWD_RTOS_H_

#include <picoos.h>
#include <picoos-u.h>

#define RTOS_HIGHEST_PRIORITY           (POSCFG_MAX_PRIO_LEVEL - 1)
#define RTOS_LOWEST_PRIORITY            0
#define RTOS_DEFAULT_THREAD_PRIORITY    1

#define RTOS_HIGHER_PRIORTIY_THAN(x)    (x < RTOS_HIGHEST_PRIORITY ? (x + 1) : RTOS_HIGHEST_PRIORITY)
#define RTOS_LOWER_PRIORTIY_THAN(x)     (x > RTOS_LOWEST_PRIORITY ? (x - 1) : RTOS_LOWEST_PRIORITY)

#define RTOS_USE_DYNAMIC_THREAD_STACK
#define WWD_THREAD_STACK_SIZE           (3000)

typedef POSTASK_t host_thread_type_t;
typedef POSSEMA_t host_semaphore_type_t;
typedef POSMUTEX_t host_mutex_type_t;
typedef UosRing* host_queue_type_t;
typedef struct {

    uint8_t dummy;
} host_rtos_thread_config_type_t;


#endif /* ifndef INCLUDED_WWD_RTOS_H_ */
