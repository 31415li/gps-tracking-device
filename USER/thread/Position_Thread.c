/**
  ******************************************************************************
  * @file    Position_Thread.c
  * @author  Mahdad Ghasemian
  * @version V0.0.0
  * @date    3-March-2018
  * @brief   
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "Position_Thread.h"
#include "cmsis_os2.h" // ::CMSIS:RTOS2
#include "rtx_os.h"    // ARM::CMSIS:RTOS2:Keil RTX5
#include "bsp.h"
#include "app_cfg.h"
#include "sim808_driver.h"
#include "flash.h"
#include "record.h"
#include "includes.h"
#include <string.h>
#include "sim808_app.h"
#include "system.h"

/** @defgroup POSITION_THREAD
  * @brief 
  * @{
  */

/** @defgroup POSITION_THREAD_Private_Variables
  * @{
  */

extern osEventFlagsId_t app_eventflag_id;

// Define objects that are statically allocated
__attribute__((section(".bss.os.thread.cb")))
osRtxThread_t position_thread_tcb;
// Reserve two areas for the stacks
// uint64_t makes sure the memory alignment is 8
uint64_t position_thread_stk[POSITION_THREAD_STK_SIZE];
// Define the attributes which are used for thread creation
const osThreadAttr_t position_thread_attr = {
    "position",
    osThreadJoinable,
    &position_thread_tcb,
    sizeof(position_thread_tcb),
    &position_thread_stk[0],
    sizeof(position_thread_stk),
    POSITION_THREAD_PRIO,
    0};
// Define ID object for thread
osThreadId_t position_thread_id;

uint8_t bufWrite[256];
uint8_t bufRead[256];

extern uint16_t speedRecord;

uint8_t countOfZeroSpeed = 0;
uint8_t inMovingflag = 1;
uint8_t intervalCount = 0;

/**
  * @}
  */

/** @defgroup POSITION_THREAD_Private_Functions
  * @{
  */

static void Position_Thread(void *argument);
static uint8_t checkSensorChange(uint8_t *accNowState, uint8_t *accChange,
                                 uint8_t *powerNowState, uint8_t *powerChange,
                                 uint8_t *doorNowState, uint8_t *doorChange);
static void readSensors(uint8_t *accState, uint8_t *powerState, uint8_t *doorState);
static uint8_t saveCombined1Record(void);
static void blinkLedGreen(uint8_t count);
static void checkAlarmStatus(void);

/**
  * @brief  
  * @param  
  * @return 
  */
void Position_ObjCreate(void)
{
}

/**
  * @brief  
  * @param  
  * @return 
  */
void Position_ThreadCreate(void)
{
   uint32_t param = NULL;

   // Create an instance of the position thread with static resources (TCB and stack)
   position_thread_id = osThreadNew(Position_Thread, &param, &position_thread_attr);
}

/**
  * @brief  
  * @param  
  * @return 
  */
