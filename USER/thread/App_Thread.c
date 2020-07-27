/**
  ******************************************************************************
  * @file    App_Thread.c
  * @author  Mahdad Ghasemian
  * @version V0.0.0
  * @date    25-March-2018
  * @brief   
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "App_Thread.h"
#include "cmsis_os2.h" // ::CMSIS:RTOS2
#include "rtx_os.h"    // ARM::CMSIS:RTOS2:Keil RTX5
#include "bsp.h"
#include "app_cfg.h"
#include <string.h>
#include "sim808_driver.h"
#include "sim808_app.h"
#include "includes.h"
#include "aes.h"
#include "system.h"
#include "record.h"

/** @defgroup APP_THREAD
  * @brief 
  * @{
  */

/** @defgroup APP_THREAD_Private_Variables
  * @{
  */

char buffSms[160];
uint8_t buffData[256];
uint8_t buffApp[256];
uint8_t buffT[256];
#define RECORD_GROUP_READ_COUNT 6

uint16_t speedRecord;

extern char avlRecordInRam[16];
extern uint16_t sequence;

// Define objects that are statically allocated
__attribute__((section(".bss.os.thread.cb")))
osRtxThread_t app_thread_tcb;
// Reserve two areas for the stacks
// uint64_t makes sure the memory alignment is 8
uint64_t app_thread_stk[APP_THREAD_STK_SIZE];
// Define the attributes which are used for thread creation
const osThreadAttr_t app_thread_attr = {
    "app",
    osThreadJoinable,
    &app_thread_tcb,
    sizeof(app_thread_tcb),
    &app_thread_stk[0],
    sizeof(app_thread_stk),
    APP_THREAD_PRIO,
    0};
// Define ID object for thread
osThreadId_t app_thread_id;

// Define objects that are statically allocated
__attribute__((section(".bss.os.evflags.cb")))
osRtxEventFlags_t app_eventflag_tcb;
// Define ID object for event flag
osEventFlagsId_t app_eventflag_id;
// Define the attributes which are used for event flag creation
const osEventFlagsAttr_t app_eventflag_attr = {
    "appFlag",
    0,
    &app_eventflag_tcb,
    sizeof(app_eventflag_tcb)};

static uint16_t callRedStatusInterval = 0;
static uint16_t sendSmsRedStatusAccInterval = 0;
static uint16_t sendSmsRedStatusPowerInterval = 0;
static uint16_t sendSmsRedStatusDoorInterval = 0;
static uint16_t sendSmsRedStatusMovingInterval = 0;
static uint16_t sendSmsTSLimitInterval = 0;

/**
  * @}
  */

/** @defgroup APP_THREAD_Private_Functions
  * @{
  */

static void App_Thread(void *argument);
static uint16_t padDataInMultipleOf16(uint8_t *data, uint16_t dataLen);

/**
  * @brief  
  * @param  
  * @return 
  */
void App_ObjCreate(void)
{
   // Create app event flag
   app_eventflag_id = osEventFlagsNew(&app_eventflag_attr);
}

/**
  * @brief  
  * @param  
  * @return 
  */
void App_ThreadCreate(void)
{
   uint32_t param = NULL;

   // Create an instance of the app thread with static resources (TCB and stack)
   app_thread_id = osThreadNew(App_Thread, &param, &app_thread_attr);
}

/**
  * @brief  
  * @param  
  * @return 
  */
