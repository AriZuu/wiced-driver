/*
 * Copyright (c) 2016, Ari Suutari <ari@stonepile.fi>.
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
#include <wiced-driver.h>
#include "platform_init.h"
#include "platform_constants.h"
#include "platform_peripheral.h"

platform_result_t platform_mcu_powersave_init()
{
#if POSCFG_FEATURE_POWER
  return PLATFORM_SUCCESS;
#else
  return PLATFORM_FEATURE_DISABLED;
#endif
}

platform_result_t platform_mcu_powersave_disable()
{
#if POSCFG_FEATURE_POWER
  posPowerDisableSleep();
  return PLATFORM_SUCCESS;
#else
  return PLATFORM_FEATURE_DISABLED;
#endif
}
 
platform_result_t platform_mcu_powersave_enable()
{
#if POSCFG_FEATURE_POWER
  posPowerEnableSleep();
  return PLATFORM_SUCCESS;
#else
  return PLATFORM_FEATURE_DISABLED;
#endif
}

void platform_mcu_powersave_exit_notify()
{
}

platform_result_t platform_rtc_init()
{
  return PLATFORM_FEATURE_DISABLED;
}
