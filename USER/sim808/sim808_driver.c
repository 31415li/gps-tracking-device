/**
  ******************************************************************************
  * @file    sim808_driver.c
  * @author  Mahdad Ghasemian
  * @version V0.0.1
  * @date    17-Feb-2018
  * @brief   
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "sim808_driver.h"
#include "cmsis_os2.h" // ::CMSIS:RTOS2
#include "includes.h"
#include "bsp.h"
#include <stdbool.h>
#include <string.h>

/** @defgroup SIM808_DRIVER
  * @brief 
  * @{
  */

/** @defgroup SIM808_DRIVER_Private_Macros
  * @{
  */

//CR 0x0D 13
//LF 0x0A 10

#define S3_character 0x0D
#define S4_character 0x0A

/**
  * @}
  */

/** @defgroup SIM808_DRIVER_Private_Variables
  * @{
  */

GPS_Data_t gpsData;

SIM808_New_Event_t newSmsReceiveFlag = SIM808_READ_EVENT;
SIM808_New_Event_t newTcpReceiveFlag = SIM808_READ_EVENT;
uint8_t timeoutCount;
uint8_t tcpSendFailCounter;

static SIM808_Response_t response;
static char sendBuffer[SIM808_SEND_BUFFER_MAX_SIZE];
static char originatorAddress[SIM808_PHONE_NUMBER_MAX_SIZE];
uint8_t receivePacket[SIM808_RECEIVE_TCP_MAX_SIZE];
uint8_t receivePacket2[SIM808_RECEIVE_TCP_MAX_SIZE];
uint16_t receivePacketLen;
uint16_t receivePacketLen2;
uint8_t httpReadData[SIM808_RECEIVE_HTTP_MAX_SIZE];
uint16_t httpReadLen;
uint8_t httpReadFlag = 0;

static PDP_Context_Status_t pdp1Status;
static GPRS_Attach_Status_t gprsStatus;
static uint8_t signalStrength;
static uint8_t bitErrorRate;
static Sms_Format smsFormat;
static Sms_Receive_Mode_t smsNotificatonMode;
static Sms_Receive_Rules_t smsNotificatonRule;
static GNSS_Power_Mode_t gnssPowerMode;
static char LastNMEASentence[4];
static Network_Reg_URC_Mode_t networkRegistrationMode;
static Network_Reg_Status_t networkRegistrationStatus;
static char apnName[16];
static IPD_Status_t ipdStatus;
static uint8_t cidNumber;
static char ipAddress[SIM808_IP_MAX_SIZE];
static Http_Method_t httpMethod;
static uint16_t httpStatusCode;
static uint16_t httpDataLength;
static uint8_t textSMSParam_FO;
static uint8_t textSMSParam_VP;
static uint8_t textSMSParam_PID;
static uint8_t textSMSParam_DCS;

extern osMutexId_t sim808_mutex_id;
extern osMemoryPoolId_t sim808_mempool_id;
extern osMessageQueueId_t sim808_messagequeue_id;
extern osEventFlagsId_t sim808_eventflag_id;
extern osEventFlagsId_t app_eventflag_id;

/**
  * @}
  */

/** @defgroup SIM808_DRIVER_Static_Functions
  * @{
  */

static void Sim808Execute(const char *command,
                          int resp,
                          int timeout);
static int16_t Sim808SendCommand(const char *command);
static void Sim808SendBinary(uint8_t *ptr, uint16_t length);
static void Sim808SendBinaryWithCtrlZ(uint8_t *ptr, uint16_t length);
static void Sim808WaitForResponse(uint32_t resp, uint32_t timeout);

static void SIM808Init(void);

static int16_t GsmStaffing(uint8_t *inData,
                           uint16_t inLen,
                           uint8_t *outData,
                           uint16_t *outLen,
                           uint16_t max);
static int16_t GsmDestuffing(uint8_t *inPtr,
                             uint16_t inLen,
                             uint8_t *outPtr,
                             uint16_t *outLen,
                             uint16_t max);

/**
  * @}
  */

/** @defgroup SIM808_DRIVER_Private_Functions
  * @{
  */

/**
  * @brief  Get TA Revision Identification of Software Release
  * @param  
  * @return 
  */
int16_t SIM808GetTASoftwareRelease(char *version)
{
   Sim808Execute("AT+CGMR",
                 SIM808_RESULT_FLAG_OK | SIM808_RESULT_FLAG_ERROR,
                 100);

   if (response.type == SIM808_RESULT_FLAG_OK)
   {
      sscanf((char *)&response.text, "Revision:%64s", version);
      return 0;
   }

   return -1;
}

/**
  * @brief  Get IMEI
  * @param  
  * @return 
  */
int16_t SIM808GetIMEI(char *IMEI)
{
   Sim808Execute("AT+CGSN",
                 SIM808_RESULT_FLAG_OK | SIM808_RESULT_FLAG_ERROR,
                 100);

   if (response.type == SIM808_RESULT_FLAG_OK)
   {
      sscanf((char *)&response.text, "%16s", IMEI);
      return 0;
   }

   return -1;
}

/**
  * @brief  Get Signal Strengt
  * @param  
  * @return 
  */
int16_t SIM808GetSignalStrength(uint8_t *rssi, uint8_t *ber)
{

   signalStrength = 0;
   bitErrorRate = 0;

   Sim808Execute("AT+CSQ",
                 SIM808_RESULT_FLAG_OK | SIM808_RESULT_FLAG_ERROR,
                 100);

   if (response.type == SIM808_RESULT_FLAG_OK)
   {
      *rssi = signalStrength;
      *ber = bitErrorRate;
      return 0;
   }

   return -1;
}

/**
  * @brief  Set Eco State
  * @param  
  * @return 
  */
int16_t SIM808SetEcoState(FunctionalState state)
{
   char temp[16];

   if (state == ENABLE)
      sprintf(temp, "ATE1");
   else
      sprintf(temp, "ATE0");

   Sim808Execute(temp,
                 SIM808_RESULT_FLAG_OK | SIM808_RESULT_FLAG_ERROR,
                 100);

   if (response.type == SIM808_RESULT_FLAG_OK)
      return 0;

   return -1;
}

/**
  * @brief  Check Eco State
  * @param  
  * @return 
  */
int16_t SIM808CheckEcoState(FunctionalState *state)
{
   Sim808Execute("AT",
                 SIM808_RESULT_FLAG_TEXT,
                 100);

   if (response.type == SIM808_RESULT_FLAG_TEXT)
   {
      *state = ENABLE;
      return 0;
   }
   else
   {
      *state = DISABLE;
      return 0;
   }

   return -1;
}

/**
  * @brief  Call to dial number
  * @param  
  * @return 
  */
int16_t SIM808CallToDialNumber(char *dialingNumber)
{
   char temp[32];

   snprintf(temp, sizeof(temp), "ATD%s;", dialingNumber);

   Sim808Execute(temp,
                 SIM808_RESULT_FLAG_OK |
                     SIM808_RESULT_FLAG_DIAL_ERROR |
                     SIM808_RESULT_FLAG_ERROR,
                 10000);

   if (response.type == SIM808_RESULT_FLAG_OK)
   {
      return 0;
   }

   return -1;
}

/**
  * @brief  Disconnect existing connection
  * @param  
  * @return 
  */
int16_t SIM808DisconnectCall(void)
{

   Sim808Execute("ATH",
                 SIM808_RESULT_FLAG_OK |
                     SIM808_RESULT_FLAG_ERROR,
                 10000);

   if (response.type == SIM808_RESULT_FLAG_OK)
   {
      return 0;
   }

   return -1;
}

/**
  * @brief  Get Sms Format
  * @param  
  * @return 
  */
int16_t SIM808GetSmsFormat(Sms_Format *mode)
{

   smsFormat = SMS_FORMAT_MODE_PDU;

   Sim808Execute("AT+CMGF?",
                 SIM808_RESULT_FLAG_OK | SIM808_RESULT_FLAG_ERROR,
                 100);

   if (response.type == SIM808_RESULT_FLAG_OK)
   {
      *mode = smsFormat;
      return 0;
   }

   return -1;
}

/**
  * @brief  Get SMS Text Mode Parameter
  * @param  
  * @return 
  */
