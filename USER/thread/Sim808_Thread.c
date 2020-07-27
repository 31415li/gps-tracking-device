/**
  ******************************************************************************
  * @file    Sim808_Thread.c
  * @author  Mahdad Ghasemian
  * @version V0.0.0
  * @date    4-March-2018
  * @brief   
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "Sim808_Thread.h"
#include "cmsis_os2.h" // ::CMSIS:RTOS2
#include "rtx_os.h"    // ARM::CMSIS:RTOS2:Keil RTX5
#include "bsp.h"
#include "app_cfg.h"
#include "includes.h"
#include "sim808_driver.h"
#include <string.h>
#include <stdbool.h>
#include "system.h"

/** @defgroup SIM808_THREAD
  * @brief 
  * @{
  */

/** @defgroup SIM808_THREAD_Private_Variables
  * @{
  */

extern SIM808_New_Event_t newSmsReceiveFlag;
extern SIM808_New_Event_t newTcpReceiveFlag;
extern uint8_t timeoutCount;
extern uint8_t tcpSendFailCounter;

char buff[128];
char buff2[32];
uint8_t rssi;
uint8_t ber;

// Define objects that are statically allocated
__attribute__((section(".bss.os.thread.cb")))
osRtxThread_t sim808basic_thread_tcb;
// Reserve two areas for the stacks
// uint64_t makes sure the memory alignment is 8
uint64_t sim808basic_thread_stk[SIM808BASIC_THREAD_STK_SIZE];
// Define ID object for thread
osThreadId_t sim808basic_thread_id;
// Define the attributes which are used for thread creation
const osThreadAttr_t sim808basic_thread_attr = {
    "sim808Basic",
    osThreadJoinable,
    &sim808basic_thread_tcb,
    sizeof(sim808basic_thread_tcb),
    &sim808basic_thread_stk[0],
    sizeof(sim808basic_thread_stk),
    SIM808BASIC_THREAD_PRIO,
    0};

// Define objects that are statically allocated
__attribute__((section(".bss.os.thread.cb")))
osRtxThread_t sim808urc_thread_tcb;
// Reserve two areas for the stacks
// uint64_t makes sure the memory alignment is 8
uint64_t sim808urc_thread_stk[SIM808URC_THREAD_STK_SIZE];
// Define ID object for thread
osThreadId_t sim808urc_thread_id;
// Define the attributes which are used for thread creation
const osThreadAttr_t sim808urc_thread_attr = {
    "sim808Urc",
    osThreadJoinable,
    &sim808urc_thread_tcb,
    sizeof(sim808urc_thread_tcb),
    &sim808urc_thread_stk[0],
    sizeof(sim808urc_thread_stk),
    SIM808URC_THREAD_PRIO,
    0};

// Define objects that are statically allocated
__attribute__((section(".bss.os.mutex.cb")))
osRtxMutex_t sim808_mutex_tcb;
// Define ID object for mutex
osMutexId_t sim808_mutex_id;
// Define the attributes which are used for mutex creation
const osMutexAttr_t sim808_mutex_attr = {
    "sim808Mutex",
    osMutexRobust,
    &sim808_mutex_tcb,
    sizeof(sim808_mutex_tcb)};

// Define objects that are statically allocated
__attribute__((section(".bss.os.msgqueue.cb")))
osRtxMessageQueue_t sim808_messagequeue_tcb;
// Buffer
uint8_t sim808_messagequeue_buf[SIM808_MSG_COUNT][sizeof(SIM808_Msg_t) + 0xc];
// Define ID object for message queue
osMessageQueueId_t sim808_messagequeue_id;
// Define the attributes which are used for message queue creation
const osMessageQueueAttr_t sim808_messagequeue_attr = {
    "sim808Queue",
    0,
    &sim808_messagequeue_tcb,
    sizeof(sim808_messagequeue_tcb),
    &sim808_messagequeue_buf[0],
    SIM808_MSG_COUNT *(sizeof(SIM808_Msg_t) + 0xc)};

// Define objects that are statically allocated
__attribute__((section(".bss.os.evflags.cb")))
osRtxEventFlags_t sim808_eventflag_tcb;
// Define ID object for event flag
osEventFlagsId_t sim808_eventflag_id;
// Define the attributes which are used for event flag creation
const osEventFlagsAttr_t sim808_eventflag_attr = {
    "sim808Flag",
    0,
    &sim808_eventflag_tcb,
    sizeof(sim808_eventflag_tcb)};

/**
  * @}
  */

/** @defgroup SIM808_THREAD_Private_Functions
  * @{
  */

