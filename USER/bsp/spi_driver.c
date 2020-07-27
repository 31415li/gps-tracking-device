/**
  ******************************************************************************
  * @file    spi_driver.c
  * @author  Mahdad Ghasemian
  * @version V0.0.0
  * @date    08-April-2018
  * @brief   
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "spi_driver.h"
#include "rtx_os.h"    // ARM::CMSIS:RTOS2:Keil RTX5
#include "cmsis_os2.h" // ::CMSIS:RTOS2

/** @defgroup SPI_DRIVER 
  * @brief 
  * @{
  */

/** @defgroup SPI_DRIVER_Private_Macros
  * @{
  */

/**
  * @}
  */

/** @defgroup SPI_DRIVER_Private_Variables
  * @{
  */

extern osMutexId_t spi_mutex_id;

/**
  * @}
  */

/** @defgroup SPI_DRIVER_Private_Functions
  * @{
  */

/**
  * @brief  
	* @param 
  * @return 
  */
uint8_t spiSendReciveByte(uint8_t data)
{
  // Loop while DR register in not emplty
  while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET)
    ;

  // Send byte through the SPI1 peripheral
  SPI_I2S_SendData(SPI1, data);

  // Wait to receive a byte
  while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_RXNE) == RESET)
    ;

  // Return the byte read from the SPI bus
  return SPI_I2S_ReceiveData(SPI1);
}

/**
  * @brief  
  * @param  ندارد
  * @return ندراد
  */
void spiLock(void)
{
  osStatus_t status;

  if (spi_mutex_id != NULL)
  {
    status = osMutexAcquire(spi_mutex_id, osWaitForever);
    if (status != osOK)
    {
    }
  }
}

/**
  * @brief  
  * @param  ندارد
  * @return ندراد
  */
void spiUnLock(void)
{
  osStatus_t status;

  if (spi_mutex_id != NULL)
  {
    status = osMutexRelease(spi_mutex_id);
    if (status != osOK)
    {
    }
  }
}

/**
  * @}
  */

/**
  * @}
  */

/*********************************END OF FILE****************************/