int16_t SIM808GetSmsTextModeParameter(uint8_t *fo, uint8_t *vp, uint8_t *pid, uint8_t *dcs)
{

   if (SIM808SetSmsFormat(SMS_FORMAT_MODE_TEXT) == 0)
   {
      Sim808Execute("AT+CSMP?",
                    SIM808_RESULT_FLAG_OK | SIM808_RESULT_FLAG_ERROR,
                    500);

      if (response.type == SIM808_RESULT_FLAG_OK)
      {
         *fo = textSMSParam_FO;
         *vp = textSMSParam_VP;
         *pid = textSMSParam_PID;
         *dcs = textSMSParam_DCS;
         return 0;
      }
   }

   return -1;
}

/**
  * @brief  Set SMS Text Mode Parameter
  * @param  
  * @return 
  */
int16_t SIM808SetSmsTextModeParameter(uint8_t fo, uint8_t vp, uint8_t pid, uint8_t dcs)
{
   //AT+CSMP=17,167,0,0 En sms
   //AT+CSMP=17,167,0,8 Farsi sms

   char temp[32];

   snprintf(temp, sizeof(temp), "AT+CSMP=%d,%d,%d,%d", fo, vp, pid, dcs);

   Sim808Execute(temp,
                 SIM808_RESULT_FLAG_OK | SIM808_RESULT_FLAG_ERROR,
                 500);

   if (response.type == SIM808_RESULT_FLAG_OK)
   {
      return 0;
   }

   return -1;
}

/**
  * @brief  Set Sms Format
  * @param  
  * @return 
  */
int16_t SIM808SetSmsFormat(Sms_Format mode)
{
   char temp[16];

   sprintf(temp, "AT+CMGF=%d", mode);

   Sim808Execute(temp,
                 SIM808_RESULT_FLAG_OK | SIM808_RESULT_FLAG_ERROR,
                 100);

   if (response.type == SIM808_RESULT_FLAG_OK)
      return 0;

   return -1;
}

/**
  * @brief  Get Sms Notification
  * @param  
  * @return 
  */
int16_t SIM808GetSmsNotification(Sms_Receive_Mode_t *mode, Sms_Receive_Rules_t *rule)
{

   smsNotificatonMode = SMS_RECEIVE_MODE_0;
   smsNotificatonRule = SMS_RECEIVE_RULE_0;

   if (SIM808SetSmsFormat(SMS_FORMAT_MODE_TEXT) == 0)
   {
      Sim808Execute("AT+CNMI?",
                    SIM808_RESULT_FLAG_OK | SIM808_RESULT_FLAG_ERROR,
                    100);

      if (response.type == SIM808_RESULT_FLAG_OK)
      {
         *mode = smsNotificatonMode;
         *rule = smsNotificatonRule;
         return 0;
      }
   }

   return -1;
}

/**
  * @brief  Set Sms Notification
  * @param  
  * @return 
  */
int16_t SIM808SetSmsNotification(Sms_Receive_Mode_t mode, Sms_Receive_Rules_t rule)
{
   char temp[16];

   if (SIM808SetSmsFormat(SMS_FORMAT_MODE_TEXT) == 0)
   {
      sprintf(temp, "AT+CNMI=%d,%d", mode, rule);

      Sim808Execute(temp,
                    SIM808_RESULT_FLAG_OK | SIM808_RESULT_FLAG_ERROR,
                    100);

      if (response.type == SIM808_RESULT_FLAG_OK)
         return 0;
   }

   return -1;
}

/**
  * @brief  Sms Delete
  * @param  
  * @return 
  */
int16_t SIM808SmsDelete(uint8_t index, Sms_Delete_Flag_t mode)
{
   char temp[16];

   sprintf(temp, "AT+CMGD=%d,%d", index, mode);

   Sim808Execute(temp,
                 SIM808_RESULT_FLAG_OK | SIM808_RESULT_FLAG_ERROR,
                 5000);

   if (response.type == SIM808_RESULT_FLAG_OK)
      return 0;

   return -1;
}

/**
  * @brief  Sms Read
  * @param  
  * @return 
  */
int16_t SIM808SmsRead(uint8_t index, Sms_Read_Mode_t mode, char *phoneNumber, char *text)
{
   char temp[16];

   originatorAddress[0] = '\0';

   sprintf(temp, "AT+CMGR=%d,%d", index, mode);

   Sim808Execute(temp,
                 SIM808_RESULT_FLAG_OK | SIM808_RESULT_FLAG_ERROR,
                 6000);

   if (response.type == SIM808_RESULT_FLAG_OK)
   {
      char *token;

      // Text
      sprintf(temp, "%%%ds", SIM808_RECEIVE_SMS_MAX_SIZE);
      sscanf((char *)&response.text, temp, text);

      // Number
      if (originatorAddress[0] != '\0')
      {
         snprintf(phoneNumber, SIM808_PHONE_NUMBER_MAX_SIZE, "%s", originatorAddress);
         return 0;
      }
   }

   return -1;
}

/**
  * @brief  Sms Send
  * @param  
  * @return 
  */
int16_t SIM808SmsSend(char *destinationAddress, char *text)
{
   static char address[SIM808_PHONE_NUMBER_MAX_SIZE + 16];
   static char message[SIM808_SEND_SMS_MAX_SIZE];

   snprintf(address, SIM808_PHONE_NUMBER_MAX_SIZE + 16, "AT+CMGS=\"%s\"", destinationAddress);
   snprintf(message, SIM808_SEND_SMS_MAX_SIZE, "%s", text);

   if (strlen((char *)&message) == 0)
      return -1;

   Sim808Execute(address,
                 SIM808_RESULT_FLAG_OK |
                     SIM808_RESULT_FLAG_ERROR |
                     SIM808_RESULT_FLAG_TEXT,
                 10000);

   if (response.type == SIM808_RESULT_FLAG_TEXT)
   {
      if (memcmp(response.text, ">", 1) == 0)
      {
         Sim808SendBinaryWithCtrlZ((uint8_t *)&message, strlen((char *)&message));

         Sim808WaitForResponse(SIM808_RESULT_FLAG_OK |
                                   SIM808_RESULT_FLAG_ERROR,
                               70000);
         if (response.type == SIM808_RESULT_FLAG_OK)
         {
            return 0;
         }
      }
   }

   return -1;
}

/**
  * @brief  Get GNSS Power MODE
  * @param  
  * @return 
  */
int16_t SIM808GetGNSSPowerMode(GNSS_Power_Mode_t *mode)
{

   gnssPowerMode = GNSS_POWER_MODE_TURN_OFF;

   Sim808Execute("AT+CGNSPWR?",
                 SIM808_RESULT_FLAG_OK | SIM808_RESULT_FLAG_ERROR,
                 100);

   if (response.type == SIM808_RESULT_FLAG_OK)
   {
      *mode = gnssPowerMode;
      return 0;
   }

   return -1;
}

/**
  * @brief  Set GNSS Power MODE
  * @param  
  * @return 
  */
int16_t SIM808SetGNSSPowerMode(GNSS_Power_Mode_t mode)
{
   char temp[16];

   sprintf(temp, "AT+CGNSPWR=%d", mode);

   Sim808Execute(temp,
                 SIM808_RESULT_FLAG_OK | SIM808_RESULT_FLAG_ERROR,
                 100);

   if (response.type == SIM808_RESULT_FLAG_OK)
      return 0;

   return -1;
}

/**
  * @brief  Get Last NMEA sentence that parsed
  * @param  
  * @return 
  */
int16_t SIM808GetLastParsedNMEA(char *sentence)
{

   LastNMEASentence[0] = '\0';

   Sim808Execute("AT+CGNSSEQ?",
                 SIM808_RESULT_FLAG_OK | SIM808_RESULT_FLAG_ERROR,
                 100);

   if (response.type == SIM808_RESULT_FLAG_OK)
   {
      snprintf(sentence, 4, "%s", LastNMEASentence);
      return 0;
   }
   else
   {
      *sentence = '\0';
      return 0;
   }

   return -1;
}

/**
  * @brief  Define Last NMEA sentence that parsed
  * @param  
  * @return 
  */
int16_t SIM808DefineLastParsedNMEA(char *sentence)
{
   char temp[16];

   sprintf(temp, "AT+CGNSSEQ=%s", sentence);

   Sim808Execute(temp,
                 SIM808_RESULT_FLAG_OK | SIM808_RESULT_FLAG_ERROR,
                 100);

   if (response.type == SIM808_RESULT_FLAG_OK)
      return 0;

   return -1;
}

/**
  * @brief  Read GNSS NMEA parameter
  * @param  
  * @return 
  */