static void Position_Thread(void *argument)
{
   uint16_t i;
   RECORD_Status recordStatus;
   uint32_t flag, read, write;
   uint16_t count = 0;
   uint8_t resetFlag = 0;

   // reload watch
   IWDG_ReloadCounter();

   osDelay(1000);

   flag = osEventFlagsGet(app_eventflag_id);
   if (flag & APP_FLASH_INIT_RECORD_FLAG)
   {
      recordLock();
      recordGetIndex(&read, &write);
      recordUnLock();

      APP_TRACE_INFO(("Read=%d, Write=%d\r", read, write));
   }

   for (;;)
   {
      // reload watch
      IWDG_ReloadCounter();

      flag = osEventFlagsWait(app_eventflag_id,
                              APP_ACC_INPUT_INT_FLAG |
                                  APP_DOOR_INPUT_INT_FLAG |
                                  APP_POWER_INPUT_INT_FLAG,
                              osFlagsWaitAny,
                              9000);
      if (flag == osFlagsErrorTimeout)
      {
         // timeout
         if (SystemSetup.SaveAnyTime == 0)
         {
            uint8_t acc, power, door;

            // وضعيت سوئيچ را بررسي مي کند
            // اگه سوئيچ روشن باشه همان 10 ثانيه ذخيره انجام مي شه
            // اگه سوئيچ خاموش باشه وضعيت پارک بودن خودرو بررسي مي شه
            readSensors(&acc, &power, &door);
            if (acc == 0)
            {
               // تشخيص در حال حرکت بودن
               if (gpsData.speed > 1)
               {
                  // سرعت بيش از 1 کيلومتر در ساعت درحال حرکت حساب مي شود
                  countOfZeroSpeed = 0;
                  intervalCount = 0;
                  inMovingflag = 1;
               }
               else
               {
                  // اگه سرعت زير 2 کيلومتر بود پس خودرو ايستاده است احتمالا
                  if (countOfZeroSpeed < UINT8_MAX)
                     countOfZeroSpeed++;

                  // اگر خودرو حدود 5 دقيقه سرعت زير 2 کيلومتر را داشته باشد پس فرض ميکنيم که خودرو پارک شده است.
                  if (countOfZeroSpeed > 30)
                  {
                     countOfZeroSpeed = 0;
                     inMovingflag = 0;
                  }

                  // هر نيم ساعت يک بار براي جلوگيري از اشتباه بيخيال پارک بودن خودرو ميشه
                  //if (intervalCount < UINT8_MAX)
                  //   intervalCount++;
                  // اينجا بيخال پارک بودن خودرو ميشه و شمارنده ها را صفر ميکنه و در حال حرکت را يک مي کنه
                  //if (intervalCount > 180) {
                  //   intervalCount = 0;
                  //   countOfZeroSpeed = 0;
                  //   inMovingflag = 1;
                  //}
               }
            }
            else
            {
               inMovingflag = 1;
            }
         }
         else
         {
            inMovingflag = 1;
         }

         // ذخيره رکورد
         // در صورتي که در حال حرکت باشد هر 10 ثانيه يک ثبت رکورد
         // در صورتي که ثابت باشد هر 5 دقيقه يک ثبت رکورد
         // در صورتي که توسط تنظيمات ارسال هميشگي فعال باشه هر 10 ثانيه يک رکورد ذخيره مي شه
         if (inMovingflag == 1 || count > 30)
         {
            //  آيا ماژول جي اس ام جي پي اس اصلا روشن هست؟
            if (osEventFlagsGet(app_eventflag_id) & APP_GSM_INIT_OK_FLAG)
            {
               recordStatus = saveCombined1Record();
               if (recordStatus == RECORD_SUCCESS)
               {
                  APP_TRACE_INFO(("Add a record\r"));
                  blinkLedGreen(5);
               }
            }

            count = 0;
         }

         count++;
      }
      else if ((flag & APP_ACC_INPUT_INT_FLAG) ||
               (flag & APP_DOOR_INPUT_INT_FLAG) ||
               (flag & APP_POWER_INPUT_INT_FLAG))
      {
         // changed in state of sensor

         // بررسي وضعيت آلارم
         checkAlarmStatus();

         //  آيا ماژول جي اس ام جي پي اس اصلا روشن هست؟
         if (osEventFlagsGet(app_eventflag_id) & APP_GSM_INIT_OK_FLAG)
         {
            recordStatus = saveCombined1Record();
            if (recordStatus == RECORD_SUCCESS)
            {
               APP_TRACE_INFO(("event sensor and add reocrd\r"));
               blinkLedGreen(5);
            }
         }
      }

      // آيا به ساعت 12 شب رسيده ايم؟
      if (gpsData.hour == 19 && gpsData.min == 29 && gpsData.sec >= 0 && gpsData.sec < 22)
      {
         // پس يک بار ميکرو را ريست کن
         APP_TRACE_INFO(("Reset ---------------------------\r"));
         NVIC_SystemReset();
      }

      if (gpsData.speed > SystemSetup.MaximumSpeed)
      {
         speedRecord = gpsData.speed;
         osEventFlagsSet(app_eventflag_id, APP_MAXIMUM_SPEED_FLAG);
      }

      if (gpsData.fix)
         bspLedGreenON();
      else
         bspLedGreenOFF();
   }
}