static void Sim808Basic_Thread(void *argument);
static void Sim808Urc_Thread(void *argument);

extern void TcpCmd(uint8_t *data, uint16_t length);
extern void SmsCmd(char *number, char *text);
extern void AppGsmINITSet(void);
extern void AppGsmINITClear(void);
extern void AppGsmPSDSet(void);
extern void AppGsmPSDClear(void);
extern void AppGsmTCPSet(void);
extern void AppGsmTCPClear(void);
extern void AppGsmBearerSet(void);
extern void AppGsmBearerClear(void);

/**
  * @brief  
  * @param  
  * @return 
  */
void Sim808_ObjCreate(void)
{
   // Create sim808 mutex
   sim808_mutex_id = osMutexNew(&sim808_mutex_attr);

   // Create sim808 message queue
   sim808_messagequeue_id = osMessageQueueNew(SIM808_MSG_COUNT, sizeof(SIM808_Msg_t), &sim808_messagequeue_attr);

   // Create sim808 event flag
   sim808_eventflag_id = osEventFlagsNew(&sim808_eventflag_attr);
}

/**
  * @brief  
  * @param  
  * @return 
  */
void Sim808_ThreadCreate(void)
{
   uint32_t param = NULL;

   // Create an instance of the basic and urc thread with static resources (TCB and stack)
   sim808urc_thread_id = osThreadNew(Sim808Urc_Thread, &param, &sim808urc_thread_attr);
   sim808basic_thread_id = osThreadNew(Sim808Basic_Thread, &param, &sim808basic_thread_attr);
}

/**
  * @brief  
  * @param  
  * @return 
  */
