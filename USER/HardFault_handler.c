/******************************************************************************
 * @file     HardFault_Handler.c
 * @brief    HardFault handler example
 * @version  V1.00
 * @date     10. July 2017
 ******************************************************************************/
/*
 * Copyright (c) 2017 ARM Limited. All rights reserved.
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
 */

#include "RTE_Components.h"  // Component selection
#include CMSIS_device_header // Include device header file from
#include "stdio.h"           // include stdio library for printf output
#include "app_cfg.h"

extern void sendTraceSerial(const char *format, ...);

#define printf sendTraceSerial

#ifdef FAULT_REPORT_MODE_EN

void HardFault_Handler_C(unsigned long *svc_args, unsigned int lr_value);

// HardFault handler wrapper in assembly language.
// It extracts the location of stack frame and passes it to the handler written
// in C as a pointer. We also extract the LR value as second parameter.
__asm void HardFault_Handler(void)
{
  TST LR, #4 ITE EQ MRSEQ R0, MSP MRSNE R0, PSP MOV R1, LR B __cpp(HardFault_Handler_C)
}

// HardFault handler in C, with stack frame location and LR value extracted
// from the assembly wrapper as input parameters
void HardFault_Handler_C(unsigned long *hardfault_args, unsigned int lr_value)
{
  unsigned long stacked_r0;
  unsigned long stacked_r1;
  unsigned long stacked_r2;
  unsigned long stacked_r3;
  unsigned long stacked_r12;
  unsigned long stacked_lr;
  unsigned long stacked_pc;
  unsigned long stacked_psr;
  unsigned long cfsr;
  unsigned long bus_fault_address;
  unsigned long memmanage_fault_address;

  bus_fault_address = SCB->BFAR;
  memmanage_fault_address = SCB->MMFAR;
  cfsr = SCB->CFSR;

  stacked_r0 = ((unsigned long)hardfault_args[0]);
  stacked_r1 = ((unsigned long)hardfault_args[1]);
  stacked_r2 = ((unsigned long)hardfault_args[2]);
  stacked_r3 = ((unsigned long)hardfault_args[3]);
  stacked_r12 = ((unsigned long)hardfault_args[4]);
  stacked_lr = ((unsigned long)hardfault_args[5]);
  stacked_pc = ((unsigned long)hardfault_args[6]);
  stacked_psr = ((unsigned long)hardfault_args[7]);

  printf("[HardFault]\n");
  printf("- Stack frame:\n");
  printf(" R0  = %x\n", stacked_r0);
  printf(" R1  = %x\n", stacked_r1);
  printf(" R2  = %x\n", stacked_r2);
  printf(" R3  = %x\n", stacked_r3);
  printf(" R12 = %x\n", stacked_r12);
  printf(" LR  = %x\n", stacked_lr);
  printf(" PC  = %x\n", stacked_pc);
  printf(" PSR = %x\n", stacked_psr);
  printf("- FSR/FAR:\n");
  printf(" CFSR = %x\n", cfsr);
  printf(" HFSR = %x\n", SCB->HFSR);
  printf(" DFSR = %x\n", SCB->DFSR);
  printf(" AFSR = %x\n", SCB->AFSR);
  if (cfsr & 0x0080)
    printf(" MMFAR = %x\n", memmanage_fault_address);
  if (cfsr & 0x8000)
    printf(" BFAR = %x\n", bus_fault_address);
  printf("- Misc\n");
  printf(" LR/EXC_RETURN= %x\n", lr_value);

  while (1)
    ; // endless loop
}
#endif
