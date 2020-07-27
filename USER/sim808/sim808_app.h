/**
  ******************************************************************************
  * @file    sim808_app.h
  * @author  Mahdad Ghasemian
  * @version V0.0.0
  * @date    25-March-2018
  * @brief   
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __SIM808_APP_H
#define __SIM808_APP_H

#ifdef __cplusplus
extern "C"
{
#endif

/* Includes ------------------------------------------------------------------*/
#include "cmsis_os2.h" // ::CMSIS:RTOS2
#include "rtx_os.h"    // ARM::CMSIS:RTOS2:Keil RTX5

  /** @defgroup SIM808_APP
  * @{
  */

  /** @defgroup SIM808_APP_Exported_Macros
  * @{
  */

#define APP_GSM_INIT_OK_FLAG 0x00000001
#define APP_GSM_PSD_OK_FLAG 0x00000002
#define APP_GSM_TCP_OK_FLAG 0x00000004
#define APP_GSM_BEARER_OK_FLAG 0x00000008
#define APP_SPI_FLASH_READY_FLAG 0x00000010
#define APP_FLASH_INIT_RECORD_FLAG 0x00000020
#define APP_ACC_INPUT_INT_FLAG 0x00000040
#define APP_DOOR_INPUT_INT_FLAG 0x00000080
#define APP_POWER_INPUT_INT_FLAG 0x00000100
#define APP_RESERVE_1_FLAG 0x00000200
#define APP_RESERVE_2_FLAG 0x00000400
#define APP_RESERVE_3_FLAG 0x00000800
#define APP_RED_STATUS_ACC_FLAG 0x00001000
#define APP_RED_STATUS_POWER_FLAG 0x00002000
#define APP_RED_STATUS_DOOR_FLAG 0x00004000
#define APP_RED_STATUS_MOVING_FLAG 0x00008000
#define APP_MAXIMUM_SPEED_FLAG 0x00010000
#define APP_RED_STATUS_CALL_FLAG 0x00020000

  /**
  * @}
  */

  /** @defgroup SIM808_APP_Exported_Types
  * @{
  */

  /**
  * @}
  */

  /** @defgroup SIM808_APP_Exported_Variables
  * @{
  */

  extern osEventFlagsId_t app_eventflag_id;

  /**
  * @}
  */

  /** @defgroup SIM808_APP_Exported_Functions
  * @{
  */

  extern void AppGsmPSDClear(void);
  extern void AppGsmTCPClear(void);

  /**
  * @}
  */

  /**
  * @}
  */

#ifdef __cplusplus
}
#endif

#endif /* __SIM808_APP_H */

/*********************************END OF FILE****************************/
