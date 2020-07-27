/**
  ******************************************************************************
  * @file    input.c
  * @author  Mahdad Ghasemian
  * @version V0.0.1
  * @date    28-April-2018
  * @brief   
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "input.h"
#include "cmsis_os2.h" // ::CMSIS:RTOS2
#include "includes.h"
#include "bsp.h"
#include <stdbool.h>
#include <string.h>

/** @defgroup INPUT
  * @brief 
  * @{
  */

/** @defgroup INPUT_Private_Macros
  * @{
  */

/**
  * @}
  */

/** @defgroup INPUT_Private_Variables
  * @{
  */

INPUT_New_Event_t newAccChangeFlag = INPUT_READ_EVENT;
INPUT_New_Event_t newDoorChangeFlag = INPUT_READ_EVENT;

/**
  * @}
  */

/** @defgroup INPUT_Static_Functions
  * @{
  */

/**
  * @}
  */

/** @defgroup INPUT_Private_Functions
  * @{
  */

/**
  * @brief  
  * @param  ندراد
  * @return ندراد
  */
void AccInterrupt(void)
{
  newAccChangeFlag = INPUT_NEW_EVENT;
}

/**
  * @brief  
  * @param  ندراد
  * @return ندراد
  */
void DoorInterrupt(void)
{
  newDoorChangeFlag = INPUT_NEW_EVENT;
}

/**
  * @}
  */

/**
  * @}
  */

/*********************************END OF FILE****************************/
