/**
  ******************************************************************************
  * @file    eeprom.h
  * @author  Mahdad Ghasemian
  * @version V0.0.0
  * @date    07-April-2018
  * @brief   
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __EEPROM_H
#define __EEPROM_H

#ifdef __cplusplus
extern "C"
{
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f10x.h" // Device header

/** @defgroup EEPROM 
  * @{
  */

/** @defgroup EEPROM_Exported_Macros
  * @{
  */

//-------- <<< Use Configuration Wizard in Context Menu >>> -----------------

// <o> EEPROM I2C Address <0x00-0xFE:0x02>
#define EEPROM_I2C_ADDRESS 0xA0
// <o> EEPROM I2C Speed (KHz) <100-400>
// <i> in KHz
#define EEPROM_I2C_SPEED 100 * 1000
// <o> EEPROM Size (Byte) <2=>2K <4=>4K <8=>8K <16=>16K <32=>32K <64=>64K
// <i> in Byte
#define EEPROM_I2C_SIZE 8 * 1024

#define EEPROM_I2C_SAME_WRITE_CYCLE_SIZE ((EEPROM_I2C_SIZE * 4) / 1024)

#define EEPROM_I2C_DELAY(x) osDelay(x)

  /**
  * @}
  */

  /** @defgroup EEPROM_Exported_Types
  * @{
  */

  /**
  * @}
  */

  /** @defgroup EEPROM_Exported_Variables
  * @{
  */

  /**
  * @}
  */

  /** @defgroup EEPROM_Exported_Functions
  * @{
  */

  void eepromInit(void);
  int16_t eepromWrite(uint16_t address, uint8_t *data, uint16_t length);
  int16_t eepromRead(uint16_t address, uint8_t *data, uint16_t length);

  void eepromLock(void);
  void eepromUnLock(void);

  /**
  * @}
  */

#ifdef __cplusplus
}
#endif

/**
  * @}
  */

#endif /* __EEPROM_H */

/*********************************END OF FILE****************************/
