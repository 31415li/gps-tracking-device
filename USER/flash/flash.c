/**
  ******************************************************************************
  * @file    flash.c
  * @author  Mahdad Ghasemian
  * @version V0.0.0
  * @date    08-April-2018
  * @brief   
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "flash.h"
#include "app_cfg.h"
#include "spi_driver.h"

/** @defgroup FLASH 
  * @brief 
  * @{
  */

/** @defgroup FLASH_Private_Macros
  * @{
  */

#define FLASH_SPI_CS_LOW() GPIO_ResetBits(FLASH_SPI_PORT, FLASH_SPI_CS_PIN)
#define FLASH_SPI_CS_HIGH() GPIO_SetBits(FLASH_SPI_PORT, FLASH_SPI_CS_PIN)

/**
  * @}
  */

/** @defgroup FLASH_Private_Variables
  * @{
  */

/**
  * @}
  */

/** @defgroup FLASH_Private_Functions
  * @{
  */

static Flash_Internal_Status_t sFlashReadStatusRegister(void);
static void sFlashBusyWait(void);
static void sFlashReadID(uint8_t *jedecCode,
                         uint8_t *familyCode,
                         uint8_t *densityCode);
static void sFlashReadBuffer(Flash_Buffer_Number_t number,
                             uint16_t index,
                             uint8_t *data,
                             uint16_t count);
static void sFlashWriteBuffer(Flash_Buffer_Number_t number,
                              uint16_t index,
                              uint8_t *data,
                              uint16_t count);
static void sFlashWriteBufferToMainMemoryWithErase(Flash_Buffer_Number_t number,
                                                   uint16_t pageAddress);
static void sFlashMainMemoryToBuffer(Flash_Buffer_Number_t number,
                                     uint16_t pageAddress);

/**
  * @brief  
  * @param  
  * @return 
  */
static Flash_Internal_Status_t sFlashReadStatusRegister(void)
{
   uint8_t temp1, temp2;

   FLASH_SPI_CS_LOW();
   spiSendReciveByte(FLASH_READ_STATUS_OPCODE);
   temp1 = spiSendReciveByte(0x00);
   temp2 = spiSendReciveByte(0x00);
   FLASH_SPI_CS_HIGH();

   if (temp1 & 0x80)
      return FLASH_INTERNAL_BUSY;
   else
      return FLASH_INTERNAL_READY;
}

/**
  * @brief  
  * @param  
  * @return 
  */
static void sFlashBusyWait(void)
{
   uint8_t temp1, temp2;

   do
   {
      FLASH_SPI_CS_LOW();
      spiSendReciveByte(FLASH_READ_STATUS_OPCODE);
      temp1 = spiSendReciveByte(0x00);
      temp2 = spiSendReciveByte(0x00);
      FLASH_SPI_CS_HIGH();
   } while ((temp1 & 0x80) == 0x00);
}

/**
  * @brief  
  * @param  
  * @return 
  */
static void sFlashReadID(uint8_t *jedecCode,
                         uint8_t *familyCode,
                         uint8_t *densityCode)
{
   uint8_t temp;

   FLASH_SPI_CS_LOW();
   spiSendReciveByte(FLASH_READ_ID_OPCODE);
   *jedecCode = spiSendReciveByte(0x00);
   temp = spiSendReciveByte(0x00);
   *familyCode = temp >> 5;
   *densityCode = temp & 0x1F;
   temp = spiSendReciveByte(0x00);

   FLASH_SPI_CS_HIGH();
}

/**
  * @brief  
  * @param  
  * @return 
  */
void sFlashReadBuffer(Flash_Buffer_Number_t number,
                      uint16_t index,
                      uint8_t *data,
                      uint16_t count)
{
   uint16_t i;

   spiLock();

   FLASH_SPI_CS_LOW();
   if (number == FLASH_BUFFER_1)
      spiSendReciveByte(FLASH_BUFFER_1_READ_OPCODE);
   else
      spiSendReciveByte(FLASH_BUFFER_2_READ_OPCODE);
   spiSendReciveByte(0x00);
   spiSendReciveByte((uint8_t)(index >> 8));
   spiSendReciveByte((uint8_t)index);
   for (i = 0; i < count; i++)
      *data++ = spiSendReciveByte(0x00);
   FLASH_SPI_CS_HIGH();

   spiUnLock();
}

/**
  * @brief  
  * @param  
  * @return 
  */
void sFlashWriteBuffer(Flash_Buffer_Number_t number,
                       uint16_t index,
                       uint8_t *data,
                       uint16_t count)
{
   uint16_t i;

   spiLock();

   FLASH_SPI_CS_LOW();
   if (number == FLASH_BUFFER_1)
      spiSendReciveByte(FLASH_BUFFER_1_WRITE_OPCODE);
   else
      spiSendReciveByte(FLASH_BUFFER_2_WRITE_OPCODE);
   spiSendReciveByte(0x00);
   spiSendReciveByte((uint8_t)(index >> 8));
   spiSendReciveByte((uint8_t)index);
   for (i = 0; i < count; i++)
      spiSendReciveByte(*data++);
   FLASH_SPI_CS_HIGH();

   spiUnLock();
}

/**
  * @brief  
  * @param  
  * @return 
  */
