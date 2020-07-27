/**
  ******************************************************************************
  * @file    system.h
  * @author  Mahdad Ghasemian
  * @version V0.0.0
  * @date    07-Apil-2017
  * @brief   
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __SYSTEM_H
#define __SYSTEM_H

#ifdef __cplusplus
extern "C"
{
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f10x.h" // Device header
#include "sim808_driver.h"

  /** @defgroup SYSTEM 
  * @{
  */

  /** @defgroup SYSTEM_Exported_Macros
  * @{
  */

#define SETUP_SYSTEM_EEPROM_START 0
#define SETUP_SYSTEM_EEPROM_END (SETUP_SYSTEM_EEPROM_START + (sizeof(setup_t)))
#define RECORD_MONITOR_EEPROM_START (SETUP_SYSTEM_EEPROM_END)
#define RECORD_MONITOR_EEPROM_END (RECORD_MONITOR_EEPROM_START + (sizeof(record_monitor_t)))

  /**
  * @}
  */

  /** @defgroup SYSTEM_Exported_Types
  * @{
  */

  /** \brief  Setup setting structure.
 */
  typedef struct
  {
    uint32_t Flag;
    uint32_t SerialNumber;
    uint8_t ServerIP[SIM808_IP_MAX_SIZE];
    uint8_t ServerPort[SIM808_PORT_MAX_SIZE];
    uint8_t adminPhoneNumber[SIM808_PHONE_NUMBER_MAX_SIZE];
    uint8_t userPhoneNumber[USER_MAX_COUNT][SIM808_PHONE_NUMBER_MAX_SIZE];
    uint8_t AlarmStatusFlag;
    uint8_t AlarmModeSms;
    uint8_t AlarmModeCall;
    uint8_t TSLimit;
    uint16_t MaximumSpeed;
    uint8_t ServerModeActive;
    uint8_t SaveAnyTime;
    uint8_t SaveRecordIntervalFactor;
    uint8_t rezerve[23];
  } setup_t;

  /** \brief  Record monitor index structure.
 */
  typedef struct
  {
    uint32_t ReadIndex;
    uint32_t WriteIndex;
  } record_monitor_t;

  /**
  * @}
  */

  /** @defgroup SYSTEM_Exported_Variables
  * @{
  */

  extern setup_t SystemSetup;
  extern record_monitor_t RecordMonitor;

  /**
  * @}
  */

  /** @defgroup SYSTEM_Exported_Functions
  * @{
  */

  void parameterLoad(void);
  void parameterWrite(void);
  int8_t reocrdMonitorIndexLoad(void);
  int8_t reocrdMonitorIndexWrite(void);

  void setParameterToVirgin(void);
  void setParameterToFactory(void);
  void setParameterToFlatState(uint32_t SerialNumber);

#ifdef __cplusplus
}
#endif

#endif /* __SYSTEM_H */

/**
  * @}
  */

/**
  * @}
  */

/*********************************END OF FILE****************************/