static void App_Thread(void *argument)
{
   int16_t status;
   uint32_t flag;
   RECORD_Status recordStatus;

   osDelay(1000);

   flag = osEventFlagsWait(app_eventflag_id, APP_GSM_INIT_OK_FLAG, osFlagsWaitAny | osFlagsNoClear, osWaitForever);
   if (!(flag & APP_GSM_INIT_OK_FLAG))
      APP_TRACE_INFO(("APP_GSM_INIT_OK_FLAG Error\r"));

   for (;;)
   {

      // بررسي وضعيت فلگ ها
      flag = osEventFlagsGet(app_eventflag_id);

      // ارسال پيامک هشدار و تماس با مدير و کاربران در صورت وارد شدن به وضعيت قرمز
      if ((flag & APP_RED_STATUS_CALL_FLAG) ||
          (flag & APP_RED_STATUS_ACC_FLAG) ||
          (flag & APP_RED_STATUS_POWER_FLAG) ||
          (flag & APP_RED_STATUS_DOOR_FLAG) ||
          (flag & APP_RED_STATUS_MOVING_FLAG))
      {
         uint8_t door = 0, acc = 0, power = 0, moving = 0, call = 0;

         if ((flag & APP_RED_STATUS_ACC_FLAG) && (sendSmsRedStatusAccInterval == 0))
         {
            osEventFlagsClear(app_eventflag_id, APP_RED_STATUS_ACC_FLAG);
            sendSmsRedStatusAccInterval = 30;
            acc = 1;
         }

         if ((flag & APP_RED_STATUS_POWER_FLAG) && (sendSmsRedStatusPowerInterval == 0))
         {
            osEventFlagsClear(app_eventflag_id, APP_RED_STATUS_POWER_FLAG);
            sendSmsRedStatusPowerInterval = 30;
            power = 1;
         }

         if ((flag & APP_RED_STATUS_DOOR_FLAG) && (sendSmsRedStatusDoorInterval == 0))
         {
            osEventFlagsClear(app_eventflag_id, APP_RED_STATUS_DOOR_FLAG);
            sendSmsRedStatusDoorInterval = 30;
            door = 1;
         }

         if ((flag & APP_RED_STATUS_MOVING_FLAG) && (sendSmsRedStatusMovingInterval == 0))
         {
            osEventFlagsClear(app_eventflag_id, APP_RED_STATUS_MOVING_FLAG);
            sendSmsRedStatusMovingInterval = 30;
            moving = 1;
         }

         if ((flag & APP_RED_STATUS_CALL_FLAG) && (callRedStatusInterval == 0))
         {
            osEventFlagsClear(app_eventflag_id, APP_RED_STATUS_CALL_FLAG);
            callRedStatusInterval = 60;
            call = 1;
         }

         // ارسال پيامک به مدير و کاربران
         if (SystemSetup.AlarmModeSms == 1)
         {
            if (acc || power || door || moving)
            {
               uint8_t i;

               snprintf(buffSms, sizeof(buffSms),
                        "WARNING:\n%s%s%s%s",
                        (door) ? "Door is open\n" : "",
                        (moving) ? "Car is moving\n" : "",
                        (power) ? "Battery unplugged\n" : "",
                        (acc) ? "ACC is on\n" : "");

               Sim808Lock();
               SIM808SmsSend((char *)SystemSetup.adminPhoneNumber, buffSms);
               Sim808UnLock();

               // reload watch
               IWDG_ReloadCounter();

               for (i = 0; i < USER_MAX_COUNT; i++)
               {
                  if ((SystemSetup.userPhoneNumber[i][0] != 0) ||
                      (SystemSetup.userPhoneNumber[i][SIM808_PHONE_NUMBER_MAX_SIZE - 1] != 0))
                  {

                     osDelay(2000);

                     Sim808Lock();
                     SIM808SmsSend((char *)&SystemSetup.userPhoneNumber[i], buffSms);
                     Sim808UnLock();
                  }

                  // reload watch
                  IWDG_ReloadCounter();
               }

               osDelay(5000);

               // reload watch
               IWDG_ReloadCounter();
            }
         }

         // تماس با مدير و کاربران
         if (SystemSetup.AlarmModeCall == 1)
         {
            if (call == 1)
            {
               uint8_t i;

               Sim808Lock();
               if (SIM808CallToDialNumber((char *)SystemSetup.adminPhoneNumber) == 0)
               {
                  osDelay(10000);

                  // reload watch
                  IWDG_ReloadCounter();

                  osDelay(10000);

                  // reload watch
                  IWDG_ReloadCounter();

                  SIM808DisconnectCall();
               }
               Sim808UnLock();

               for (i = 0; i < USER_MAX_COUNT; i++)
               {
                  if ((SystemSetup.userPhoneNumber[i][0] != 0) ||
                      (SystemSetup.userPhoneNumber[i][SIM808_PHONE_NUMBER_MAX_SIZE - 1] != 0))
                  {

                     osDelay(5000);

                     // reload watch
                     IWDG_ReloadCounter();

                     Sim808Lock();
                     if (SIM808CallToDialNumber((char *)SystemSetup.userPhoneNumber[i]) == 0)
                     {
                        osDelay(10000);

                        // reload watch
                        IWDG_ReloadCounter();

                        osDelay(10000);

                        // reload watch
                        IWDG_ReloadCounter();

                        SIM808DisconnectCall();
                     }
                     Sim808UnLock();
                  }
               }

               osDelay(1000);

               // reload watch
               IWDG_ReloadCounter();

               // پس از اتمام تماس بهتر ديده شد که يک بار ارتباط با سرور قطع و وصل شود
               // آيا مد ارتباط با سرور فعال هست يا فقط کاربر از اس ام اس استفاده مي کند؟
               if (SystemSetup.ServerModeActive)
               {
                  // Shut down pdp for init state
                  AppGsmPSDClear();
                  AppGsmTCPClear();
                  Sim808Lock();
                  status = SIM808DeactiveGPRSPDP();
                  Sim808UnLock();
               }
            }
         }
      }

      // ارسال پيامک حداکثر سرعت مجاز
      if (flag & APP_MAXIMUM_SPEED_FLAG)
      {

         if (sendSmsTSLimitInterval == 0)
         {
            osEventFlagsClear(app_eventflag_id, APP_MAXIMUM_SPEED_FLAG);
            sendSmsTSLimitInterval = 30;

            snprintf(buffSms, sizeof(buffSms), "%s %d km/h", "very fast", speedRecord);

            Sim808Lock();
            SIM808SmsSend((char *)SystemSetup.adminPhoneNumber, buffSms);
            Sim808UnLock();

            // reload watch
            IWDG_ReloadCounter();

            osDelay(500);
         }
      }

      // ارسال رکورد ذخيره شده در صورت برقراري ارتباط با سرور
      if (flag & APP_GSM_TCP_OK_FLAG)
      {
         // آيا مد ارتباط با سرور فعال هست يا فقط کاربر از اس ام اس استفاده مي کند؟
         if (SystemSetup.ServerModeActive)
         {
            valueUint32 value32;
            valueUint16 value16;
            uint16_t i, unread;
            uint8_t len;
            uint8_t countRead;
            uint8_t size;
            uint8_t sourceType = 0;

            flag = osEventFlagsGet(app_eventflag_id);
            if (flag & APP_FLASH_INIT_RECORD_FLAG)
            {
               recordLock();
               recordStatus = recordGets(RECORD_GROUP_READ_COUNT, &countRead, buffT);
               if (recordStatus == RECORD_ERROR_EEPROM)
                  osEventFlagsClear(app_eventflag_id, APP_FLASH_INIT_RECORD_FLAG);

               if (countRead == RECORD_GROUP_READ_COUNT)
               {
                  if (recordReadInRam(&buffT[RECORD_SIZE * countRead]) == RECORD_SUCCESS)
                     countRead++;
               }
               recordUnLock();
               sourceType = 1;
            }
            else
            {
               recordLock();
               recordStatus = recordReadInRam(buffT);
               recordUnLock();
               countRead = 1;
               sourceType = 0;
            }

            if (recordStatus == RECORD_SUCCESS)
            {
               uint8_t j = 0, n = 0;

               memset(buffData, 0, sizeof(buffData));

               // CMD Code
               buffData[j++] = 6;
               // Serial Number
               value32.self = SystemSetup.SerialNumber;
               buffData[j++] = value32.byte[0];
               buffData[j++] = value32.byte[1];
               buffData[j++] = value32.byte[2];
               buffData[j++] = value32.byte[3];
               // Count Record
               buffData[j++] = countRead;
               // Data
               memcpy(&buffData[j], buffT, sizeof(Record_t) * countRead);
               j += sizeof(Record_t) * countRead;

               size = padDataInMultipleOf16(buffData, j);

               APP_TRACE_INFO(("Get a record\r"));

               encrypt_ecb(buffData, buffApp, size, (uint8_t *)TCP_AES_KEY);

               // Send TCP
               Sim808Lock();
               status = SIM808TcpSend(buffApp, size);
               Sim808UnLock();
               if (status == 0)
               {
                  APP_TRACE_INFO(("Send \r"));
               }
            }
         }
      }

      // کاهش شمارنده ها
      if (callRedStatusInterval > 0)
         callRedStatusInterval--;
      if (sendSmsRedStatusAccInterval > 0)
         sendSmsRedStatusAccInterval--;
      if (sendSmsRedStatusPowerInterval > 0)
         sendSmsRedStatusPowerInterval--;
      if (sendSmsRedStatusDoorInterval > 0)
         sendSmsRedStatusDoorInterval--;
      if (sendSmsRedStatusMovingInterval > 0)
         sendSmsRedStatusMovingInterval--;
      if (sendSmsTSLimitInterval > 0)
         sendSmsTSLimitInterval--;

      // تاخير
      osDelay(2000);
   }
}