static void Sim808Basic_Thread(void *argument)
{
   uint16_t timeout;
   FunctionalState ecoState;
   Sms_Format smsMode;
   Sms_Receive_Mode_t smsReceiveMode;
   Sms_Receive_Rules_t smsReceiveRule;
   Network_Reg_URC_Mode_t netMode;
   Network_Reg_Status_t netStatus;
   int16_t status;
   uint32_t flag;
   GPRS_Attach_Status_t gprsStatus;
   PDP_Context_Status_t currentPdpStatus;
   Connection_Status_t connectionStatus;
   IPD_Status_t ipdStatus;
   GNSS_Power_Mode_t gnssPowermode;
   Bearer_Status_t bearerStatus;
   char ipBearer[24];
   uint16_t httpStatusCode;
   uint16_t httpDataLen;
   uint8_t fo;
   uint8_t vp;
   uint8_t pid;
   uint8_t dcs;

   osDelay(1000);

   // Power on
   Sim808Lock();
   status = SIM808SetPowerState(ENABLE);
   Sim808UnLock();
   if (status == 0)
   {
      APP_TRACE_INFO(("Module Power ON\r"));
      AppGsmINITSet();
      osDelay(100);
   }
   else
   {
      APP_TRACE_INFO(("Can not Power ON!\r"));
      APP_TRACE_INFO(("....................................................\r"));
      AppGsmINITClear();
      osDelay(5000);
      NVIC_SystemReset();
   }

   // Get Release Version
   Sim808Lock();
   SIM808GetTASoftwareRelease(buff);
   Sim808UnLock();
   APP_TRACE_INFO(("Release: %s\r", buff));

   // Get IMEI
   Sim808Lock();
   SIM808GetIMEI(buff);
   Sim808UnLock();
   APP_TRACE_INFO(("IMEI: %s\r", buff));

   for (;;)
   {

      /*
      Sim808Lock();
      SIM808ATTest("AT+CUSD=1",2000);
      osDelay(1000);
      SIM808ATTest("AT+CUSD=1,\"*733*2#\"",5000);
      Sim808UnLock();
      */

      // Eco OFF
      Sim808Lock();
      if (SIM808CheckEcoState(&ecoState) == 0 && ecoState == ENABLE)
         SIM808SetEcoState(DISABLE);
      Sim808UnLock();

#if SIM808_SMS_EN
      {
         // SMS Mode
         Sim808Lock();
         if (SIM808GetSmsFormat(&smsMode) == 0 && smsMode != SMS_FORMAT_MODE_TEXT)
            SIM808SetSmsFormat(SMS_FORMAT_MODE_TEXT);
         Sim808UnLock();

         // SMS Notification
         Sim808Lock();
         if (SIM808GetSmsNotification(&smsReceiveMode, &smsReceiveRule) == 0 &&
             (smsReceiveMode != SMS_RECEIVE_MODE_1 || smsReceiveRule != SMS_RECEIVE_RULE_0))
            SIM808SetSmsNotification(SMS_RECEIVE_MODE_1, SMS_RECEIVE_RULE_0);
         Sim808UnLock();

         // SMS Text mode parameter
         Sim808Lock();
         if (SIM808GetSmsTextModeParameter(&fo, &vp, &pid, &dcs) == 0)
         {
            if (fo != 17 || vp != 167 || pid != 0 || dcs != 0)
               SIM808SetSmsTextModeParameter(17, 167, 0, 0);
         }
         Sim808UnLock();
      }
#endif

      // GNSS Power ON
      Sim808Lock();
      if (SIM808GetGNSSPowerMode(&gnssPowermode) == 0 && gnssPowermode == GNSS_POWER_MODE_TURN_OFF)
         SIM808SetGNSSPowerMode(GNSS_POWER_MODE_TURN_ON);
      Sim808UnLock();

      // Last NMEA Parsed
      Sim808Lock();
      SIM808DefineLastParsedNMEA("GGA");
      Sim808UnLock();

      // Get CSQ
      Sim808Lock();
      status = SIM808GetSignalStrength(&rssi, &ber);
      Sim808UnLock();
      if (status == 0)
         APP_TRACE_INFO(("RSSI: %d, BER: %d\r", rssi, ber));

      // Get Network Registration Status
      Sim808Lock();
      status = SIM808GetNetworkRegistrationStatus(&netMode, &netStatus);
      Sim808UnLock();
      if (status == 0)
         APP_TRACE_INFO(("Net Registration: %d, %d\r", netMode, netStatus));

      // Get Position
      Sim808Lock();
      SIM808GetGNSSNMEAParameter();
      Sim808UnLock();

      APP_TRACE_INFO(("Run: %d, Fix: %d, "
                      "UTC: %04d/%02d/%02d %02d:%02d:%02d, "
                      "Lat: %i, Lon: %i, Speed: %d, FixMode: %d\r",
                      gpsData.run, gpsData.fix,
                      gpsData.year,
                      gpsData.month,
                      gpsData.day,
                      gpsData.hour,
                      gpsData.min,
                      gpsData.sec,
                      gpsData.latitude, gpsData.longitude,
                      gpsData.speed,
                      gpsData.fixMode));

#if (SIM808_TCP_EN || SIM808_HTTP_EN)
      {
         // آيا مد ارتباط با سرور فعال هست يا فقط کاربر از اس ام اس استفاده مي کند؟
         if (SystemSetup.ServerModeActive)
         {
            // +IPD Head Status
            Sim808Lock();
            if (SIM808GetIPDStatus(&ipdStatus) == 0 && ipdStatus != HEAD_PACKET_IPD)
               SIM808DefineIPDStatus(HEAD_PACKET_IPD);
            Sim808UnLock();

            // PDP Active
            Sim808Lock();
            status = SIM808GetPDPContextStatus(&currentPdpStatus);
            if (status == 0 && currentPdpStatus == GPRS_DETACHED)
            {
               status = SIM808SetPDPContextStatus(PDP_ACTIVATED);
            }
            Sim808UnLock();
            if (status == 0)
               APP_TRACE_INFO(("PDP Actived\r"));

            // GPRS Attach
            Sim808Lock();
            status = SIM808GetGPRSAttachStatus(&gprsStatus);
            if (status == 0 && gprsStatus == GPRS_DETACHED)
            {
               status = SIM808SetGPRSAttachStatus(GPRS_ATTACHED);
            }
            Sim808UnLock();
            if (status == 0)
               APP_TRACE_INFO(("GPRS Attach status: %d\r", gprsStatus));
         }
      }
#endif

#if SIM808_SMS_EN
      {
         // Check SMS Receive
         if (newSmsReceiveFlag == SIM808_NEW_EVENT)
         {
            newSmsReceiveFlag = SIM808_READ_EVENT;

            APP_TRACE_INFO(("Receive new SMS\r"));

            Sim808Lock();
            status = SIM808SmsRead(1, SMS_READ_MODE_NORMAL, buff2, buff);
            if (status == 0)
            {
               APP_TRACE_INFO(("Read SMS\r"));
               SIM808SmsDelete(1, SMS_DELETE_FLAG_4);
               if (status == 0)
                  APP_TRACE_INFO(("Delete SMS\r"));
            }
            Sim808UnLock();

            if (status == 0)
               SmsCmd(buff2, buff);
         }
      }
#endif

#if SIM808_TCP_EN
      {
         // آيا مد ارتباط با سرور فعال هست يا فقط کاربر از اس ام اس استفاده مي کند؟
         if (SystemSetup.ServerModeActive)
         {
            // Check TCP Receive
            if (newTcpReceiveFlag == SIM808_NEW_EVENT)
            {
               APP_TRACE_INFO(("Receive new TCP\r"));
               extern uint8_t receivePacket2[SIM808_RECEIVE_TCP_MAX_SIZE];
               extern uint16_t receivePacketLen2;

               Sim808Lock();
               Sim808TrueReceiveData();
               Sim808UnLock();

               TcpCmd(receivePacket2, receivePacketLen2);

               newTcpReceiveFlag = SIM808_READ_EVENT;
            }

            // Get Connection Status
            Sim808Lock();
            status = SIM808GetConnectionStatus(&connectionStatus);
            Sim808UnLock();
            if (status == 0)
            {
               APP_TRACE_INFO(("Connection Status: %d\r", connectionStatus));

               if (connectionStatus != CONNECT_OK)
               {
                  AppGsmTCPClear();
               }
               if (connectionStatus == PDP_DEACT ||
                   connectionStatus == TCP_CLOSED)
               {
                  AppGsmPSDClear();
                  AppGsmTCPClear();
                  // Shut down pdp for init state
                  Sim808Lock();
                  status = SIM808DeactiveGPRSPDP();
                  Sim808UnLock();
                  if (status == 0)
                     APP_TRACE_INFO(("Deactive GPRS PDP!\r"));
               }
               if (connectionStatus == IP_INITIAL)
               {
                  // Set APN Setting
                  Sim808Lock();
                  status = SIM808SetApnNameAndStartTask(APN_NAME);
                  Sim808UnLock();
                  if (status == 0)
                     APP_TRACE_INFO(("APN Set!\r"));
               }
               if (connectionStatus == IP_START)
               {
                  // Bringup wireless connection
                  Sim808Lock();
                  status = SIM808BringUpWireless();
                  Sim808UnLock();
                  if (status == 0)
                     APP_TRACE_INFO(("Bringup wireless connection!\r"));
               }
               if (connectionStatus == IP_GPRSACT ||
                   connectionStatus == IP_STATUS ||
                   connectionStatus == TCP_CONNECTING ||
                   connectionStatus == CONNECT_OK ||
                   connectionStatus == TCP_CLOSING ||
                   connectionStatus == TCP_CLOSED)
               {
                  // Get Local IP Address
                  Sim808Lock();
                  status = SIM808GetLocalIPAddress(buff);
                  Sim808UnLock();
                  if (status == 0)
                     APP_TRACE_INFO(("Local IP: %s\r", buff));
               }
               if (connectionStatus == IP_INITIAL ||
                   connectionStatus == IP_STATUS)
               {
                  Cip_Status_t cipstatus;

                  // Start TCP
                  Sim808Lock();
                  status = SIM808StartTCP(SystemSetup.ServerIP, SystemSetup.ServerPort, &cipstatus);
                  Sim808UnLock();
                  if (status == 0)
                     APP_TRACE_INFO(("TCP Start : %d\r", cipstatus));
               }
               if (connectionStatus == CONNECT_OK)
               {
                  if (tcpSendFailCounter > 5)
                  {
                     APP_TRACE_INFO(("TCP Send fail counter is active!\r"));
                     // Shut down pdp for init state
                     AppGsmPSDClear();
                     AppGsmTCPClear();
                     Sim808Lock();
                     status = SIM808DeactiveGPRSPDP();
                     Sim808UnLock();
                     if (status == 0)
                        APP_TRACE_INFO(("Deactive GPRS PDP!\r"));
                     tcpSendFailCounter = 0;
                  }
                  else
                  {
                     tcpSendFailCounter = 0;
                     AppGsmTCPSet();
                  }
               }
            }
         }
      }
#endif

#if SIM808_HTTP_EN
      {
         // آيا مد ارتباط با سرور فعال هست يا فقط کاربر از اس ام اس استفاده مي کند؟
         if (SystemSetup.ServerModeActive)
         {
            // Http Initialize
            Sim808Lock();
            status = SIM808HttpInitialize();
            Sim808UnLock();
            if (status == 0)
               APP_TRACE_INFO(("Http Initialized\r"));

            // Set GPRS Bearer Connection
            Sim808Lock();
            status = SIM808SetBearerInternetType(1, GPRS_CONNECTION);
            Sim808UnLock();
            if (status == 0)
               APP_TRACE_INFO(("GPRS Bearer Connection\r"));

            // Set APN name Bearer
            Sim808Lock();
            status = SIM808SetBearerApnName(1, APN_NAME);
            Sim808UnLock();
            if (status == 0)
               APP_TRACE_INFO(("APN Set Bearer\r"));

            // Set Http Cid
            Sim808Lock();
            status = SIM808SetHttpCid(1);
            Sim808UnLock();
            if (status == 0)
               APP_TRACE_INFO(("Set Http Cid to 1\r"));

            // Set Http Server URL
            Sim808Lock();
            status = SIM808SetHttpURL(SERVER_IP, SERVER_PORT, "devices");
            Sim808UnLock();
            if (status == 0)
               APP_TRACE_INFO(("Set Http URL\r"));

            // Set Http Conent_type
            Sim808Lock();
            status = SIM808SetHttpCntentType("text/plain");
            //status = SIM808SetHttpCntentType("application/octet-stream");
            Sim808UnLock();
            if (status == 0)
               APP_TRACE_INFO(("Set Cntent-Type to text/plain\r"));

            // Get Bearer Status
            Sim808Lock();
            status = SIM808GetBearerStatus(1, &bearerStatus, ipBearer);
            Sim808UnLock();
            if (status == 0)
            {
               APP_TRACE_INFO(("Bearer Status: %d\r", bearerStatus));

               if (bearerStatus == BEARER_IS_CLOSED)
               {
                  // Bearer Open
                  Sim808Lock();
                  status = SIM808BearerOpen(1);
                  Sim808UnLock();
                  if (status == 0)
                     APP_TRACE_INFO(("Bearer Opened\r"));
               }
               if (bearerStatus == BEARER_IS_CONNECTED)
               {
                  AppGsmBearerSet();

                  Sim808Lock();
                  status = SIM808HttpGet(&httpStatusCode, &httpDataLen);
                  Sim808UnLock();
                  if (status == 0)
                  {
                     APP_TRACE_INFO(("Get Http Status = %d, data len = %d\r", httpStatusCode, httpDataLen));

                     Sim808Lock();
                     status = SIM808HttpRead(0, httpDataLen);
                     Sim808UnLock();

                     extern uint8_t httpReadData[SIM808_RECEIVE_HTTP_MAX_SIZE];
                     if (status == 0)
                        APP_TRACE_INFO(("Get Http Data = %s\r", httpReadData));

                     Sim808Lock();
                     {
                        uint8_t data[8] = {0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38};
                        status = SIM808HttpInputData(data, 8);
                     }
                     Sim808UnLock();
                     if (status == 0)
                     {
                        APP_TRACE_INFO(("Input Data OK\r"));

                        Sim808Lock();
                        status = SIM808HttpPost(&httpStatusCode, &httpDataLen);
                        Sim808UnLock();
                        if (status == 0)
                        {
                           APP_TRACE_INFO(("Post Http Status = %d, data len = %d\r", httpStatusCode, httpDataLen));

                           Sim808Lock();
                           status = SIM808HttpRead(0, httpDataLen);
                           Sim808UnLock();
                        }
                     }
                  }
               }
            }
         }
      }
#endif

      // SIM808 is Hang?
      if (timeoutCount > SIM808_TIMEOUT_HANG_LEVEL)
      {
         static uint16_t hangeCounter = 0;

         hangeCounter++;
         if (hangeCounter < SIM808_MAX_TRY_HANG_COUNT)
         {
            APP_TRACE_INFO(("Module Hang proccess\r"));
            Sim808Lock();
            SIM808SetPowerState(DISABLE);
            osDelay(500);
            SIM808SetPowerState(ENABLE);
            Sim808UnLock();
         }
         else
         {
            hangeCounter = 0;
            APP_TRACE_INFO(("Module Hang proccess hard\r"));
            Sim808Lock();
            SIM808SetPowerState(DISABLE);
            Sim808UnLock();
            osDelay(500);
            NVIC_SystemReset();
         }
      }

      // Check Module Power
      {
         FunctionalState state;

         Sim808Lock();
         state = bspSim808ReadStatus();
         Sim808UnLock();
         if (state == DISABLE)
         {
            //Module is off
            APP_TRACE_INFO(("Module PowerDown Detect and reset mcu\r"));

            NVIC_SystemReset();
         }
      }

      osDelay(1000);
   }
}

/**
  * @brief  
  * @param  
  * @return 
  */
static void Sim808Urc_Thread(void *argument)
{
   osStatus_t status;
   SIM808_Msg_t msg;
   uint32_t result;

   for (;;)
   {
      status = osMessageQueueGet(sim808_messagequeue_id, &msg, NULL, osWaitForever);
      if (status == osOK)
      {
         result = Sim808CheckResponse((char *)&msg.data[0], msg.length);
         osEventFlagsSet(sim808_eventflag_id, result);
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
