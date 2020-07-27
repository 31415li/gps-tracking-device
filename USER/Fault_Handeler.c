/**
  ******************************************************************************
  * @file    Fault_Handeler.c
  * @author  Mahdad Ghasemian
  * @version V0.0.0
  * @date    16-December-2015
  * @brief   
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "Fault_Handeler.h"
#include <stdio.h>
#include "app_cfg.h"
#include "stm32f10x.h" // Device header

/** @defgroup FAULT_HANDELER
  * @brief 
  * @{
  */

extern void sendTraceSerial(const char *format, ...);
#define printErrorMsg sendTraceSerial

/** @defgroup FAULT_HANDELER_Private_Macros
  * @{
  */

/**
  * @}
  */

/** @defgroup FAULT_HANDELER_Private_Variables
  * @{
  */

/**
  * @}
  */

/** @defgroup FAULT_HANDELER_Private_Types
  * @{
  */

#ifdef DEBUG_MODE_EN
enum
{
  r0,
  r1,
  r2,
  r3,
  r12,
  lr,
  pc,
  psr
};
#endif

/**
  * @}
  */

/** @defgroup FAULT_HANDELER_Private_Functions
  * @{
  */

#ifdef DEBUG_MODE_EN

/**
  * @brief  مقادير داخل آرايه که حاوي محتويات رجيستر هاي اصلي مي باشد را براي چاپ آماده مي کند 
  * @param  stack : آرايه حاوي مقادير
  * @return ندراد
  */
static void stackDump(uint32_t stack[])
{
  static char msg[80];
  sprintf(msg, "r0  = 0x%08x", stack[r0]);
  printErrorMsg(msg);
  sprintf(msg, "r1  = 0x%08x", stack[r1]);
  printErrorMsg(msg);
  sprintf(msg, "r2  = 0x%08x", stack[r2]);
  printErrorMsg(msg);
  sprintf(msg, "r3  = 0x%08x", stack[r3]);
  printErrorMsg(msg);
  sprintf(msg, "r12 = 0x%08x", stack[r12]);
  printErrorMsg(msg);
  sprintf(msg, "lr  = 0x%08x", stack[lr]);
  printErrorMsg(msg);
  sprintf(msg, "pc  = 0x%08x", stack[pc]);
  printErrorMsg(msg);
  sprintf(msg, "psr = 0x%08x", stack[psr]);
  printErrorMsg(msg);

  //	sprintf(msg, "Wifi1 = %d", logSystem.WifiToTCP1);
  //  printErrorMsg(msg);
  //	sprintf(msg, "Wifi2 = %d", logSystem.WifiToTCP2);
  //  printErrorMsg(msg);
  ////	sprintf(msg, "Wifi3 = %d", logSystem.WifiToTCP3);
  ////  printErrorMsg(msg);
  ////	sprintf(msg, "Wifi4 = %d", logSystem.WifiToTCP4);
  ////  printErrorMsg(msg);
  ////	sprintf(msg, "Wifi5 = %d", logSystem.WifiToTCP5);
  ////  printErrorMsg(msg);
  ////	sprintf(msg, "Wifi6 = %d", logSystem.WifiToTCP6);
  ////  printErrorMsg(msg);
  ////	sprintf(msg, "Wifi7 = %d", logSystem.WifiToTCP7);
  ////  printErrorMsg(msg);
  ////	sprintf(msg, "Wifi8 = %d", logSystem.WifiToTCP8);
  ////  printErrorMsg(msg);
  //	sprintf(msg, "WifiS1 = %d", logSystem.WifiSend1);
  //  printErrorMsg(msg);
  //	sprintf(msg, "WifiS2 = %d", logSystem.WifiSend2);
  //  printErrorMsg(msg);
  //	sprintf(msg, "WifiS3 = %d", logSystem.WifiSend3);
  //  printErrorMsg(msg);
  //	sprintf(msg, "Gsm1 = %d", logSystem.GSMToTCP1);
  //  printErrorMsg(msg);
  //	sprintf(msg, "TCP1 = %d", logSystem.TCPReceive1);
  //  printErrorMsg(msg);
  //	sprintf(msg, "TCP2 = %d", logSystem.TCPReceive2);
  //  printErrorMsg(msg);
  //	sprintf(msg, "TCP3 = %d", logSystem.TCPReceive3);
  //  printErrorMsg(msg);
  //	sprintf(msg, "MNG1 = %d", logSystem.MNGReceive1);
  //  printErrorMsg(msg);
}