/**
  * @brief  
  * @param  
  * @return 
  */
void clearCheckMovingIntervals(void)
{
   intervalCount = 0;
   countOfZeroSpeed = 0;
   inMovingflag = 1;
}

/**
  * @brief  
  * @param  
  * @return 
  */
static uint8_t checkSensorChange(
    uint8_t *accNowState, uint8_t *accChange,
    uint8_t *powerNowState, uint8_t *powerChange,
    uint8_t *doorNowState, uint8_t *doorChange)
{
   static uint8_t oldAcc = 2;
   static uint8_t oldPower = 2;
   static uint8_t oldDoor = 2;
   uint8_t acc, power, door;

   // read now sensors status
   readSensors(&acc, &power, &door);

   // check acc
   *accNowState = acc;
   if (oldAcc != acc)
   {
      oldAcc = acc & 0x01;
      *accChange = 1;
   }
   else
   {
      *accChange = 0;
   }

   // check power
   *powerNowState = power;
   if (oldPower != power)
   {
      oldPower = power & 0x01;
      *powerChange = 1;
   }
   else
   {
      *powerChange = 0;
   }

   // check door
   *doorNowState = door;
   if (oldDoor != door)
   {
      oldDoor = door & 0x01;
      *doorChange = 1;
   }
   else
   {
      *doorChange = 0;
   }
}

/**
  * @brief  
  * @param  
  * @return 
  */
static void readSensors(uint8_t *accState, uint8_t *powerState, uint8_t *doorState)
{
   uint8_t count;

   count = 5;
   while (count)
   {
      if (bspAccReadStatus(accState) == 1)
         break;
      count--;

      // reload watch
      IWDG_ReloadCounter();
   }

   osDelay(10);

   count = 5;
   while (count)
   {
      if (bspPowerReadStatus(powerState) == 1)
         break;
      count--;

      // reload watch
      IWDG_ReloadCounter();
   }

   osDelay(10);

   count = 5;
   while (count)
   {
      if (bspDoorReadStatus(doorState) == 1)
         break;
      count--;

      // reload watch
      IWDG_ReloadCounter();
   }
}

/**
  * @brief  
  * @param  
  * @return 
  */