int16_t SIM808ReadGNSSNMEAParameter(GNSS_Run_Status_t *run,
                                    GNSS_Fix_Status_t *fix,
                                    uint16_t *year,
                                    uint8_t *month,
                                    uint8_t *day,
                                    uint8_t *hour,
                                    uint8_t *min,
                                    uint8_t *sec,
                                    uint32_t *latitude,
                                    uint32_t *longitude,
                                    uint16_t *speed,
                                    uint8_t *fixMode)
{

   Sim808Execute("AT+CGNSINF",
                 SIM808_RESULT_FLAG_OK | SIM808_RESULT_FLAG_ERROR,
                 500);

   if (response.type == SIM808_RESULT_FLAG_OK)
   {
      *run = gpsData.run;
      *fix = gpsData.fix;
      *year = gpsData.year;
      *month = gpsData.month;
      *day = gpsData.day;
      *hour = gpsData.hour;
      *min = gpsData.min;
      *sec = gpsData.sec;
      *latitude = gpsData.latitude;
      *longitude = gpsData.longitude;
      *speed = gpsData.speed;
      *fixMode = gpsData.fixMode;

      return 0;
   }

   return -1;
}

/**
  * @brief  Get GNSS NMEA parameter
  * @param  
  * @return 
  */
int16_t SIM808GetGNSSNMEAParameter(void)
{

   Sim808Execute("AT+CGNSINF",
                 SIM808_RESULT_FLAG_OK | SIM808_RESULT_FLAG_ERROR,
                 500);

   if (response.type == SIM808_RESULT_FLAG_OK)
      return 0;

   return -1;
}

/**
  * @brief  Get Network Registration Status
  * @param  
  * @return 
  */
int16_t SIM808GetNetworkRegistrationStatus(Network_Reg_URC_Mode_t *mode,
                                           Network_Reg_Status_t *status)
{

   networkRegistrationMode = NETWORK_REGISTRATION_URC_DISABLE;
   networkRegistrationStatus = NETWORK_NOT_REGISTERED_NOT_TRY;

   Sim808Execute("AT+CREG?",
                 SIM808_RESULT_FLAG_OK | SIM808_RESULT_FLAG_ERROR,
                 100);

   if (response.type == SIM808_RESULT_FLAG_OK)
   {
      *mode = networkRegistrationMode;
      *status = networkRegistrationStatus;
      return 0;
   }

   return -1;
}

/**
  * @brief  Get GPRS Attach Status
  * @param  
  * @return 
  */
int16_t SIM808GetGPRSAttachStatus(GPRS_Attach_Status_t *status)
{

   gprsStatus = GPRS_DETACHED;

   Sim808Execute("AT+CGATT?",
                 SIM808_RESULT_FLAG_OK | SIM808_RESULT_FLAG_ERROR,
                 10000); // 75000 );

   if (response.type == SIM808_RESULT_FLAG_OK)
   {
      *status = gprsStatus;
      return 0;
   }

   return -1;
}

/**
  * @brief  Set GPRS Attach Status
  * @param  
  * @return 
  */
int16_t SIM808SetGPRSAttachStatus(GPRS_Attach_Status_t status)
{
   char temp[16];

   snprintf(temp, 16, "AT+CGATT=%d", status);

   Sim808Execute(temp,
                 SIM808_RESULT_FLAG_OK | SIM808_RESULT_FLAG_ERROR,
                 10000); // 75000 );

   if (response.type == SIM808_RESULT_FLAG_OK)
      return 0;

   return -1;
}

/**
  * @brief  Get APN Name
  * @param  
  * @return 
  */
int16_t SIM808GetApnName(char *apn)
{

   apn[0] = '\0';

   Sim808Execute("AT+CSTT?",
                 SIM808_RESULT_FLAG_OK | SIM808_RESULT_FLAG_ERROR,
                 100);

   if (response.type == SIM808_RESULT_FLAG_OK)
   {
      snprintf(apn, 16, "%s", apnName);
      return 0;
   }

   return -1;
}

/**
  * @brief  Set APN Name And Start Task
  * @param  
  * @return 
  */
int16_t SIM808SetApnNameAndStartTask(char *apnName)
{
   int16_t status;
   char temp[32];

   //   status = SIM808GetApnName(temp);
   //   if (status == 0 && strncmp(apnName, temp, 32) == 0) {
   //      return 0;
   //   } else {
   snprintf(temp, 32, "AT+CSTT=\"%s\"", apnName);

   Sim808Execute(temp,
                 SIM808_RESULT_FLAG_OK | SIM808_RESULT_FLAG_ERROR,
                 100);

   if (response.type == SIM808_RESULT_FLAG_OK)
      return 0;
   //   }

   return -1;
}

/**
  * @brief  Bring Up Wireless Connection
  * @param  
  * @return 
  */
int16_t SIM808BringUpWireless(void)
{
   Sim808Execute("AT+CIICR",
                 SIM808_RESULT_FLAG_OK | SIM808_RESULT_FLAG_ERROR,
                 10000); // 85000 );

   if (response.type == SIM808_RESULT_FLAG_OK)
      return 0;

   return -1;
}

/**
  * @brief  Get Local IP Address
  * @param  
  * @return 
  */
int16_t SIM808GetLocalIPAddress(char *ip)
{
   char format[16];

   Sim808Execute("AT+CIFSR",
                 SIM808_RESULT_FLAG_TEXT | SIM808_RESULT_FLAG_ERROR,
                 100);

   if (response.type == SIM808_RESULT_FLAG_TEXT)
   {
      sprintf(format, "%%%ds", SIM808_IP_MAX_SIZE);
      sscanf((char *)&response.text, format, ip);
      return 0;
   }

   return -1;
}

/**
  * @brief  Start TCP Connection
  * @param  
  * @return 
  */
int16_t SIM808StartTCP(char *serverIP, char *serverPort, Cip_Status_t *status)
{
   char temp[48];

   snprintf(temp, 48, "AT+CIPSTART=\"TCP\",\"%s\",\"%s\"", serverIP, serverPort);

   Sim808Execute(temp,
                 SIM808_RESULT_FLAG_ALREADY_CONNECT |
                     SIM808_RESULT_FLAG_CONNECT_OK |
                     SIM808_RESULT_FLAG_CONNECT_FAIL |
                     SIM808_RESULT_FLAG_ERROR,
                 10000); //170000 );

   if (response.type == SIM808_RESULT_FLAG_ALREADY_CONNECT)
   {
      *status = CIP_STATUS_ALREADY_CONNECT;
      return 0;
   }
   else if (response.type == SIM808_RESULT_FLAG_CONNECT_OK)
   {
      *status = CIP_STATUS_CONNECT_OK;
      return 0;
   }
   else if (response.type == SIM808_RESULT_FLAG_CONNECT_FAIL)
   {
      *status = CIP_STATUS_CONNECT_FAIL;
      return 0;
   }

   return -1;
}

/**
  * @brief  Get Connection Status
  * @param  
  * @return 
  */
int16_t SIM808GetConnectionStatus(Connection_Status_t *status)
{
   char format[16];

   Sim808Execute("AT+CIPSTATUS",
                 SIM808_RESULT_FLAG_IP_INITIAL |
                     SIM808_RESULT_FLAG_IP_START |
                     SIM808_RESULT_FLAG_IP_CONFIG |
                     SIM808_RESULT_FLAG_IP_GPRSACT |
                     SIM808_RESULT_FLAG_IP_STATUS |
                     SIM808_RESULT_FLAG_TCP_CONNECTING |
                     SIM808_RESULT_FLAG_CONNECT_OK |
                     SIM808_RESULT_FLAG_TCP_CLOSING |
                     SIM808_RESULT_FLAG_TCP_CLOSED |
                     SIM808_RESULT_FLAG_PDP_DEACT |
                     SIM808_RESULT_FLAG_ERROR,
                 100);

   if (response.type == SIM808_RESULT_FLAG_IP_INITIAL)
   {
      *status = IP_INITIAL;
      return 0;
   }
   else if (response.type == SIM808_RESULT_FLAG_IP_START)
   {
      *status = IP_START;
      return 0;
   }
   else if (response.type == SIM808_RESULT_FLAG_IP_CONFIG)
   {
      *status = IP_CONFIG;
      return 0;
   }
   else if (response.type == SIM808_RESULT_FLAG_IP_GPRSACT)
   {
      *status = IP_GPRSACT;
      return 0;
   }
   else if (response.type == SIM808_RESULT_FLAG_IP_STATUS)
   {
      *status = IP_STATUS;
      return 0;
   }
   else if (response.type == SIM808_RESULT_FLAG_TCP_CONNECTING)
   {
      *status = TCP_CONNECTING;
      return 0;
   }
   else if (response.type == SIM808_RESULT_FLAG_CONNECT_OK)
   {
      *status = CONNECT_OK;
      return 0;
   }
   else if (response.type == SIM808_RESULT_FLAG_TCP_CLOSING)
   {
      *status = TCP_CLOSING;
      return 0;
   }
   else if (response.type == SIM808_RESULT_FLAG_TCP_CLOSED)
   {
      *status = TCP_CLOSED;
      return 0;
   }
   else if (response.type == SIM808_RESULT_FLAG_PDP_DEACT)
   {
      *status = PDP_DEACT;
      return 0;
   }

   return -1;
}