void sFlashWriteBufferToMainMemoryWithErase(Flash_Buffer_Number_t number,
                                            uint16_t pageAddress)
{
   spiLock();

   FLASH_SPI_CS_LOW();
   if (number == FLASH_BUFFER_1)
      spiSendReciveByte(FLASH_BUFFER1_TO_MAIN_MEMORY_WITH_ERASE_OPCODE);
   else
      spiSendReciveByte(FLASH_BUFFER2_TO_MAIN_MEMORY_WITH_ERASE_OPCODE);
   spiSendReciveByte((uint8_t)(pageAddress >> 6));
   spiSendReciveByte(((uint8_t)(pageAddress << 2)) & 0xFC);
   spiSendReciveByte(0x00);
   FLASH_SPI_CS_HIGH();

   sFlashBusyWait();

   spiUnLock();
}

/**
  * @brief  
  * @param  
  * @return 
  */
void sFlashTransferMainMemoryToBuffer(Flash_Buffer_Number_t number,
                                      uint16_t pageAddress)
{
   spiLock();

   FLASH_SPI_CS_LOW();
   if (number == FLASH_BUFFER_1)
      spiSendReciveByte(FLASH_MAIN_MEMORY_TO_BUFFER1_OPCODE);
   else
      spiSendReciveByte(FLASH_MAIN_MEMORY_TO_BUFFER2_OPCODE);
   spiSendReciveByte((uint8_t)(pageAddress >> 6));
   spiSendReciveByte(((uint8_t)(pageAddress << 2)) & 0xFC);
   spiSendReciveByte(0x00);
   FLASH_SPI_CS_HIGH();

   sFlashBusyWait();

   spiUnLock();
}

/**
  * @brief  
  * @param  
  * @return 
  */
Flash_Device_Status_t sFlashInit(void)
{
   uint8_t i;
   uint8_t jedecCode;
   uint8_t familyCode;
   uint8_t densityCode;

   for (i = 0; i < 5; i++)
   {
      spiLock();

      sFlashReadID(&jedecCode, &familyCode, &densityCode);

      spiUnLock();

      if (jedecCode == 0x1F)
         return FLASH_DEVICE_OK;
      flashDelayms(10);
   }
   return FLASH_DEVICE_ERROR;
}

/**
  * @brief  
  * @param  ندارد
  * @return ندراد
  */
void sFlashChipErase(void)
{
   spiLock();

   FLASH_SPI_CS_LOW();
   spiSendReciveByte(FLASH_ERASE_CHIP_A_OPCODE);
   spiSendReciveByte(FLASH_ERASE_CHIP_B_OPCODE);
   spiSendReciveByte(FLASH_ERASE_CHIP_C_OPCODE);
   spiSendReciveByte(FLASH_ERASE_CHIP_D_OPCODE);
   FLASH_SPI_CS_HIGH();

   sFlashBusyWait();

   spiUnLock();
}

/**
  * @brief  
  * @param  
  * @return ندراد
  */
void sFlashPageErase(uint16_t pageAddress)
{
   spiLock();

   FLASH_SPI_CS_LOW();
   spiSendReciveByte(FLASH_ERASE_PAGE_OPCODE);
   spiSendReciveByte((uint8_t)(pageAddress >> 6));
   spiSendReciveByte(((uint8_t)(pageAddress << 2)) & 0xFC);
   spiSendReciveByte(0x00);
   FLASH_SPI_CS_HIGH();

   sFlashBusyWait();

   spiUnLock();
}

/**
  * @brief  
  * @param  
  * @return 
  */
void sFlashReadByteFromMainMemory(uint16_t pageAddress,
                                  uint16_t indexInPage,
                                  uint8_t *data,
                                  uint16_t count)
{
   uint16_t i;

   spiLock();

   sFlashBusyWait();

   FLASH_SPI_CS_LOW();

   spiSendReciveByte(FLASH_CONTINUOUS_ARRAY_READ_OPCODE);
   spiSendReciveByte((uint8_t)(pageAddress >> 6));
   spiSendReciveByte((((uint8_t)(pageAddress << 2)) & 0xFC) | ((uint8_t)(indexInPage >> 8)));
   spiSendReciveByte((uint8_t)indexInPage);
   spiSendReciveByte(0x00);
   spiSendReciveByte(0x00);
   for (i = 0; i < count; i++)
   {
      *data++ = spiSendReciveByte(0x00);
   }

   FLASH_SPI_CS_HIGH();

   spiUnLock();
}

/**
  * @brief  
  * @param  
  * @return 
  */
void sFlashWriteByteToMainMemory(uint16_t pageAddress,
                                 uint16_t indexInPage,
                                 uint8_t *data,
                                 uint16_t count)
{
   uint16_t i;

   spiLock();

   FLASH_SPI_CS_LOW();
   spiSendReciveByte(FLASH_BYTE_TO_MAIN_MEMORY_OPCODE);
   spiSendReciveByte((uint8_t)(pageAddress >> 6));
   spiSendReciveByte((((uint8_t)(pageAddress << 2)) & 0xFC) | ((uint8_t)(indexInPage >> 8)));
   spiSendReciveByte((uint8_t)indexInPage);
   for (i = 0; i < count; i++)
      spiSendReciveByte(*data++);
   FLASH_SPI_CS_HIGH();

   sFlashBusyWait();

   spiUnLock();
}

/**
  * @}
  */

/**
  * @}
  */

/*********************************END OF FILE****************************/
