/**
  ******************************************************************************
  * @file    Project/STM32F10x_StdPeriph_Template/stm32f10x_it.c 
  * @author  MCD Application Team
  * @version V3.5.0
  * @date    08-April-2011
  * @brief   Main Interrupt Service Routines.
  *          This file provides template for all exceptions handler and 
  *          peripherals interrupt service routine.
  ******************************************************************************
  * @attention
  *
  * THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
  * WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE
  * TIME. AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY
  * DIRECT, INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
  * FROM THE CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE
  * CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
  *
  * <h2><center>&copy; COPYRIGHT 2011 STMicroelectronics</center></h2>
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "stm32f10x_it.h"
#include "app_cfg.h"
#include "sim808_app.h"

extern void Sim808Interrupt(void);
extern void Sim808CheckReceiveByte(uint8_t byte);

#define UART3_Int_Recive_HANDLER(byte) Sim808CheckReceiveByte(byte)

/** @addtogroup STM32F10x_StdPeriph_Template
  * @{
  */

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/******************************************************************************/
/*            Cortex-M3 Processor Exceptions Handlers                         */
/******************************************************************************/

/**
  * @brief  This function handles NMI exception.
  * @param  None
  * @retval None
  */
void NMI_Handler(void)
{
}

/**
  * @brief  This function handles Hard Fault exception.
  * @param  None
  * @retval None
  */
#ifndef FAULT_REPORT_MODE_EN
void HardFault_Handler(void)
{
  /* Go to infinite loop when Hard Fault exception occurs */
  while (1)
  {
    NVIC_SystemReset();
  }
}
#endif

/**
  * @brief  This function handles Memory Manage exception.
  * @param  None
  * @retval None
  */
void MemManage_Handler(void)
{
  /* Go to infinite loop when Memory Manage exception occurs */
  while (1)
  {
    NVIC_SystemReset();
  }
}

/**
  * @brief  This function handles Bus Fault exception.
  * @param  None
  * @retval None
  */
void BusFault_Handler(void)
{
  /* Go to infinite loop when Bus Fault exception occurs */
  while (1)
  {
    NVIC_SystemReset();
  }
}

/**
  * @brief  This function handles Usage Fault exception.
  * @param  None
  * @retval None
  */
void UsageFault_Handler(void)
{
  /* Go to infinite loop when Usage Fault exception occurs */
  while (1)
  {
    NVIC_SystemReset();
  }
}

/**
  * @brief  This function handles Debug Monitor exception.
  * @param  None
  * @retval None
  */
void DebugMon_Handler(void)
{
}

/******************************************************************************/
/*                 STM32F10x Peripherals Interrupt Handlers                   */
/*  Add here the Interrupt Handler for the used peripheral(s) (PPP), for the  */
/*  available peripheral interrupt handler's name please refer to the startup */
/*  file (startup_stm32f10x_xx.s).                                            */
/******************************************************************************/

/**
  * @brief  This function handles PPP interrupt request.
  * @param  None
  * @retval None
  */
/*void PPP_IRQHandler(void)
{
}*/

/**
  * @brief  
  * @param  
  * @return ندراد
  */
void EXTI0_IRQHandler(void)
{
  /* بيت فلگ رخ دادن وقفه را پاک ميکند */
  EXTI_ClearITPendingBit(EXTI_Line0);

  osEventFlagsSet(app_eventflag_id, APP_POWER_INPUT_INT_FLAG);
}

/**
  * @brief  
  * @param  
  * @return ندراد
  */
void EXTI1_IRQHandler(void)
{
  /* بيت فلگ رخ دادن وقفه را پاک ميکند */
  EXTI_ClearITPendingBit(EXTI_Line1);

  osEventFlagsSet(app_eventflag_id, APP_ACC_INPUT_INT_FLAG);
}

/**
  * @brief  
  * @param  
  * @return ندراد
  */
void EXTI9_5_IRQHandler(void)
{
  /* بيت فلگ رخ دادن وقفه را پاک ميکند */
  EXTI_ClearITPendingBit(EXTI_Line8);

  Sim808Interrupt();
}

/**
  * @brief  
  * @param  
  * @return ندراد
  */
void EXTI15_10_IRQHandler(void)
{
  /* بيت فلگ رخ دادن وقفه را پاک ميکند */
  EXTI_ClearITPendingBit(EXTI_Line14);

  osEventFlagsSet(app_eventflag_id, APP_DOOR_INPUT_INT_FLAG);
}

/**
  * @brief 
  * @param
  * @return
  */
void USART3_IRQHandler(void)
{
  //FlagStatus tc_status;
  FlagStatus rxne_status;
  uint8_t BSP_SerRxData;

  rxne_status = USART_GetFlagStatus(USART3, USART_FLAG_RXNE);
  if (rxne_status == SET)
  {
    BSP_SerRxData = USART_ReceiveData(USART3) & 0xFF; /* Read one byte from the receive data register.      */
    USART_ClearITPendingBit(USART3, USART_IT_RXNE);   /* Clear the USART1 receive interrupt.                */
    UART3_Int_Recive_HANDLER(BSP_SerRxData);
  }

  //tc_status = USART_GetFlagStatus(USART3, USART_FLAG_TC);
  //if (tc_status == SET) {
  //		USART_ITConfig(USART3, USART_IT_TC, DISABLE);
  //		USART_ClearITPendingBit(USART3, USART_IT_TC);
  //BSP_OS_SemPost(&BSP_Ser1TxWait);
  //}
}

/**
  * @}
  */

/******************* (C) COPYRIGHT 2011 STMicroelectronics *****END OF FILE****/