/**
  * @brief  Deactive GPRS PDP
  * @param  
  * @return 
  */
int16_t SIM808DeactiveGPRSPDP(void)
{

   Sim808Execute("AT+CIPSHUT",
                 SIM808_RESULT_FLAG_SHUT_OK |
                     SIM808_RESULT_FLAG_ERROR,
                 10000); // 75000 );

   if (response.type == SIM808_RESULT_FLAG_SHUT_OK)
   {
      return 0;
   }

   return -1;
}

/**
  * @brief  Get PDP Context Status
  * @param  
  * @return 
  */
int16_t SIM808GetPDPContextStatus(PDP_Context_Status_t *pdpStatus)
{

   pdp1Status = PDP_DEACTIVATED;

   Sim808Execute("AT+CGACT?",
                 SIM808_RESULT_FLAG_OK | SIM808_RESULT_FLAG_ERROR,
                 10000); // 150000 );

   if (response.type == SIM808_RESULT_FLAG_OK)
   {
      *pdpStatus = pdp1Status;
      return 0;
   }

   return -1;
}

/**
  * @brief  Set PDP Context Status
  * @param  
  * @return 
  */
int16_t SIM808SetPDPContextStatus(PDP_Context_Status_t pdpStatus)
{
   char temp[16];

   snprintf(temp, 16, "AT+CGACT=%d", pdpStatus);

   Sim808Execute(temp,
                 SIM808_RESULT_FLAG_OK | SIM808_RESULT_FLAG_ERROR,
                 10000); // 150000 );

   if (response.type == SIM808_RESULT_FLAG_OK)
      return 0;

   return -1;
}

/**
  * @brief  Define PDP Context (+IPD)
  * @param  
  * @return 
  */
int16_t SIM808DefinePDPContext(char *apn)
{
   char temp[32];

   snprintf(temp, 32, "AT+CGDCONT=1,\"IP\",\"%s\"", apn);

   Sim808Execute(temp,
                 SIM808_RESULT_FLAG_OK | SIM808_RESULT_FLAG_ERROR,
                 200);

   if (response.type == SIM808_RESULT_FLAG_OK)
      return 0;

   return -1;
}

/**
  * @brief  Get Head At Packet Received Status (+IPD)
  * @param  
  * @return 
  */
int16_t SIM808GetIPDStatus(IPD_Status_t *status)
{

   ipdStatus = HEAD_PACKET_NONE;

   Sim808Execute("AT+CIPHEAD?",
                 SIM808_RESULT_FLAG_OK | SIM808_RESULT_FLAG_ERROR,
                 200);

   if (response.type == SIM808_RESULT_FLAG_OK)
   {
      *status = ipdStatus;
      return 0;
   }

   return -1;
}

/**
  * @brief  Define Head At Packet Received
  * @param  
  * @return 
  */
int16_t SIM808DefineIPDStatus(IPD_Status_t status)
{
   char temp[32];

   snprintf(temp, 32, "AT+CIPHEAD=%d", status);

   Sim808Execute(temp,
                 SIM808_RESULT_FLAG_OK | SIM808_RESULT_FLAG_ERROR,
                 100);

   if (response.type == SIM808_RESULT_FLAG_OK)
   {
      tcpSendFailCounter = 0;
      return 0;
   }
   else
   {
      tcpSendFailCounter++;
   }

   return -1;
}

/**
  * @brief  Sms Tcp Send
  * @param  
  * @return 
  */
int16_t SIM808TcpSend(uint8_t *data, uint16_t length)
{
   static uint8_t message[SIM808_SEND_TCP_MAX_SIZE];
   uint16_t messageLength;
   int16_t status;

   if (length == 0)
      return -1;

   // GSM Staffing
   status = GsmStaffing(data,
                        length,
                        message,
                        &messageLength,
                        SIM808_SEND_TCP_MAX_SIZE);
   if (status != 0)
      return -1;

   Sim808Execute("AT+CIPSEND",
                 SIM808_RESULT_FLAG_OK |
                     SIM808_RESULT_FLAG_ERROR |
                     SIM808_RESULT_FLAG_TEXT,
                 10000);

   if (response.type == SIM808_RESULT_FLAG_TEXT)
   {
      if (memcmp(response.text, ">", 1) == 0)
      {
         Sim808SendBinaryWithCtrlZ(message, messageLength);
         Sim808WaitForResponse(SIM808_RESULT_FLAG_SEND_OK |
                                   SIM808_RESULT_FLAG_ERROR,
                               10000);
         if (response.type == SIM808_RESULT_FLAG_SEND_OK)
         {
            return 0;
         }
      }
   }

   return -1;
}

/**
  * @brief  Tcp Close
  * @param  
  * @return 
  */
int16_t SIM808TcpClose(void)
{

   Sim808Execute("AT+CIPCLOSE",
                 SIM808_RESULT_FLAG_TEXT | SIM808_RESULT_FLAG_ERROR,
                 10000);

   if (response.type == SIM808_RESULT_FLAG_TEXT)
   {
      if (memcmp(response.text, "CLOSE OK", 8) == 0)
         return 0;
   }

   return -1;
}

/**
  * @brief  Set Bearer Internet Type
  * @param  
  * @return 
  */
int16_t SIM808SetBearerInternetType(uint8_t cid, Bearer_Con_type_t type)
{
   char temp[32];

   snprintf(temp,
            32,
            "AT+SAPBR=3,%d,\"Contype\",\"%s\"",
            cid,
            (type == CSD_CONNECTION ? "CSD" : "GPRS"));

   Sim808Execute(temp,
                 SIM808_RESULT_FLAG_OK | SIM808_RESULT_FLAG_ERROR,
                 200);

   if (response.type == SIM808_RESULT_FLAG_OK)
      return 0;

   return -1;
}

/**
  * @brief  Set Bearer Apn Name
  * @param  
  * @return 
  */
int16_t SIM808SetBearerApnName(uint8_t cid, char *apn)
{
   char temp[48];

   snprintf(temp,
            48,
            "AT+SAPBR=3,%d,\"APN\",\"%s\"",
            cid,
            apn);

   Sim808Execute(temp,
                 SIM808_RESULT_FLAG_OK | SIM808_RESULT_FLAG_ERROR,
                 200);

   if (response.type == SIM808_RESULT_FLAG_OK)
      return 0;

   return -1;
}

/**
  * @brief  Bearer Close
  * @param  
  * @return 
  */
int16_t SIM808BearerClose(uint8_t cid)
{
   char temp[32];

   snprintf(temp,
            32,
            "AT+SAPBR=0,%d",
            cid);

   Sim808Execute(temp,
                 SIM808_RESULT_FLAG_OK | SIM808_RESULT_FLAG_ERROR,
                 10000); // 75000 );

   if (response.type == SIM808_RESULT_FLAG_OK)
      return 0;

   return -1;
}

/**
  * @brief  Bearer Open
  * @param  
  * @return 
  */
int16_t SIM808BearerOpen(uint8_t cid)
{
   char temp[32];

   snprintf(temp,
            32,
            "AT+SAPBR=1,%d",
            cid);

   Sim808Execute(temp,
                 SIM808_RESULT_FLAG_OK | SIM808_RESULT_FLAG_ERROR,
                 10000); // 95000 );

   if (response.type == SIM808_RESULT_FLAG_OK)
      return 0;

   return -1;
}

/**
  * @brief  Get Bearer Status
  * @param  
  * @return 
  */