/**
  * @brief  
  * @param  
  * @return 
  */
static uint16_t padDataInMultipleOf16(uint8_t *data, uint16_t dataLen)
{
   uint16_t count, i = 0;

   count = (dataLen % 16 == 0) ? 0 : (16 - (dataLen % 16));
   for (i = 0; i < count; i++)
      data[dataLen + i] = 0;
   return (dataLen + i);
}

/**
  * @brief  
  * @param  
  * @return 
  */
void clearIntervals(void)
{
   osEventFlagsClear(app_eventflag_id, APP_RED_STATUS_CALL_FLAG);
   osEventFlagsClear(app_eventflag_id, APP_RED_STATUS_ACC_FLAG);
   osEventFlagsClear(app_eventflag_id, APP_RED_STATUS_POWER_FLAG);
   osEventFlagsClear(app_eventflag_id, APP_RED_STATUS_DOOR_FLAG);
   osEventFlagsClear(app_eventflag_id, APP_RED_STATUS_MOVING_FLAG);

   callRedStatusInterval = 0;
   sendSmsRedStatusAccInterval = 0;
   sendSmsRedStatusPowerInterval = 0;
   sendSmsRedStatusDoorInterval = 0;
   sendSmsRedStatusMovingInterval = 0;
}

/**
  * @}
  */

/**
  * @}
  */

/*********************************END OF FILE****************************/
