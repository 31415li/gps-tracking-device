/**
  ******************************************************************************
  * @file    bsp.h
  * @author  Mahdad Ghasemian
  * @version V0.0.0
  * @date    3-March-2015
  * @brief   
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __BSP_H
#define __BSP_H

#ifdef __cplusplus
extern "C"
{
#endif

/* Includes ------------------------------------------------------------------*/
#include <stdint.h>
#include "stm32f10x.h" // Device header

  /** @defgroup BSP 
  * @{
  */

  /** @defgroup BSP_Exported_Macros
  * @{
  */

  /**
  * @}
  */

  /** @defgroup BSP_Exported_Types
  * @{
  */

  /**
  * @}
  */

  /** @defgroup BSP_Exported_Variables
  * @{
  */

  extern uint8_t resetSource;

  /**
  * @}
  */

  /** @defgroup BSP_Exported_Functions
  * @{
  */

  void bspInit(void);
  void sendTraceSerial(const char *format, ...);
  void sendTraceSerialBinaray(uint8_t *ptr, uint16_t len);
  void bspSim808Reset(void);
  void bspSim808PowerOn(void);
  FunctionalState bspSim808ReadStatus(void);
  void bspSim808SendBinaray(uint8_t *ptr, uint16_t len);
  void bspLedGreenOFF(void);
  void bspLedGreenON(void);
  void bspLedGreenToggle(void);
  uint32_t timeToSecond(uint16_t year,
                        uint8_t month,
                        uint8_t day,
                        uint8_t hour,
                        uint8_t min,
                        uint8_t sec);
  uint8_t bspAccReadStatus(uint8_t *state);
  uint8_t bspPowerReadStatus(uint8_t *state);
  uint8_t bspDoorReadStatus(uint8_t *state);

  char *strtokSingle(char *str, const char *delims);

  /**
  * @}
  */

#ifdef __cplusplus
}
#endif

/**
  * @}
  */

#endif /* __BSP_H */

/*********************************END OF FILE****************************/