int16_t SIM808GetBearerStatus(uint8_t cid, Bearer_Status_t *status, char *ip)
{
   char temp[32];

   snprintf(temp,
            32,
            "AT+SAPBR=2,%d",
            cid);

   cidNumber = 0;

   Sim808Execute(temp,
                 SIM808_RESULT_FLAG_BEARER_IS_CONNECTING |
                     SIM808_RESULT_FLAG_BEARER_IS_CONNECTED |
                     SIM808_RESULT_FLAG_BEARER_IS_CLOSING |
                     SIM808_RESULT_FLAG_BEARER_IS_CLOSED |
                     SIM808_RESULT_FLAG_ERROR,
                 200);

   if (response.type == SIM808_RESULT_FLAG_BEARER_IS_CONNECTING)
   {
      *status = BEARER_IS_CONNECTING;
      if (cid == cidNumber)
      {
         memcpy(ip, ipAddress, SIM808_IP_MAX_SIZE);
         ip[SIM808_IP_MAX_SIZE - 1] = '\0';
         return 0;
      }
   }
   else if (response.type == SIM808_RESULT_FLAG_BEARER_IS_CONNECTED)
   {
      *status = BEARER_IS_CONNECTED;
      if (cid == cidNumber)
      {
         memcpy(ip, ipAddress, SIM808_IP_MAX_SIZE);
         ip[SIM808_IP_MAX_SIZE - 1] = '\0';
         return 0;
      }
   }
   else if (response.type == SIM808_RESULT_FLAG_BEARER_IS_CLOSING)
   {
      *status = BEARER_IS_CLOSING;
      if (cid == cidNumber)
      {
         memcpy(ip, ipAddress, SIM808_IP_MAX_SIZE);
         ip[SIM808_IP_MAX_SIZE - 1] = '\0';
         return 0;
      }
   }
   else if (response.type == SIM808_RESULT_FLAG_BEARER_IS_CLOSED)
   {
      *status = BEARER_IS_CLOSED;
      if (cid == cidNumber)
      {
         memcpy(ip, ipAddress, SIM808_IP_MAX_SIZE);
         ip[SIM808_IP_MAX_SIZE - 1] = '\0';
         return 0;
      }
   }

   return -1;
}

/**
  * @brief  Query IP Address From Domain
  * @param  
  * @return 
  */
int16_t SIM808QueryIPAddressFromDomain(char *domain, char *ip)
{
   char temp[48];

   if (domain == NULL)
      return -1;

   snprintf(temp,
            48,
            "AT+CDNSGIP=%s",
            domain);

   Sim808Execute(temp,
                 SIM808_RESULT_FLAG_ERROR |
                     SIM808_RESULT_FLAG_DOMAIN_IP,
                 2000);

   if (response.type == SIM808_RESULT_FLAG_DOMAIN_IP)
   {
      memcpy(ip, ipAddress, SIM808_IP_MAX_SIZE);
      ip[SIM808_IP_MAX_SIZE - 1] = '\0';
      return 0;
   }

   return -1;
}

/**
  * @brief  Http Initialize
  * @param  
  * @return 
  */
int16_t SIM808HttpInitialize(void)
{

   Sim808Execute("AT+HTTPINIT",
                 SIM808_RESULT_FLAG_OK | SIM808_RESULT_FLAG_ERROR,
                 200);

   if (response.type == SIM808_RESULT_FLAG_OK)
   {
      return 0;
   }

   return -1;
}

/**
  * @brief  Set Http Cid Number
  * @param  
  * @return 
  */
int16_t SIM808SetHttpCid(uint8_t cid)
{
   char temp[32];

   snprintf(temp,
            32,
            "AT+HTTPPARA=\"CID\",%d",
            cid);

   Sim808Execute(temp,
                 SIM808_RESULT_FLAG_OK | SIM808_RESULT_FLAG_ERROR,
                 100);

   if (response.type == SIM808_RESULT_FLAG_OK)
      return 0;

   return -1;
}

/**
  * @brief  Set Http URL
  * @param  
  * @return 
  */
int16_t SIM808SetHttpURL(char *serverAddress, char *serverPort, char *path)
{
   char temp[64];

   if (path != NULL)
   {
      snprintf(temp,
               64,
               "AT+HTTPPARA=\"URL\","
               "\"http://%s:%s/%s\"",
               serverAddress,
               serverPort,
               path);
   }
   else
   {
      snprintf(temp,
               64,
               "AT+HTTPPARA=\"URL\","
               "\"http://%s:%s\"",
               serverAddress,
               serverPort);
   }

   Sim808Execute(temp,
                 SIM808_RESULT_FLAG_OK | SIM808_RESULT_FLAG_ERROR,
                 100);

   if (response.type == SIM808_RESULT_FLAG_OK)
      return 0;

   return -1;
}

/**
  * @brief  Set Http Cntent-Type
  * @param  
  * @return 
  */
int16_t SIM808SetHttpCntentType(char *content)
{
   char temp[64];

   snprintf(temp,
            64,
            "AT+HTTPPARA=\"CONTENT\",\"%s\"",
            content);

   Sim808Execute(temp,
                 SIM808_RESULT_FLAG_OK | SIM808_RESULT_FLAG_ERROR,
                 100);

   if (response.type == SIM808_RESULT_FLAG_OK)
      return 0;

   return -1;
}

/**
  * @brief  Http Get
  * @param  
  * @return 
  */
int16_t SIM808HttpGet(uint16_t *statusCode, uint16_t *dataLenth)
{

   Sim808Execute("AT+HTTPACTION=0",
                 SIM808_RESULT_FLAG_HTTP_ACTION | SIM808_RESULT_FLAG_ERROR,
                 10000); // 60000 );

   if (response.type == SIM808_RESULT_FLAG_HTTP_ACTION)
   {
      if (httpMethod == HTTP_METHOD_GET)
      {
         *statusCode = httpStatusCode;
         *dataLenth = httpDataLength;
         return 0;
      }
   }

   return -1;
}

/**
  * @brief  Http Read
  * @param  
  * @return 
  */
int16_t SIM808HttpRead(uint16_t startIndex, uint16_t size)
{
   char temp[32];

   if (size == 0)
      return -1;

   snprintf(temp,
            32,
            "AT+HTTPREAD=%d,%d",
            startIndex,
            size);

   httpReadFlag = 0;
   httpReadData[0] = '\0';

   Sim808Execute(temp,
                 SIM808_RESULT_FLAG_OK | SIM808_RESULT_FLAG_ERROR,
                 2000);

   if (response.type == SIM808_RESULT_FLAG_OK)
   {
      return 0;
   }

   return -1;
}

/**
  * @brief  Http Input Data
  * @param  
  * @return 
  */
int16_t SIM808HttpInputData(uint8_t *data, uint16_t length)
{
   char temp[32];

   if (length == 0)
      return -1;

   snprintf(temp,
            32,
            "AT+HTTPDATA=%d,%d",
            length,
            10000);

   Sim808Execute(temp,
                 SIM808_RESULT_FLAG_OK |
                     SIM808_RESULT_FLAG_ERROR |
                     SIM808_RESULT_FLAG_TEXT,
                 500);

   if (response.type == SIM808_RESULT_FLAG_TEXT)
   {
      if (memcmp(response.text, "DOWNLOAD", 8) == 0)
      {
         Sim808SendBinary(data, length);

         Sim808WaitForResponse(SIM808_RESULT_FLAG_OK |
                                   SIM808_RESULT_FLAG_ERROR,
                               10000);
         if (response.type == SIM808_RESULT_FLAG_OK)
         {
            return 0;
         }
      }
   }

   return -1;
}

/**
  * @brief  Http Post
  * @param  
  * @return 
  */
int16_t SIM808HttpPost(uint16_t *statusCode, uint16_t *dataLenth)
{

   Sim808Execute("AT+HTTPACTION=1",
                 SIM808_RESULT_FLAG_HTTP_ACTION | SIM808_RESULT_FLAG_ERROR,
                 10000); // 60000 );

   if (response.type == SIM808_RESULT_FLAG_HTTP_ACTION)
   {
      if (httpMethod == HTTP_METHOD_POST)
      {
         *statusCode = httpStatusCode;
         *dataLenth = httpDataLength;
         return 0;
      }
   }

   return -1;
}

/**
  * @brief  Test
  * @param  
  * @return 
  */
int16_t SIM808Test(void)
{
   Sim808Execute("AT",
                 SIM808_RESULT_FLAG_OK | SIM808_RESULT_FLAG_ERROR,
                 100);

   if (response.type == SIM808_RESULT_FLAG_OK)
      return 0;

   return -1;
}

/**
  * @brief  Test
  * @param  
  * @return 
  */
int16_t SIM808ATTest(char *at, uint32_t timeout)
{
   Sim808Execute(at,
                 SIM808_RESULT_FLAG_OK | SIM808_RESULT_FLAG_ERROR,
                 timeout);

   if (response.type == SIM808_RESULT_FLAG_OK)
      return 0;

   return -1;
}

