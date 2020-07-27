/*
 * Copyright (c) 2013-2018 Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the License); you may
 * not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an AS IS BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * -----------------------------------------------------------------------------
 *
 * $Revision:   V5.1.0
 *
 * Project:     CMSIS-RTOS RTX
 * Title:       RTX Configuration
 *
 * -----------------------------------------------------------------------------
 */
 
#include "cmsis_compiler.h"
#include "rtx_os.h"
#include "stm32f10x.h"                  // Device header
#include "app_cfg.h"

 
// OS Idle Thread
__WEAK __NO_RETURN void osRtxIdleThread (void *argument) {
  (void)argument;

  for (;;) {}
}
 
// OS Error Callback function
__WEAK uint32_t osRtxErrorNotify (uint32_t code, void *object_id) {
  (void)object_id;

  switch (code) {
    case osRtxErrorStackUnderflow:
      // Stack overflow detected for thread (thread_id=object_id)
      APP_TRACE_INFO(("Stack overflow detected for thread_id : 0x%X\r\n", object_id));
      break;
    case osRtxErrorISRQueueOverflow:
      // ISR Queue overflow detected when inserting object (object_id)
      APP_TRACE_INFO(("ISR Queue overflow detected when inserting object : %i\r", object_id));
      break;
    case osRtxErrorTimerQueueOverflow:
      // User Timer Callback Queue overflow detected for timer (timer_id=object_id)
      APP_TRACE_INFO(("User Timer Callback Queue overflow detected for timer_id : %i\r", object_id));
      break;
    case osRtxErrorClibSpace:
      // Standard C/C++ library libspace not available: increase OS_THREAD_LIBSPACE_NUM
      APP_TRACE_INFO(("Standard C/C++ library libspace not available: increase OS_THREAD_LIBSPACE_NUM\r"));
      break;
    case osRtxErrorClibMutex:
      // Standard C/C++ library mutex initialization failed
      APP_TRACE_INFO(("Standard C/C++ library mutex initialization failed\r"));
      break;
    default:
      // Reserved
      break;
  }
  //for (;;) {}
  __NVIC_SystemReset();
//return 0U;
}