/**
  * @brief  
  * @param  ندارد
  * @return ندراد
  */
void Hard_Fault_Handler(uint32_t stack[])
{
  static char msg[80];

  printErrorMsg("In Hard Fault Handler");
  sprintf(msg, "SCB->HFSR = 0x%08x", SCB->HFSR);
  printErrorMsg(msg);

  if ((SCB->HFSR & 0x00000002))
    printErrorMsg("VECTTBL");
  if ((SCB->HFSR & 0x40000000))
    printErrorMsg("Forced Hard Fault");

  if ((SCB->HFSR & 0x40000000) != 0)
  {
    sprintf(msg, "SCB->CFSR = 0x%08x", SCB->CFSR);
    printErrorMsg(msg);

    if ((SCB->CFSR & 0x00200000))
      printErrorMsg("Divide by zero (PC)");
  }

  if ((SCB->CFSR & 0x00008000))
  {
    printErrorMsg("Memory address Bus Fualt:");
    sprintf(msg, "SCB->BFAR = 0x%08x", SCB->BFAR);
    printErrorMsg(msg);
  }

  if ((SCB->CFSR & 0x00000080))
  {
    printErrorMsg("Memory address Mem Fualt:");
    sprintf(msg, "SCB->MMFAR = 0x%08x", SCB->MMFAR);
    printErrorMsg(msg);
  }

  stackDump(stack);
  // __ASM volatile("BKPT #01");
  while (1)
    ;
}

/**
  * @brief  
  * @param  ندارد
  * @return ندراد
  */
void Bus_Fault_Handler(uint32_t stack[])
{
  static char msg[80];

  printErrorMsg("In Bus Fault Handler");
  sprintf(msg, "SCB->CFSR = 0x%08x", SCB->CFSR);
  printErrorMsg(msg);
  if ((SCB->CFSR & 0x0000FF00) != 0)
  {
    uint32_t CFSRValue = SCB->CFSR;
    if ((CFSRValue & 0x00000100))
      printErrorMsg("IBUSERR");
    if ((CFSRValue & 0x00000200))
      printErrorMsg("PRECISEERR");
    if ((CFSRValue & 0x00000400))
      printErrorMsg("IMPRECISERR");
    if ((CFSRValue & 0x00000800))
      printErrorMsg("UNSTKERR");
    if ((CFSRValue & 0x00001000))
      printErrorMsg("STKERR");
    if ((CFSRValue & 0x00008000))
      printErrorMsg("BFARVALID");

    if ((SCB->CFSR & 0x00000200))
    {
      printErrorMsg("Memory address Bus Fualt:");
      sprintf(msg, "SCB->BFAR = 0x%08x", SCB->BFAR);
      printErrorMsg(msg);
    }
  }

  stackDump(stack);
  // __ASM volatile("BKPT #01");
  while (1)
    ;
}

/**
  * @brief  
  * @param  ندارد
  * @return ندراد
  */
void MemManage_Fault_Handler(uint32_t stack[])
{
  static char msg[80];

  printErrorMsg("In MemManage Fault Handler");
  sprintf(msg, "SCB->CFSR = 0x%08x", SCB->CFSR);
  printErrorMsg(msg);
  if ((SCB->CFSR & 0x000000FF) != 0)
  {
    uint32_t CFSRValue = SCB->CFSR;
    if ((CFSRValue & 0x00000001))
      printErrorMsg("IACCVIOL");
    if ((CFSRValue & 0x00000002))
      printErrorMsg("DACCVIOL");
    if ((CFSRValue & 0x00000008))
      printErrorMsg("MUNSTKERR");
    if ((CFSRValue & 0x00000010))
      printErrorMsg("MSTKERR");
    if ((CFSRValue & 0x00000080))
      printErrorMsg("MMFARVALID");
  }

  stackDump(stack);
  // __ASM volatile("BKPT #01");
  while (1)
    ;
}