/**
  * @brief  Set DTE Eco State
  * @param  
  * @return 
  */
int16_t SIM808SetDTEBaudRate(uint32_t baudRate)
{
   char temp[16];

   sprintf(temp, "AT+IPR=%i", baudRate);

   Sim808Execute(temp,
                 SIM808_RESULT_FLAG_OK | SIM808_RESULT_FLAG_ERROR,
                 1000);

   if (response.type == SIM808_RESULT_FLAG_OK)
      return 0;

   return -1;
}

/**
  * @brief  SIM808 Set Power State
  * @param  
  * @return 
  */
int16_t SIM808SetPowerState(FunctionalState state)
{
   uint8_t tryCount = 0;

   if (state == ENABLE)
   {
      if (bspSim808ReadStatus() == DISABLE)
      {
         do
         {
            osDelay(100);
            bspSim808PowerOn();
            osDelay(4000);
            tryCount++;
         } while ((bspSim808ReadStatus() == DISABLE) && (tryCount < SIM808_MAX_TRY_COUNT_FOR_POWER_ON));
         if (tryCount >= SIM808_MAX_TRY_COUNT_FOR_POWER_ON)
            return -1;
      }

      bspSim808Reset();

      osDelay(1000);

      timeoutCount = 0;
      tcpSendFailCounter = 0;
      SIM808Init();

      return 0;
   }
   else
   {
      if (bspSim808ReadStatus() == DISABLE)
      {
         return 0;
      }
      else
      {
         do
         {
            osDelay(100);
            bspSim808PowerOn();
            osDelay(4000);
            tryCount++;
         } while ((bspSim808ReadStatus() == ENABLE) && (tryCount < SIM808_MAX_TRY_COUNT_FOR_POWER_ON));
         if (tryCount >= SIM808_MAX_TRY_COUNT_FOR_POWER_ON)
            return -1;

         return 0;
      }
   }

   return -1;
}

/**
  * @brief  SIM808 Reset
  * @param  
  * @return 
  */
int16_t SIM808Reset(void)
{

   osDelay(100);
   bspSim808Reset();
   osDelay(4000);

   timeoutCount = 0;
   tcpSendFailCounter = 0;
   SIM808Init();

   return 0;
}

/**
  * @brief  SIM808 Init
  * @param  
  * @return 
  */
static void SIM808Init(void)
{
   int16_t status;
   FunctionalState ecoState;

   SIM808SetEcoState(DISABLE);

   /* Eco Off */
   if (SIM808CheckEcoState(&ecoState) == 0 && ecoState == ENABLE)
      SIM808SetEcoState(DISABLE);

   /* Set DTE Baud rate */
   SIM808SetDTEBaudRate(SIM808_BAUDRATE);

   /* Test */
   SIM808Test();

   /* Eco Off */
   if (SIM808CheckEcoState(&ecoState) == 0 && ecoState == ENABLE)
      SIM808SetEcoState(DISABLE);

   // Set SMS Format
   SIM808SetSmsFormat(SMS_FORMAT_MODE_TEXT);
   // Delete all SMS
   SIM808SmsDelete(1, SMS_DELETE_FLAG_4);
   // Set SMS Notification
   SIM808SetSmsNotification(SMS_RECEIVE_MODE_1, SMS_RECEIVE_RULE_0);
   // Set SMS text mode parameter
   SIM808SetSmsTextModeParameter(17, 167, 0, 0);
}

/**
  * @brief  
  * @param  
  * @return 
  */
static void Sim808Execute(const char *command,
                          int resp,
                          int timeout)
{
   if (Sim808SendCommand(command) == 0)
      Sim808WaitForResponse(resp, timeout);
}

/**
  * @brief  
  * @param  
  * @return 
  */
static int16_t Sim808SendCommand(const char *command)
{
#if (S3_character == 0x0D)
   snprintf(sendBuffer, SIM808_SEND_BUFFER_MAX_SIZE, "%s\r", command);
#else
   snprintf(sendBuffer, SIM808_SEND_BUFFER_MAX_SIZE, "%s", command);
#endif
   if (strlen(sendBuffer) == 0)
      return -1;

   APP_TRACE_LOG(("->>> %s\r", sendBuffer));
   osEventFlagsClear(sim808_eventflag_id, 0x7FFFFFFF);
   bspSim808SendBinaray((uint8_t *)&sendBuffer[0], strlen(sendBuffer));

   return 0;
}

/**
  * @brief  
  * @param  
  * @return 
  */
static void Sim808SendBinary(uint8_t *ptr, uint16_t length)
{
   uint16_t len;

   len = (length < SIM808_SEND_BUFFER_MAX_SIZE) ? length : SIM808_SEND_BUFFER_MAX_SIZE;
   memcpy(sendBuffer, ptr, len);
   APP_TRACE_LOG_BINARRAY(((uint8_t *)&sendBuffer[0], len));
   osEventFlagsClear(sim808_eventflag_id, 0x7FFFFFFF);
   bspSim808SendBinaray((uint8_t *)&sendBuffer[0], len);
}

/**
  * @brief  
  * @param  
  * @return 
  */
static void Sim808SendBinaryWithCtrlZ(uint8_t *ptr, uint16_t length)
{
   uint16_t len;
   char ctlZ = 26;
   uint32_t i;

   len = (length < SIM808_SEND_BUFFER_MAX_SIZE) ? length : SIM808_SEND_BUFFER_MAX_SIZE;
   memcpy(sendBuffer, ptr, len);
   APP_TRACE_LOG_BINARRAY(((uint8_t *)&sendBuffer[0], len));
   osEventFlagsClear(sim808_eventflag_id, 0x7FFFFFFF);
   bspSim808SendBinaray((uint8_t *)&sendBuffer[0], len);
   bspSim808SendBinaray((uint8_t *)&ctlZ, 1);
}

/**
  * @brief  
  * @param  ندارد
  * @return ندراد
  */
static void Sim808WaitForResponse(uint32_t resp, uint32_t timeout)
{
   uint32_t type;

   response.text[0] = 0x00;
   response.type = SIM808_RESULT_FLAG_NONE;
   do
   {
      type = osEventFlagsWait(sim808_eventflag_id, resp, osFlagsWaitAny, timeout);

      if (type == osFlagsErrorTimeout)
      {
         timeoutCount++;
         response.type = SIM808_RESULT_FLAG_TIMEOUT;
         break;
      }
      else
      {
         if ((uint32_t)(resp & type) > 0)
         {
            timeoutCount = 0;
            response.type = (uint32_t)(resp & type);
            break;
         }
      }
   } while (1);
}

/**
  * @brief  
  * @param  
  * @return 
  */

