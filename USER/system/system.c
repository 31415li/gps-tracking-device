/**
  ******************************************************************************
  * @file    system.c
  * @author  Mahdad Ghasemian
  * @version V0.0.0
  * @date    07-Apil-2017
  * @brief   
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "system.h"
#include "eeprom.h"
#include "app_cfg.h"
#include <stdio.h>
#include "cmsis_os2.h" // ::CMSIS:RTOS2
#include "rtx_os.h"    // ARM::CMSIS:RTOS2:Keil RTX5
#include "string.h"

/** @defgroup SYSTEM 
  * @brief 
  * @{
  */

/** @defgroup SYSTEM_Private_Macros
  * @{
  */

#define PARAMETER_FLASH_PAGE_ADDRESS ((uint32_t)0x0801F800) //page 126

/**
  * @}
  */

/** @defgroup SYSTEM_Private_Variables
  * @{
  */

setup_t SystemSetup;

/**
  * @}
  */

/** @defgroup SYSTEM_Private_Functions
  * @{
  */

/**
  * @brief  
  * @param  ندارد
  * @return 
  */
static uint8_t Flash_Parameter_Flash_Erase(void)
{
  FLASH_Status status;

  status = FLASH_ErasePage(PARAMETER_FLASH_PAGE_ADDRESS);

  if (status == FLASH_COMPLETE)
    return 1;

  return 0;
}

/**
  * @brief  
  * @param  ندارد
  * @return 
  */
static void Flash_Parameter_Flash_To_Ram(void)
{
  uint32_t Address = PARAMETER_FLASH_PAGE_ADDRESS;
  uint32_t i = 0, j = 0;
  uint32_t len = sizeof(SystemSetup);
  uint16_t temp;
  uint8_t *data = (uint8_t *)&SystemSetup;

  for (i = 0; i < len; i += 2, Address += 2)
  {
    temp = (*(__IO uint16_t *)Address);
    data[i] = temp;
    data[i + 1] = temp >> 8;
  }
}

/**
  * @brief  
  * @param  ندارد
  * @return 
  */
static uint8_t Flash_Parameter_Ram_To_Flash(void)
{
  FLASH_Status status;
  uint32_t BaseAddress = PARAMETER_FLASH_PAGE_ADDRESS;
  uint32_t i = 0;
  uint32_t len = sizeof(SystemSetup);

  for (i = 0; i < len; i += 4)
  {
    status = FLASH_ProgramWord(BaseAddress + i, (*((uint32_t *)((uint8_t *)&SystemSetup + i))));
    if (status != FLASH_COMPLETE)
      return 1;
  }

  return 0;
}

/**
  * @brief  
  * @param  ندارد
  * @return 
  */
static void Flash_Parameter_Read(void)
{
  Flash_Parameter_Flash_To_Ram();
}

/**
  * @brief  
  * @param  ندارد
  * @return 
  */
static void Flash_Parameter_Write(void)
{
  Flash_Parameter_Flash_Erase();
  Flash_Parameter_Ram_To_Flash();
}

/**
  * @brief  EEPROM Write with mutex
  * @param  
  * @return 
  */
static int8_t writeEeprom(uint16_t address, uint8_t *data, uint16_t length)
{
  if (eepromWrite(address, data, length) == -1)
    return -1;

  return 0;
}

/**
  * @brief  EEPROM Read with mutex
  * @param  
  * @return 
  */
static int8_t readEeprom(uint16_t address, uint8_t *data, uint16_t length)
{
  if (eepromRead(address, data, length) == -1)
    return -1;

  return 0;
}

/**
  * @brief  
  * @param  ندارد
  * @return 
  */
void parameterLoad(void)
{
  osDelay(20);
  /* خواندن تنظيمات سيستم */
  eepromLock();
#if DATA_PARAMETER_INTERFACE_TYPE == DATA_PARAMETER_INTERFACE_INTERNAL_FLASH
  Flash_Parameter_Read();
#elif DATA_PARAMETER_INTERFACE_TYPE == DATA_PARAMETER_INTERFACE_I2C_EEPROM
  readEeprom(SETUP_SYSTEM_EEPROM_START, (uint8_t *)&SystemSetup, sizeof(setup_t));
#endif
  eepromUnLock();
}

/**
  * @brief  
  * @param  ندارد
  * @return 
  */
void parameterWrite(void)
{
  osDelay(20);
  /* نوشتن تنظيمات سيستم */
  eepromLock();
#if DATA_PARAMETER_INTERFACE_TYPE == DATA_PARAMETER_INTERFACE_INTERNAL_FLASH
  Flash_Parameter_Write();
#elif DATA_PARAMETER_INTERFACE_TYPE == DATA_PARAMETER_INTERFACE_I2C_EEPROM
  writeEeprom(SETUP_SYSTEM_EEPROM_START, (uint8_t *)&SystemSetup, sizeof(setup_t));
#endif
  eepromUnLock();
}