/**
  * @brief  
  * @param  ندارد
  * @return ندراد
  */
void Usage_Fault_Handler(uint32_t stack[])
{
  static char msg[80];

  printErrorMsg("In Usage Fault Handler");
  sprintf(msg, "SCB->CFSR = 0x%08x", SCB->CFSR);
  printErrorMsg(msg);

  if ((SCB->CFSR & 0xFFFF0000) != 0)
  {
    uint32_t CFSRValue = SCB->CFSR;
    if ((CFSRValue & 0x00010000))
      printErrorMsg("Undefined instruction (PC)");
    if ((CFSRValue & 0x00020000))
      printErrorMsg("Illegal use of the EPSR (PC)");
    if ((CFSRValue & 0x00040000))
      printErrorMsg("Illegal EXC_RETURN (PC)");
    if ((CFSRValue & 0x00080000))
      printErrorMsg("Proc not support coprocessor");
    if ((CFSRValue & 0x01000000))
      printErrorMsg("Unalign accesses");
    if ((CFSRValue & 0x02000000))
      printErrorMsg("Divide by zero (PC)");
  }

  stackDump(stack);
  // __ASM volatile("BKPT #01");
  while (1)
    ;
}

/**
  * @brief  
  * @param  ندارد
  * @return ندراد
  */
/*
void NMI_Fault_Handler(uint32_t stack[])
{ 
	static char msg[80];
	
	clearErrorMsg();
	printErrorMsg("In NMI Fault Handler");

	// __ASM volatile("BKPT #01"); 
	while(1);
}
*/
/**
  * @brief  
  * @param  ندارد
  * @return ندراد
  */
#if defined(__CC_ARM)
__asm void HardFault_Handler(void)
{
  TST lr, #4 ITE EQ MRSEQ r0, MSP MRSNE r0, PSP B __cpp(Hard_Fault_Handler)
}
#elif defined(__ICCARM__)
void HardFault_Handler(void)
{
  __asm("TST lr, #4");
  __asm("ITE EQ");
  __asm("MRSEQ r0, MSP");
  __asm("MRSNE r0, PSP");
  __asm("B Hard_Fault_Handler");
}
#else
#warning Not supported compiler type
#endif

/**
  * @brief  
  * @param  ندارد
  * @return ندراد
  */
#if defined(__CC_ARM)
__asm void BusFault_Handler(void)
{
  TST lr, #4 ITE EQ MRSEQ r0, MSP MRSNE r0, PSP B __cpp(Bus_Fault_Handler)
}
#else
#warning Not supported compiler type
#endif

/**
  * @brief  
  * @param  ندارد
  * @return ندراد
  */
#if defined(__CC_ARM)
__asm void MemManage_Handler(void)
{
  TST lr, #4 ITE EQ MRSEQ r0, MSP MRSNE r0, PSP B __cpp(MemManage_Fault_Handler)
}
#else
#warning Not supported compiler type
#endif

/**
  * @brief  
  * @param  ندارد
  * @return ندراد
  */
#if defined(__CC_ARM)
__asm void UsageFault_Handler(void)
{
  TST lr, #4 ITE EQ MRSEQ r0, MSP MRSNE r0, PSP B __cpp(Usage_Fault_Handler)
}
#else
#warning Not supported compiler type
#endif

/**
  * @brief  
  * @param  ندارد
  * @return ندراد
  */
/*
#if defined(__CC_ARM)
__asm void NMI_Handler(void)
{
   TST lr, #4
   ITE EQ
   MRSEQ r0, MSP
   MRSNE r0, PSP
   B __cpp(NMI_Fault_Handler)
}
#else
  #warning Not supported compiler type
#endif
*/

#else

/*
void NMI_Handler(void)
{
	//ResetSystem ();
}

#ifndef DEBUG_MODE_EN
void HardFault_Handler(void)
{
	ResetSystem ();
}

void MemManage_Handler(void)
{
	ResetSystem ();
}

void BusFault_Handler(void)
{
	ResetSystem ();
}

void UsageFault_Handler(void)
{
	ResetSystem ();
}
#endif
*/
#endif

/**
  * @}
  */

/**
  * @}
  */

/*********************************END OF FILE****************************/
