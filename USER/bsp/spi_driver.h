/**
  ******************************************************************************
  * @file    spi_driver.h
  * @author  Mahdad Ghasemian
  * @version V0.0.0
  * @date    08-April-2018
  * @brief   
  ******************************************************************************
  */
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __SPI_DRIVER_H
#define __SPI_DRIVER_H

#ifdef __cplusplus
extern "C"
{
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f10x.h" // Device header

  /** @defgroup SPI_DRIVER 
  * @{
  */

  /** @defgroup SPI_DRIVER_Exported_Macros
  * @{
  */

  //-------- <<< Use Configuration Wizard in Context Menu >>> -----------------

  /**
  * @}
  */

  /** @defgroup SPI_DRIVER_Exported_Types
  * @{
  */

  /**
  * @}
  */

  /** @defgroup SPI_DRIVER_Exported_Variables
  * @{
  */

  /**
  * @}
  */

  /** @defgroup SPI_DRIVER_Exported_Functions
  * @{
  */

  uint8_t spiSendReciveByte(uint8_t data);
  void spiLock(void);
  void spiUnLock(void);

  /**
  * @}
  */

#ifdef __cplusplus
}
#endif

/**
  * @}
  */

#endif /* __SPI_DRIVER_H */

/*********************************END OF FILE****************************/