/**
  * @brief  
  * @param  ندارد
  * @return 
  */
int8_t reocrdMonitorIndexLoad(void)
{
  int8_t status = 0;

  osDelay(20);
  /* خواندن اطلاعات رکوردها */
  eepromLock();
  if (readEeprom(RECORD_MONITOR_EEPROM_START, (uint8_t *)&RecordMonitor, sizeof(record_monitor_t)) == -1)
  {
    osDelay(10);
    if (readEeprom(RECORD_MONITOR_EEPROM_START, (uint8_t *)&RecordMonitor, sizeof(record_monitor_t)) == -1)
      status = -1;
  }
  eepromUnLock();

  return status;
}

/**
  * @brief  
  * @param  ندارد
  * @return 
  */
int8_t reocrdMonitorIndexWrite(void)
{
  int8_t status = 0;

  osDelay(20);
  /* نوشتن اطلاعات رکوردها */
  eepromLock();
  if (writeEeprom(RECORD_MONITOR_EEPROM_START, (uint8_t *)&RecordMonitor, sizeof(record_monitor_t)) == -1)
  {
    osDelay(10);
    if (writeEeprom(RECORD_MONITOR_EEPROM_START, (uint8_t *)&RecordMonitor, sizeof(record_monitor_t)) == -1)
      status = -1;
  }
  eepromUnLock();

  return status;
}

/**
  * @brief  
  * @param  ندارد
  * @return ندراد
  */
void setParameterToVirgin(void)
{
  uint8_t i;

  SystemSetup.Flag = 0;
  SystemSetup.SerialNumber = 0;
  memset(&SystemSetup.ServerIP[0], 0, SIM808_IP_MAX_SIZE);
  memset(&SystemSetup.ServerPort[0], 0, SIM808_PORT_MAX_SIZE);
  memset(&SystemSetup.adminPhoneNumber[0], 0, SIM808_PHONE_NUMBER_MAX_SIZE);
  for (i = 0; i < USER_MAX_COUNT; i++)
    memset(&SystemSetup.userPhoneNumber[i], 0, SIM808_PHONE_NUMBER_MAX_SIZE);
}

/**
  * @brief  
  * @param  ندارد
  * @return ندراد
  */
void setParameterToFactory(void)
{
  uint8_t i;

  SystemSetup.Flag = 0;
  //SystemSetup.SerialNumber
  memset(&SystemSetup.ServerIP[0], 0, SIM808_IP_MAX_SIZE);
  memset(&SystemSetup.ServerPort[0], 0, SIM808_PORT_MAX_SIZE);
  memset(&SystemSetup.adminPhoneNumber[0], 0, SIM808_PHONE_NUMBER_MAX_SIZE);
  for (i = 0; i < USER_MAX_COUNT; i++)
    memset(&SystemSetup.userPhoneNumber[i], 0, SIM808_PHONE_NUMBER_MAX_SIZE);
  SystemSetup.AlarmStatusFlag = 0;
  SystemSetup.AlarmModeSms = 0;
  SystemSetup.AlarmModeCall = 0;
  SystemSetup.TSLimit = 10;
  SystemSetup.MaximumSpeed = 220;
  SystemSetup.ServerModeActive = 1;
  SystemSetup.SaveAnyTime = 1;
}

/**
  * @brief  
  * @param  ندارد
  * @return ندراد
  */
void setParameterToFlatState(uint32_t SerialNumber)
{
  uint8_t i;

  SystemSetup.Flag = 0;
  SystemSetup.SerialNumber = SerialNumber;
  strcpy((char *)&SystemSetup.ServerIP[0], "185.105.184.5");
  strcpy((char *)&SystemSetup.ServerPort[0], "25080");
  strcpy((char *)&SystemSetup.adminPhoneNumber[0], "09351627357");
  for (i = 0; i < USER_MAX_COUNT; i++)
    memset(&SystemSetup.userPhoneNumber[i], 0, SIM808_PHONE_NUMBER_MAX_SIZE);
  SystemSetup.AlarmStatusFlag = 0;
  SystemSetup.AlarmModeSms = 0;
  SystemSetup.AlarmModeCall = 0;
  SystemSetup.TSLimit = 10;
  SystemSetup.MaximumSpeed = 220;
  SystemSetup.ServerModeActive = 1;
  SystemSetup.SaveAnyTime = 1;
}

/**
  * @}
  */

/**
  * @}
  */

/*********************************END OF FILE****************************/
