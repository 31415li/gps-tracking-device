/**
  ******************************************************************************
  * @file    record.c
  * @author  Mahdad Ghasemian
  * @version V1.0.0
  * @date    17-March-2019
  * @brief   
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "record.h"
#include "flash.h"
#include "string.h"
#include "system.h"

/** @defgroup RECORD 
  * @brief 
  * @{
  */

/** @defgroup RECORD_Private_Macros
  * @{
  */

/**
  * @}
  */

/** @defgroup RECORD_Private_Variables
  * @{
  */

extern osMutexId_t record_mutex_id;

record_monitor_t RecordMonitor;

struct
{
   uint8_t NewFlag;
   Record_t Record;
} recordInRam;

/**
  * @}
  */

/** @defgroup RECORD_Private_Functions
  * @{
  */

/**
  * @brief  
  * @param  
  * @return 
  */

RECORD_Status recordPut(Record_t *record)
{
   uint32_t address;
   uint32_t pageAddress;
   uint32_t indexInPage;

   if (RecordMonitor.WriteIndex >= RECORD_INDEX_MAX)
      RecordMonitor.WriteIndex = RECORD_INDEX_MIN;

   // Write to sFlash
   address = RecordMonitor.WriteIndex * RECORD_SIZE;
   pageAddress = address / FLASH_PAGE_SIZE;
   indexInPage = address % FLASH_PAGE_SIZE;
   if (indexInPage == 0)
   {
      uint32_t readAddress, readPageAddress;

      sFlashPageErase(pageAddress);

      readAddress = RecordMonitor.ReadIndex * RECORD_SIZE;
      readPageAddress = readAddress / FLASH_PAGE_SIZE;

      if (RecordMonitor.WriteIndex < RecordMonitor.ReadIndex &&
          pageAddress == readPageAddress)
      {
         RecordMonitor.ReadIndex += FLASH_PAGE_SIZE;
         if (RecordMonitor.ReadIndex >= RECORD_INDEX_MAX)
            RecordMonitor.ReadIndex = RECORD_INDEX_MIN + FLASH_PAGE_SIZE;
      }
   }
   sFlashWriteByteToMainMemory(pageAddress, indexInPage, (uint8_t *)record, RECORD_SIZE);

   RecordMonitor.WriteIndex++;
   if (RecordMonitor.WriteIndex >= RECORD_INDEX_MAX)
      RecordMonitor.WriteIndex = RECORD_INDEX_MIN;

   if (reocrdMonitorIndexWrite() == -1)
      return RECORD_ERROR_EEPROM;

   return RECORD_SUCCESS;
}

/**
  * @brief  
  * @param  
  * @return 
  */
RECORD_Status recordGet(uint8_t *record)
{
   uint32_t address;
   uint32_t pageAddress;
   uint32_t indexInPage;
   uint16_t count;

   if (recordUnread() == 0)
      return RECORD_READ_FULL;

   address = RecordMonitor.ReadIndex * RECORD_SIZE;
   pageAddress = address / FLASH_PAGE_SIZE;
   indexInPage = address % FLASH_PAGE_SIZE;
   sFlashReadByteFromMainMemory(pageAddress, indexInPage, record, RECORD_SIZE);

   RecordMonitor.ReadIndex++;
   if (RecordMonitor.ReadIndex >= RECORD_INDEX_MAX)
      RecordMonitor.ReadIndex = RECORD_INDEX_MIN;

   if (reocrdMonitorIndexWrite() == -1)
      return RECORD_ERROR_EEPROM;

   return RECORD_SUCCESS;
}

/**
  * @brief  
  * @param  
  * @return 
  */
RECORD_Status recordGets(uint8_t count, uint8_t *countRead, uint8_t *record)
{
   uint8_t i;
   RECORD_Status status = RECORD_ERROR;

   for (i = 0; i < count; i++)
   {
      status = recordGet((uint8_t *)&record[i * RECORD_SIZE]);
      if (status != RECORD_SUCCESS)
      {
         break;
      }
   }

   *countRead = i;

   if (i > 0)
      return RECORD_SUCCESS;

   return status;
}

/**
  * @brief  
  * @param  
  * @return 
  */
static uint16_t recordUnread(void)
{
   uint16_t count = 0;

   if (RecordMonitor.WriteIndex > RecordMonitor.ReadIndex)
      count = RecordMonitor.WriteIndex - RecordMonitor.ReadIndex;
   else if (RecordMonitor.ReadIndex > RecordMonitor.WriteIndex)
      count = RecordMonitor.WriteIndex + (RECORD_INDEX_MAX - RecordMonitor.ReadIndex);
   else
      count = 0;

   return count;
}

/**
  * @brief  
  * @param  
  * @return 
  */
RECORD_Status recordInit(void)
{

   if (reocrdMonitorIndexLoad() == -1)
      return RECORD_ERROR_EEPROM;

   return RECORD_SUCCESS;
}

/**
  * @brief  
  * @param  
  * @return 
  */
RECORD_Status recordClearIndex(void)
{

   RecordMonitor.ReadIndex = 0;
   RecordMonitor.WriteIndex = 0;

   if (reocrdMonitorIndexWrite() == -1)
      return RECORD_ERROR_EEPROM;

   return RECORD_SUCCESS;
}

/**
  * @brief  
  * @param  
  * @return 
  */
RECORD_Status recordGetIndex(uint32_t *read, uint32_t *write)
{

   *read = RecordMonitor.ReadIndex;
   *write = RecordMonitor.WriteIndex;

   return RECORD_SUCCESS;
}

/**
  * @brief  
  * @param  
  * @return 
  */
RECORD_Status recordSetReadIndex(uint32_t index)
{

   RecordMonitor.ReadIndex = index;

   if (reocrdMonitorIndexWrite() == -1)
      return RECORD_ERROR_EEPROM;

   return RECORD_SUCCESS;
}

/**
  * @brief  
  * @param  
  * @return 
  */
RECORD_Status recordSetWriteIndex(uint32_t index)
{

   RecordMonitor.WriteIndex = index;

   if (reocrdMonitorIndexWrite() == -1)
      return RECORD_ERROR_EEPROM;

   return RECORD_SUCCESS;
}

/**
  * @brief  
  * @param  
  * @return 
  */
void recordWriteInRam(Record_t *record)
{
   uint8_t i;

   recordInRam.Record.Type = record->Type;
   for (i = 0; i < RECORD_SIZE - 1; i++)
      recordInRam.Record.Data[i] = record->Data[i];
   recordInRam.NewFlag = 1;
}

/**
  * @brief  
  * @param  
  * @return 
  */
RECORD_Status recordReadInRam(uint8_t *record)
{
   uint8_t i;

   if (recordInRam.NewFlag == 1)
   {
      memcpy((uint8_t *)record, (uint8_t *)&recordInRam.Record, sizeof(Record_t));
      recordInRam.NewFlag = 0;

      return RECORD_SUCCESS;
   }

   return RECORD_READ_FULL;
}

/**
  * @brief  
  * @param  ندارد
  * @return ندراد
  */
void recordLock(void)
{
   osStatus_t status;

   if (record_mutex_id != NULL)
   {
      status = osMutexAcquire(record_mutex_id, osWaitForever);
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
void recordUnLock(void)
{
   osStatus_t status;

   if (record_mutex_id != NULL)
   {
      status = osMutexRelease(record_mutex_id);
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
