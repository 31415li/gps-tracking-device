/**
  ******************************************************************************
  * @file    flash.h
  * @author  Mahdad Ghasemian
  * @version V0.0.0
  * @date    08-April-2018
  * @brief   
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __FLASH_H
#define __FLASH_H

#ifdef __cplusplus
extern "C"
{
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f10x.h" // Device header
#include "cmsis_os2.h" // ::CMSIS:RTOS2
#include "rtx_os.h"    // ARM::CMSIS:RTOS2:Keil RTX5

  /** @defgroup FLASH 
  * @{
  */

  /** @defgroup FLASH_Exported_Macros
  * @{
  */

  //-------- <<< Use Configuration Wizard in Context Menu >>> -----------------

#define FLASH_PAGE_SIZE 528

#define FLASH_READ_STATUS_OPCODE 0xD7
#define FLASH_READ_ID_OPCODE 0x9F
#define FLASH_ERASE_CHIP_A_OPCODE 0xC7
#define FLASH_ERASE_CHIP_B_OPCODE 0x94
#define FLASH_ERASE_CHIP_C_OPCODE 0x80
#define FLASH_ERASE_CHIP_D_OPCODE 0x9A
#define FLASH_ERASE_PAGE_OPCODE 0x81
#define FLASH_CONTINUOUS_ARRAY_READ_OPCODE 0x1B
#define FLASH_BUFFER_1_READ_OPCODE 0xD4
#define FLASH_BUFFER_2_READ_OPCODE 0xD6
#define FLASH_BUFFER_1_WRITE_OPCODE 0x84
#define FLASH_BUFFER_2_WRITE_OPCODE 0x87
#define FLASH_BUFFER1_TO_MAIN_MEMORY_WITH_ERASE_OPCODE 0x83
#define FLASH_BUFFER2_TO_MAIN_MEMORY_WITH_ERASE_OPCODE 0x86
#define FLASH_MAIN_MEMORY_TO_BUFFER1_OPCODE 0x53
#define FLASH_MAIN_MEMORY_TO_BUFFER2_OPCODE 0x55
#define FLASH_BYTE_TO_MAIN_MEMORY_OPCODE 0x02

#define flashDelayms(x) osDelay(x)

  /**
  * @}
  */

  /** @defgroup FLASH_Exported_Types
  * @{
  */

  typedef enum
  {
    FLASH_DEVICE_OK = 0,
    FLASH_DEVICE_ERROR
  } Flash_Device_Status_t;

  typedef enum
  {
    FLASH_INTERNAL_BUSY = 0,
    FLASH_INTERNAL_READY
  } Flash_Internal_Status_t;

  typedef enum
  {
    FLASH_BUFFER_1 = 0,
    FLASH_BUFFER_2
  } Flash_Buffer_Number_t;

  /**
  * @}
  */

  /** @defgroup FLASH_Exported_Variables
  * @{
  */

  /**
  * @}
  */

  /** @defgroup FLASH_Exported_Functions
  * @{
  */

  Flash_Device_Status_t sFlashInit(void);
  void sFlashReadBuffer(Flash_Buffer_Number_t number,
                        uint16_t index,
                        uint8_t *data,
                        uint16_t count);
  void sFlashWriteBuffer(Flash_Buffer_Number_t number,
                         uint16_t index,
                         uint8_t *data,
                         uint16_t count);
  void sFlashWriteBufferToMainMemoryWithErase(Flash_Buffer_Number_t number,
                                              uint16_t pageAddress);
  void sFlashTransferMainMemoryToBuffer(Flash_Buffer_Number_t number,
                                        uint16_t pageAddress);
  void sFlashChipErase(void);
  void sFlashPageErase(uint16_t pageAddress);
  void sFlashReadByteFromMainMemory(uint16_t pageAddress,
                                    uint16_t indexInPage,
                                    uint8_t *data,
                                    uint16_t count);
  void sFlashWriteByteToMainMemory(uint16_t pageAddress,
                                   uint16_t indexInPage,
                                   uint8_t *data,
                                   uint16_t count);

  void flashLock(void);
  void flashUnLock(void);

  /**
  * @}
  */

#ifdef __cplusplus
}
#endif

/**
  * @}
  */

#endif /* __FLASH_H */

/*********************************END OF FILE****************************/
