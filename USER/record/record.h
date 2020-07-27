/**
  ******************************************************************************
  * @file    record.c
  * @author  Mahdad Ghasemian
  * @version V0.0.0
  * @date    10-April-2018
  * @brief   
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __RECORD_H
#define __RECORD_H

#ifdef __cplusplus
extern "C"
{
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f10x.h" // Device header

/** @defgroup RECORD 
  * @{
  */

/** @defgroup RECORD_Exported_Macros
  * @{
  */

//-------- <<< Use Configuration Wizard in Context Menu >>> -----------------
// <o> Record Size <16-64:16>
// <i> default 32
#define RECORD_SIZE 33
// <o> Record index min <0-135168:33>
// <i> multiple by 33
#define RECORD_INDEX_MIN 0
// <o> Record index max <0-135168:33>
// <i> multiple by 33
#define RECORD_INDEX_MAX 135168
// <o> Record Count without ACK <1-32>
// <i> default 16
#define RECORD_COUNT_WITHOUT_ACK 16
  // <<< end of configuration section >>>

#define RECORD_INDEX_SIZE (RECORD_INDEX_MAX - RECORD_INDEX_MIN)

  /**
  * @}
  */

  /** @defgroup RECORD_Exported_Types
  * @{
  */

  typedef enum
  {
    RECORD_SUCCESS = 0,
    RECORD_WRITE_FULL,
    RECORD_READ_FULL,
    RECORD_ERROR,
    RECORD_EMPTY,
    RECORD_ERROR_EEPROM
  } RECORD_Status;

  typedef struct
  {
    uint8_t Type;
    uint8_t Data[RECORD_SIZE - 1];
  } Record_t;

  /**
  * @}
  */

  /** @defgroup RECORD_Exported_Variables
  * @{
  */

  /**
  * @}
  */

  /** @defgroup RECORD_Exported_Functions
  * @{
  */

  RECORD_Status recordInit(void);
  RECORD_Status recordClearIndex(void);
  RECORD_Status recordGetIndex(uint32_t *read, uint32_t *write);
  RECORD_Status recordSetReadIndex(uint32_t index);
  RECORD_Status recordSetWriteIndex(uint32_t index);
  static uint16_t recordUnread(void);
  RECORD_Status recordPut(Record_t *record);
  RECORD_Status recordGets(uint8_t count,
                           uint8_t *countRead,
                           uint8_t *records);
  RECORD_Status recordAck();
  void recordWriteInRam(Record_t *record);
  RECORD_Status recordReadInRam(uint8_t *record);

  void recordLock(void);
  void recordUnLock(void);

  /**
  * @}
  */

#ifdef __cplusplus
}
#endif

/**
  * @}
  */

#endif /* __RECORD_H */

/*********************************END OF FILE****************************/