static uint8_t saveCombined1Record(void)
{
   Record_t record;
   valueUint32 value32;
   valueUint16 value16;
   RECORD_Status recordStatus;
   uint32_t flag;
   uint8_t j = 0, state;
   uint8_t accNowState, accChange;
   uint8_t powerNowState, powerChange;
   uint8_t doorNowState, doorChange;

   // Type
   record.Type = 0x03;

   // Time
   value32.self = timeToSecond(gpsData.year, gpsData.month, gpsData.day,
                               gpsData.hour, gpsData.min, gpsData.sec);
   record.Data[j++] = value32.byte[0];
   record.Data[j++] = value32.byte[1];
   record.Data[j++] = value32.byte[2];
   record.Data[j++] = value32.byte[3];

   // Latitude
   value32.self = gpsData.latitude;
   record.Data[j++] = value32.byte[0];
   record.Data[j++] = value32.byte[1];
   record.Data[j++] = value32.byte[2];
   record.Data[j++] = value32.byte[3];

   // Longitude
   value32.self = gpsData.longitude;
   record.Data[j++] = value32.byte[0];
   record.Data[j++] = value32.byte[1];
   record.Data[j++] = value32.byte[2];
   record.Data[j++] = value32.byte[3];

   // Height
   value32.self = gpsData.height;
   record.Data[j++] = value32.byte[0];
   record.Data[j++] = value32.byte[1];
   record.Data[j++] = value32.byte[2];
   record.Data[j++] = value32.byte[3];

   // Horizontal Accuracy (m)
   record.Data[j++] = gpsData.hAcc;

   // Vertical Accuracy (m)
   record.Data[j++] = gpsData.vAcc;

   // Speed 2D
   value16.self = gpsData.speed;
   record.Data[j++] = value16.byte[0];
   record.Data[j++] = value16.byte[1];

   // FixMode
   record.Data[j++] = gpsData.fixMode;

   // Heading
   value32.self = gpsData.heading;
   record.Data[j++] = value32.byte[0];
   record.Data[j++] = value32.byte[1];
   record.Data[j++] = value32.byte[2];
   record.Data[j++] = value32.byte[3];

   // Input Group 1
   checkSensorChange(&accNowState, &accChange,
                     &powerNowState, &powerChange,
                     &doorNowState, &doorChange);
   state = 0;
   state = (uint8_t)((doorChange & 0x01) << 5) |
           (uint8_t)((doorNowState & 0x01) << 4) |
           (uint8_t)((powerChange & 0x01) << 3) |
           (uint8_t)((powerNowState & 0x01) << 2) |
           (uint8_t)((accChange & 0x01) << 1) |
           (uint8_t)((accNowState & 0x01) << 0);

   record.Data[j++] = state;

   // Input Group 2
   record.Data[j++] = 0;

   // Sim Charge
   value16.self = 0;
   record.Data[j++] = value16.byte[0];
   record.Data[j++] = value16.byte[1];

   // Battery Charge
   record.Data[j++] = 0;

   // Reserve
   record.Data[j++] = 0;
   record.Data[j++] = 0;

   // Save record
   flag = osEventFlagsGet(app_eventflag_id);
   if (flag & APP_FLASH_INIT_RECORD_FLAG)
   {
      recordLock();
      recordStatus = recordPut(&record);
      if (recordStatus == RECORD_ERROR_EEPROM)
         osEventFlagsClear(app_eventflag_id, APP_FLASH_INIT_RECORD_FLAG);
      record.Type = 0x04;
      recordWriteInRam(&record);
      recordUnLock();
      APP_TRACE_INFO(("Record Write In SPI Flash\r"));
   }
   else
   {
      recordLock();
      recordWriteInRam(&record);
      recordUnLock();
      APP_TRACE_INFO(("Record Write In Ram\r"));
   }

   return recordStatus;
}

/**
  * @brief  
  * @param  
  * @return 
  */
static void blinkLedGreen(uint8_t count)
{
   uint8_t i;
   for (i = 0; i < count; i++)
   {
      bspLedGreenOFF();
      osDelay(100);
      bspLedGreenON();
      osDelay(100);
   }
}

/**
  * @brief  
  * @param  
  * @return 
  */
static void checkAlarmStatus(void)
{
   if (SystemSetup.AlarmStatusFlag)
   {
      uint8_t acc, power, door, call = 0;

      readSensors(&acc, &power, &door);

      if (door == 1)
         osEventFlagsSet(app_eventflag_id, APP_RED_STATUS_DOOR_FLAG |
                                               APP_RED_STATUS_CALL_FLAG);
      if (acc == 1)
         osEventFlagsSet(app_eventflag_id, APP_RED_STATUS_ACC_FLAG |
                                               APP_RED_STATUS_CALL_FLAG);
      if (power == 0)
         osEventFlagsSet(app_eventflag_id, APP_RED_STATUS_POWER_FLAG |
                                               APP_RED_STATUS_CALL_FLAG);
      if (gpsData.speed > SystemSetup.TSLimit)
         osEventFlagsSet(app_eventflag_id, APP_RED_STATUS_MOVING_FLAG |
                                               APP_RED_STATUS_CALL_FLAG);
   }
}

/**
  * @}
  */

/**
  * @}
  */

/*********************************END OF FILE****************************/