uint32_t Sim808CheckResponse(char *pStr, uint16_t length)
{
   uint32_t latValue, lonValue;
   uint32_t value1, value2, value3, value4;
   char *token;

   if (length == 0)
   {
      APP_TRACE_LOG(("<<<- Empty\r"));
      return SIM808_RESULT_FLAG_ANALIZED;
   }

   APP_TRACE_LOG(("<<<- %s\r", pStr));

   if (memcmp(pStr, "OK", 2) == 0)
   {
      return SIM808_RESULT_FLAG_OK;
   }
   else if (memcmp(pStr, "ERROR", 5) == 0)
   {
      return SIM808_RESULT_FLAG_ERROR;
   }
   // Connection State
   else if (memcmp(pStr, "STATE", 5) == 0)
   {
      if (memcmp(pStr, "STATE:", 6) == 0)
      {
         if (memcmp(&pStr[7], "IP INITIAL", 10) == 0)
            return SIM808_RESULT_FLAG_IP_INITIAL;
         else if (memcmp(&pStr[7], "IP START", 8) == 0)
            return SIM808_RESULT_FLAG_IP_START;
         else if (memcmp(&pStr[7], "IP CONFIG", 9) == 0)
            return SIM808_RESULT_FLAG_IP_CONFIG;
         else if (memcmp(&pStr[7], "IP GPRSACT", 10) == 0)
            return SIM808_RESULT_FLAG_IP_GPRSACT;
         else if (memcmp(&pStr[7], "IP STATUS", 9) == 0)
            return SIM808_RESULT_FLAG_IP_STATUS;
         else if (memcmp(&pStr[7], "TCP CONNECTING", 14) == 0)
            return SIM808_RESULT_FLAG_TCP_CONNECTING;
         else if (memcmp(&pStr[7], "CONNECT OK", 10) == 0)
            return SIM808_RESULT_FLAG_CONNECT_OK;
         else if (memcmp(&pStr[7], "TCP CLOSING", 11) == 0)
            return SIM808_RESULT_FLAG_TCP_CLOSING;
         else if (memcmp(&pStr[7], "TCP CLOSED", 10) == 0)
            return SIM808_RESULT_FLAG_TCP_CLOSED;
         else if (memcmp(&pStr[7], "PDP DEACT", 9) == 0)
            return SIM808_RESULT_FLAG_PDP_DEACT;
      }
   }
   // Bearer State
   else if (memcmp(pStr, "+SAPBR", 6) == 0)
   {
      if (strlen(pStr) > 9)
      {
         sscanf(pStr, "+SAPBR:%1d,%1d,", &value1, &value2);
         token = strtokSingle(pStr, "\"");
         if (token != NULL)
         {
            token = strtokSingle(NULL, "\"");
            if (token != NULL)
            {
               snprintf(ipAddress, SIM808_IP_MAX_SIZE, "%s", token);
            }
         }

         cidNumber = value1;

         switch (value2)
         {
         case BEARER_IS_CONNECTING:
            return SIM808_RESULT_FLAG_BEARER_IS_CONNECTING;
            break;
         case BEARER_IS_CONNECTED:
            return SIM808_RESULT_FLAG_BEARER_IS_CONNECTED;
            break;
         case BEARER_IS_CLOSING:
            return SIM808_RESULT_FLAG_BEARER_IS_CLOSING;
            break;
         case BEARER_IS_CLOSED:
            return SIM808_RESULT_FLAG_BEARER_IS_CLOSED;
            break;
         default:
            break;
         }
      }
   }
   // Http Action
   else if (memcmp(pStr, "+HTTPACTION", 11) == 0)
   {
      sscanf(pStr, "+HTTPACTION: %1d,%4d,%6d", &value1, &value2, &value3);
      httpMethod = value1;
      httpStatusCode = value2;
      httpDataLength = value3;
      return SIM808_RESULT_FLAG_HTTP_ACTION;
   }
   // CIP State
   else if (memcmp(pStr, "ALREADY CONNECT", 15) == 0)
   {
      return SIM808_RESULT_FLAG_ALREADY_CONNECT;
   }
   // CIP State
   else if (memcmp(pStr, "CONNECT OK", 10) == 0)
   {
      return SIM808_RESULT_FLAG_CONNECT_OK;
   }
   // CIP State
   else if (memcmp(pStr, "CONNECT FAIL", 12) == 0)
   {
      return SIM808_RESULT_FLAG_CONNECT_FAIL;
   }
   // Tcp Send
   else if (memcmp(pStr, "SEND OK", 7) == 0)
   {
      return SIM808_RESULT_FLAG_SEND_OK;
   }
   // Tcp Shut
   else if (memcmp(pStr, "SHUT OK", 7) == 0)
   {
      return SIM808_RESULT_FLAG_SHUT_OK;
   }
   // Dial
   else if (memcmp(pStr, "NO DIALTONE", 11) == 0)
   {
      return SIM808_RESULT_FLAG_DIAL_ERROR;
   }
   else if (memcmp(pStr, "NO CARRIER", 10) == 0)
   {
      return SIM808_RESULT_FLAG_DIAL_ERROR;
   }
   else if (memcmp(pStr, "NO ANSWER", 9) == 0)
   {
      return SIM808_RESULT_FLAG_DIAL_ERROR;
   }
   else if (memcmp(pStr, "BUSY", 4) == 0)
   {
      return SIM808_RESULT_FLAG_DIAL_ERROR;
   }
   // CME Error
   else if (memcmp(pStr, "+CME", 4) == 0)
   {
      snprintf(response.errorText, SIM808_ERROR_MAX_SIZE, "%s", pStr);
      return SIM808_RESULT_FLAG_ERROR;
   }
   // CMS Error
   else if (memcmp(pStr, "+CMS", 4) == 0)
   {
      snprintf(response.errorText, SIM808_ERROR_MAX_SIZE, "%s", pStr);
      return SIM808_RESULT_FLAG_ERROR;
   }
   // GNSS
   else if (memcmp(pStr, "+CGNSINF", 8) == 0)
   {
      pStr[95] = '\0';
      token = strtokSingle(pStr, " ");
      // gps run status
      token = strtokSingle(NULL, ",");
      if (token != NULL)
      {
         sscanf(token, "%1d", &value1);
         gpsData.run = value1;
      }
      // fix status
      token = strtokSingle(NULL, ",");
      if (token != NULL)
      {
         sscanf(token, "%d", &value1);
         gpsData.fix = value1;
      }
      // UTC date and time
      token = strtokSingle(NULL, ",");
      if (token != NULL)
      {
         sscanf(&token[0], "%4d", &value1);
         gpsData.year = value1;
         sscanf(&token[4], "%2d", &value1);
         gpsData.month = value1;
         sscanf(&token[6], "%2d", &value1);
         gpsData.day = value1;
         sscanf(&token[8], "%2d", &value1);
         gpsData.hour = value1;
         sscanf(&token[10], "%2d", &value1);
         gpsData.min = value1;
         sscanf(&token[12], "%2d", &value1);
         gpsData.sec = value1;
      }
      // Latitude
      token = strtokSingle(NULL, ",");
      if (token != NULL)
      {
         if (*token != 0)
         {
            sscanf(token, "%d.%d", &value1, &value2);
            latValue = (uint32_t)(value1 * 1000000);
            latValue += value2;
            if (gpsData.fix > 0)
               gpsData.latitude = latValue;
         }
      }
      // Longitude
      token = strtokSingle(NULL, ",");
      if (token != NULL)
      {
         if (*token != 0)
         {
            sscanf(token, "%d.%d", &value1, &value2);
            lonValue = (uint32_t)(value1 * 1000000);
            lonValue += value2;
            if (gpsData.fix > 0)
               gpsData.longitude = lonValue;
         }
      }
      token = strtokSingle(NULL, ",");
      // Speed over ground
      token = strtokSingle(NULL, ",");
      if (token != NULL)
      {
         sscanf(token, "%d", &value1);
         gpsData.speed = value1;
      }
      token = strtokSingle(NULL, ",");
      // fix mode
      token = strtokSingle(NULL, ",");
      if (token != NULL)
      {
         sscanf(token, "%1d", &value1);
         gpsData.fixMode = value1;
      }
   }
   // SMS Format
   else if (memcmp(pStr, "+CMGF", 5) == 0)
   {
      sscanf(pStr, "+CMGF: %1i", &value1);
      smsFormat = value1;
   }
   // SMS Notification
   else if (memcmp(pStr, "+CNMI", 5) == 0)
   {
      sscanf(pStr, "+CNMI: %1i,%1i", &value1, &value2);
      smsNotificatonMode = value1;
      smsNotificatonRule = value2;
   }
   // SMS Text mode Parameter
   else if (memcmp(pStr, "+CSMP", 5) == 0)
   {
      sscanf(pStr, "+CSMP: %3i,%3i,%3i,%3i", &value1, &value2, &value3, &value4);
      textSMSParam_FO = value1;
      textSMSParam_VP = value2;
      textSMSParam_PID = value3;
      textSMSParam_DCS = value4;
   }
   // SMS Detail
   else if (memcmp(pStr, "+CMGR", 5) == 0)
   {
      token = strtokSingle(pStr, ",");
      token = strtokSingle(NULL, ",");
      if (token != NULL)
      {
         token = strtokSingle(token + 1, "\"");
         snprintf(originatorAddress, SIM808_PHONE_NUMBER_MAX_SIZE, "%s", token);
      }
   }
   // PDP context status
   else if (memcmp(pStr, "+CGACT", 6) == 0)
   {
      sscanf(pStr, "+CGACT: %1i,%1i", &value1, &value2);
      if (value1 == 1)
         pdp1Status = value2;
   }
   // GPRS status
   else if (memcmp(pStr, "+CGATT", 6) == 0)
   {
      sscanf(pStr, "+CGATT: %1i", &value1);
      gprsStatus = value1;
   }
   // CSQ
   else if (memcmp(pStr, "+CSQ", 4) == 0)
   {
      sscanf(pStr, "+CSQ: %3i,%3i", &value1, &value2);
      signalStrength = value1;
      bitErrorRate = value2;
   }
   // GNSS Power Mode
   else if (memcmp(pStr, "+CGNSPWR", 8) == 0)
   {
      sscanf(pStr, "+CGNSPWR: %1i", &value1);
      gnssPowerMode = value1;
   }
   // Last NMEA Parsed
   else if (memcmp(pStr, "+CGNSREQ", 8) == 0)
   {
      sscanf(pStr, "+CGNSREQ: %3s", LastNMEASentence);
   }
   // Network Registration Status
   else if (memcmp(pStr, "+CREG", 5) == 0)
   {
      sscanf(pStr, "+CREG: %1i,%1i", &value1, &value2);
      networkRegistrationMode = value1;
      networkRegistrationStatus = value2;
   }
   // APN
   else if (memcmp(pStr, "+CSTT", 5) == 0)
   {
      token = strtokSingle(pStr, " ");
      if (token != NULL)
      {
         token = strtokSingle(NULL, ",");
         if (token != NULL)
         {
            token = strtokSingle(token + 1, "\"");
            sscanf(token, "%16s", apnName);
         }
      }
   }
   // +IPD Head Status
   else if (memcmp(pStr, "+CIPHEAD", 8) == 0)
   {
      sscanf(pStr, "+CIPHEAD: %1i", &value1);
      ipdStatus = value1;
   }
   // +IPD Receive Packet
   else if (memcmp(pStr, "+IPD", 4) == 0)
   {
      if (newTcpReceiveFlag == SIM808_READ_EVENT)
      {
         sscanf(pStr, "+IPD,%5i:", &value1);
         receivePacketLen = (value1 < SIM808_RECEIVE_TCP_MAX_SIZE) ? value1 : SIM808_RECEIVE_TCP_MAX_SIZE;
         if (receivePacketLen > 0)
         {
            uint16_t ipdDataIndex;

            if (receivePacketLen < 10)
               ipdDataIndex = 7;
            else if (receivePacketLen < 100)
               ipdDataIndex = 8;
            else if (receivePacketLen < 1000)
               ipdDataIndex = 9;
            else
               ipdDataIndex = 10;

            memcpy(receivePacket, &pStr[ipdDataIndex], receivePacketLen);
            newTcpReceiveFlag = SIM808_NEW_EVENT;
            return SIM808_RESULT_FLAG_RECEIVE_PACKET;
         }
      }
   }
   // HTTP read
   else if (memcmp(pStr, "+HTTPREAD", 9) == 0)
   {
      sscanf(pStr, "+HTTPREAD: %6i", &value1);
      httpReadLen = value1;
      if (httpReadLen > 0)
         httpReadFlag = 1;
   }
   // Domain
   else if (memcmp(pStr, "+CDNSGIP", 8) == 0)
   {
      sscanf(pStr, "+CDNSGIP: %1i", &value1);
      if (value1 == 0)
      {
         return SIM808_RESULT_FLAG_ERROR;
      }
      else
      {
         token = strtokSingle(pStr, ",");
         token = strtokSingle(NULL, ",");
         token = strtokSingle(NULL, "\"");
         token = strtokSingle(NULL, "\"");
         if (token != NULL)
         {
            snprintf(ipAddress, SIM808_IP_MAX_SIZE, "%s", token);
            return SIM808_RESULT_FLAG_DOMAIN_IP;
         }
      }
   }
   // Under Voltage Warning
   else if (memcmp(pStr, "UNDER-VOLTAGE WARNNING", 22) == 0)
   {
      return SIM808_RESULT_FLAG_UNDER_VOLTAGE_WARNNING;
   }
   // Text
   else
   {
      if (httpReadLen > 0)
      {
         memcpy(httpReadData, pStr, httpReadLen);
         snprintf((char *)httpReadData, SIM808_RECEIVE_HTTP_MAX_SIZE, "%s", pStr);
         httpReadFlag = 0;
      }

      if (pStr[0] != '+')
      {
         snprintf(response.text, SIM808_RESPONSE_BUFFER_SIZE, "%s", pStr);
      }
      return SIM808_RESULT_FLAG_TEXT;
   }

   return SIM808_RESULT_FLAG_ANALIZED;
}

/**
  * @brief  
  * @param  
  * @return ندراد
  */
void Sim808CheckReceiveByte(uint8_t byte)
{
   static bool i = false;
   static SIM808_Msg_t msg;

   /* آناليز براي جداسازي متن از فرمت
   <S3><S4>"TEXT"<S3><S4> */
   /* پس از جداسازي متن به کيو پست مي شود */

   if (byte == S3_character)
      i = true;
   else if (byte == S4_character)
   {
      if (i == true & msg.length != 0)
      {
         if (msg.length < SIM808_DATA_BUFFER_SIZE)
         {
            msg.data[msg.length++] = 0;
            if (sim808_messagequeue_id != NULL)
            {
               osMessageQueuePut(sim808_messagequeue_id, &msg, 0, NULL);
            }
         }

         msg.length = 0;
      }
      i = false;
   }
   else if (msg.length == 0 && (byte == '>' || byte == '@'))
   {
      /* براي مدل هايي که انتها ندارند */

      msg.data[msg.length++] = byte;
      msg.data[msg.length++] = 0;

      if (sim808_messagequeue_id != NULL)
      {
         osMessageQueuePut(sim808_messagequeue_id, &msg, 0, NULL);
      }

      msg.length = 0;
      i = false;
   }
   else
   {
      if (msg.length < SIM808_DATA_BUFFER_SIZE)
         msg.data[msg.length++] = byte;
      else
      {
         i = false;
         msg.length = 0;
      }
   }
}

/**
  * @brief	Gsm Stuffing
  * @param  
  * @return 
  */
int16_t Sim808TrueReceiveData(void)
{
   GsmDestuffing(receivePacket, receivePacketLen, receivePacket2, &receivePacketLen2, SIM808_RECEIVE_TCP_MAX_SIZE);

   return 0;
}

/**
  * @brief  
  * @param  ندراد
  * @return ندراد
  */
void Sim808Interrupt(void)
{
   newSmsReceiveFlag = SIM808_NEW_EVENT;
}

/**
  * @brief  
  * @param  ندارد
  * @return ندراد
  */
void Sim808Lock(void)
{
   osStatus_t status;

   if (sim808_mutex_id != NULL)
   {
      status = osMutexAcquire(sim808_mutex_id, osWaitForever);
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
void Sim808UnLock(void)
{
   osStatus_t status;

   if (sim808_mutex_id != NULL)
   {
      status = osMutexRelease(sim808_mutex_id);
      if (status != osOK)
      {
      }
   }
}

/**
  * @brief	Gsm Stuffing
  * @param  
  * @return 
  */
static int16_t GsmStaffing(uint8_t *inData,
                           uint16_t inLen,
                           uint8_t *outData,
                           uint16_t *outLen,
                           uint16_t max)
{
   uint16_t i, j;

   for (j = 0, i = 0; j < inLen; j++)
   {
      if (i >= (max - 1))
         return -1;

      outData[i++] = inData[j];
      if (outData[i - 1] == 0x0D)
      {
         outData[i - 1] = 0x0E;
         outData[i++] = 0x2D;
      }
      else if (outData[i - 1] == 0x0A)
      {
         outData[i - 1] = 0x0E;
         outData[i++] = 0x2A;
      }
      else if (outData[i - 1] == 0x1A)
      {
         outData[i - 1] = 0x0E;
         outData[i++] = 0x2B;
      }
      else if (outData[i - 1] == 0x0E)
      {
         outData[i - 1] = 0x0E;
         outData[i++] = 0x2E;
      }
   }

   *outLen = i;

   return 0;
}

/**
  * @brief	Gsm Destuffing 
  * @param  
  * @return 
  */
static int16_t GsmDestuffing(uint8_t *inPtr,
                             uint16_t inLen,
                             uint8_t *outPtr,
                             uint16_t *outLen,
                             uint16_t max)
{
   volatile uint32_t i, j;

   if (inLen > max)
      return -1;

   for (i = 0, j = 0; j < inLen; i++, j++)
   {
      if (inPtr[j] == 0x0E && inPtr[j + 1] == 0x2D)
      {
         outPtr[i] = 0x0D;
         j++;
      }
      else if (inPtr[j] == 0x0E && inPtr[j + 1] == 0x2A)
      {
         outPtr[i] = 0x0A;
         j++;
      }
      else if (inPtr[j] == 0x0E && inPtr[j + 1] == 0x2B)
      {
         outPtr[i] = 0x1A;
         j++;
      }
      else if (inPtr[j] == 0x0E && inPtr[j + 1] == 0x2E)
      {
         outPtr[i] = 0x0E;
         j++;
      }
      else
         outPtr[i] = inPtr[j];
   }

   *outLen = i;

   return 0;
}

/**
  * @}
  */

/**
  * @}
  */

/*********************************END OF FILE****************************/
