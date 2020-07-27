/**
  ******************************************************************************
  * @file    sim808_driver.h
  * @author  Mahdad Ghasemian
  * @version V0.0.1
  * @date    17-Feb-2018
  * @brief   
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __SIM808_DRIVER_H
#define __SIM808_DRIVER_H

#ifdef __cplusplus
extern "C"
{
#endif

/* Includes ------------------------------------------------------------------*/
#include <stdint.h>
#include <stdio.h>
#include "stm32f10x.h" // Device header
#include "app_cfg.h"

/** @defgroup SIM808_DRIVER 
  * @{
  */

/** @defgroup SIM808_Exported_Macros
  * @{
  */

//-------- <<< Use Configuration Wizard in Context Menu >>> -----------------

// <q> SMS Enable
#define SIM808_SMS_EN 1
// <q> TCP Enable
#define SIM808_TCP_EN 1
// <q> HTPP Enable
#define SIM808_HTTP_EN 0

#define SIM808_SEND_BUFFER_MAX_SIZE 512
#define SIM808_RESPONSE_BUFFER_SIZE 512u
#define SIM808_SEND_TCP_MAX_SIZE 256
#define SIM808_RECEIVE_TCP_MAX_SIZE 256
#define SIM808_RECEIVE_HTTP_MAX_SIZE 256
#define SIM808_TEMP_BUFFER_SIZE 128
#define SIM808_SEND_SMS_MAX_SIZE 128
#define SIM808_RECEIVE_SMS_MAX_SIZE 32
#define SIM808_PHONE_NUMBER_MAX_SIZE 16
#define SIM808_IP_MAX_SIZE 16
#define SIM808_PORT_MAX_SIZE 8

#define SIM808_ERROR_MAX_SIZE 32
#define SIM808_TIMEOUT_HANG_LEVEL 10
#define SIM808_MAX_TRY_HANG_COUNT 10
#define SIM808_MAX_TRY_COUNT_FOR_POWER_ON 5

#define SIM808_RESULT_FLAG_OK 0x00000001
#define SIM808_RESULT_FLAG_ERROR 0x00000002
#define SIM808_RESULT_FLAG_ALREADY_CONNECT 0x00000004
#define SIM808_RESULT_FLAG_CONNECT_OK 0x00000008
#define SIM808_RESULT_FLAG_CONNECT_FAIL 0x00000010
#define SIM808_RESULT_FLAG_SEND_OK 0x00000020
#define SIM808_RESULT_FLAG_SHUT_OK 0x00000040
#define SIM808_RESULT_FLAG_RESERVE_13 0x00000080
#define SIM808_RESULT_FLAG_IP_INITIAL 0x00000100
#define SIM808_RESULT_FLAG_IP_START 0x00000200
#define SIM808_RESULT_FLAG_IP_CONFIG 0x00000400
#define SIM808_RESULT_FLAG_IP_GPRSACT 0x00000800
#define SIM808_RESULT_FLAG_IP_STATUS 0x00001000
#define SIM808_RESULT_FLAG_TCP_CONNECTING 0x00002000
#define SIM808_RESULT_FLAG_TCP_CLOSING 0x00004000
#define SIM808_RESULT_FLAG_TCP_CLOSED 0x00008000
#define SIM808_RESULT_FLAG_PDP_DEACT 0x00010000
#define SIM808_RESULT_FLAG_HTTP_ACTION 0x00020000
#define SIM808_RESULT_FLAG_DOMAIN_IP 0x00040000
#define SIM808_RESULT_FLAG_DIAL_ERROR 0x00080000
#define SIM808_RESULT_FLAG_TEXT 0x00100000
#define SIM808_RESULT_FLAG_TIMEOUT 0x00200000
#define SIM808_RESULT_FLAG_RECEIVE_PACKET 0x00400000
#define SIM808_RESULT_FLAG_BEARER_IS_CONNECTING 0x00800000
#define SIM808_RESULT_FLAG_BEARER_IS_CONNECTED 0x01000000
#define SIM808_RESULT_FLAG_BEARER_IS_CLOSING 0x02000000
#define SIM808_RESULT_FLAG_BEARER_IS_CLOSED 0x04000000
#define SIM808_RESULT_FLAG_UNDER_VOLTAGE_WARNNING 0x08000000
#define SIM808_RESULT_FLAG_RESERVE_3 0x10000000
#define SIM808_RESULT_FLAG_RESERVE_2 0x20000000
#define SIM808_RESULT_FLAG_ANALIZED 0x40000000
#define SIM808_RESULT_FLAG_NONE 0x80000000

   /**
  * @}
  */

   /** @defgroup SIM808_DRIVER_Exported_Types
  * @{
  */

   typedef enum
   {
      SMS_FORMAT_MODE_PDU = 0,
      SMS_FORMAT_MODE_TEXT
   } Sms_Format;

   typedef enum
   {
      SMS_RECEIVE_MODE_0 = 0,
      SMS_RECEIVE_MODE_1,
      SMS_RECEIVE_MODE_2,
      SMS_RECEIVE_MODE_3
   } Sms_Receive_Mode_t;

   typedef enum
   {
      SMS_RECEIVE_RULE_0 = 0,
      SMS_RECEIVE_RULE_1,
      SMS_RECEIVE_RULE_2,
      SMS_RECEIVE_RULE_3
   } Sms_Receive_Rules_t;

   typedef enum
   {
      SMS_DELETE_FLAG_0 = 0,
      SMS_DELETE_FLAG_1,
      SMS_DELETE_FLAG_2,
      SMS_DELETE_FLAG_3,
      SMS_DELETE_FLAG_4
   } Sms_Delete_Flag_t;

   typedef enum
   {
      SMS_READ_MODE_NORMAL = 0,
      SMS_READ_MODE_NOT_CHANGE
   } Sms_Read_Mode_t;

   typedef enum
   {
      SIM808_READ_EVENT = 0,
      SIM808_NEW_EVENT
   } SIM808_New_Event_t;

   typedef enum
   {
      GNSS_POWER_MODE_TURN_OFF = 0,
      GNSS_POWER_MODE_TURN_ON
   } GNSS_Power_Mode_t;

   typedef enum
   {
      GNSS_RUN_STATUS_NO = 0,
      GNSS_RUN_STATUS_YES
   } GNSS_Run_Status_t;

   typedef enum
   {
      GNSS_FIX_STATUS_NO = 0,
      GNSS_FIX_STATUS_YES
   } GNSS_Fix_Status_t;

   typedef enum
   {
      NETWORK_REGISTRATION_URC_DISABLE = 0,
      NETWORK_REGISTRATION_URC_ENABLE,
      NETWORK_REGISTRATION_URC_ENABLE_WITH_LOC
   } Network_Reg_URC_Mode_t;

   typedef enum
   {
      NETWORK_NOT_REGISTERED_NOT_TRY = 0,
      NETWORK_REGISTERED,
      NETWORK_NOT_REGISTERED_BUT_TRY,
      NETWORK_REGISTRATION_DENIED,
      NETWORK_UNKNOWN,
      NETWORK_REGISTERED_ROAMING
   } Network_Reg_Status_t;

   typedef enum
   {
      GPRS_DETACHED = 0,
      GPRS_ATTACHED
   } GPRS_Attach_Status_t;

   typedef enum
   {
      PDP_DEACTIVATED = 0,
      PDP_ACTIVATED
   } PDP_Context_Status_t;

   typedef enum
   {
      HEAD_PACKET_NONE = 0,
      HEAD_PACKET_IPD
   } IPD_Status_t;

   typedef enum
   {
      IP_INITIAL = 0,
      IP_START,
      IP_CONFIG,
      IP_GPRSACT,
      IP_STATUS,
      TCP_CONNECTING,
      CONNECT_OK,
      TCP_CLOSING,
      TCP_CLOSED,
      PDP_DEACT
   } Connection_Status_t;

   typedef enum
   {
      CIP_STATUS_NONE = 0,
      CIP_STATUS_ALREADY_CONNECT,
      CIP_STATUS_CONNECT_OK,
      CIP_STATUS_CONNECT_FAIL
   } Cip_Status_t;

   typedef enum
   {
      CSD_CONNECTION = 0,
      GPRS_CONNECTION
   } Bearer_Con_type_t;

   typedef enum
   {
      BEARER_IS_CONNECTING = 0,
      BEARER_IS_CONNECTED,
      BEARER_IS_CLOSING,
      BEARER_IS_CLOSED
   } Bearer_Status_t;

   typedef enum
   {
      HTTP_METHOD_GET = 0,
      HTTP_METHOD_POST,
      HTTP_METHOD_HEAD,
      HTTP_METHOD_DELETE
   } Http_Method_t;

   typedef struct
   {
      uint16_t length;
      uint8_t data[SIM808_DATA_BUFFER_SIZE];
   } SIM808_Msg_t;

   typedef struct
   {
      uint32_t type;
      char errorText[SIM808_ERROR_MAX_SIZE];
      char text[SIM808_RESPONSE_BUFFER_SIZE];
   } SIM808_Response_t;

   typedef struct
   {
      GNSS_Power_Mode_t mode;
      GNSS_Run_Status_t run;
      GNSS_Fix_Status_t fix;
      uint16_t year;
      uint8_t month;
      uint8_t day;
      uint8_t hour;
      uint8_t min;
      uint8_t sec;
      uint32_t latitude;
      uint32_t longitude;
      uint32_t height;
      uint8_t hAcc;
      uint8_t vAcc;
      uint16_t speed;
      uint8_t fixMode;
      uint32_t heading;
   } GPS_Data_t;

   /**
  * @}
  */

   /** @defgroup SIM808_DRIVER_Exported_Variables
  * @{
  */

   extern GPS_Data_t gpsData;

   /**
  * @}
  */

   /** @defgroup SIM808_DRIVER_Exported_Functions
  * @{
  */

   int16_t SIM808GetTASoftwareRelease(char *version);
   int16_t SIM808GetIMEI(char *IMEI);
   int16_t SIM808GetSignalStrength(uint8_t *rssi, uint8_t *ber);
   int16_t SIM808SetEcoState(FunctionalState state);
   int16_t SIM808CheckEcoState(FunctionalState *state);
   int16_t SIM808CallToDialNumber(char *dialingNumber);
   int16_t SIM808DisconnectCall(void);
   int16_t SIM808GetSmsFormat(Sms_Format *mode);
   int16_t SIM808GetSmsTextModeParameter(uint8_t *fo, uint8_t *vp, uint8_t *pid, uint8_t *dcs);
   int16_t SIM808SetSmsTextModeParameter(uint8_t fo, uint8_t vp, uint8_t pid, uint8_t dcs);
   int16_t SIM808SetSmsFormat(Sms_Format mode);
   int16_t SIM808GetSmsNotification(Sms_Receive_Mode_t *mode, Sms_Receive_Rules_t *rule);
   int16_t SIM808SetSmsNotification(Sms_Receive_Mode_t mode, Sms_Receive_Rules_t rule);
   int16_t SIM808SmsDelete(uint8_t index, Sms_Delete_Flag_t mode);
   int16_t SIM808SmsRead(uint8_t index, Sms_Read_Mode_t mode, char *phoneNumber, char *text);
   int16_t SIM808SmsSend(char *destinationAddress, char *text);
   int16_t SIM808GetGNSSPowerMode(GNSS_Power_Mode_t *mode);
   int16_t SIM808SetGNSSPowerMode(GNSS_Power_Mode_t mode);
   int16_t SIM808GetLastParsedNMEA(char *sentence);
   int16_t SIM808DefineLastParsedNMEA(char *sentence);
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
                                       uint8_t *fixMode);
   int16_t SIM808GetGNSSNMEAParameter(void);
   int16_t SIM808GetNetworkRegistrationStatus(Network_Reg_URC_Mode_t *mode,
                                              Network_Reg_Status_t *status);
   int16_t SIM808GetGPRSAttachStatus(GPRS_Attach_Status_t *status);
   int16_t SIM808SetGPRSAttachStatus(GPRS_Attach_Status_t status);
   int16_t SIM808GetApnName(char *apn);
   int16_t SIM808SetApnNameAndStartTask(char *apnName);
   int16_t SIM808BringUpWireless(void);
   int16_t SIM808GetLocalIPAddress(char *ip);
   int16_t SIM808StartTCP(char *serverIP, char *serverPort, Cip_Status_t *status);
   int16_t SIM808GetConnectionStatus(Connection_Status_t *status);
   int16_t SIM808DeactiveGPRSPDP(void);
   int16_t SIM808GetPDPContextStatus(PDP_Context_Status_t *pdpStatus);
   int16_t SIM808SetPDPContextStatus(PDP_Context_Status_t pdpStatus);
   int16_t SIM808DefinePDPContext(char *apn);
   int16_t SIM808GetIPDStatus(IPD_Status_t *status);
   int16_t SIM808DefineIPDStatus(IPD_Status_t status);
   int16_t SIM808TcpSend(uint8_t *data, uint16_t length);
   int16_t SIM808TcpClose(void);
   int16_t SIM808SetBearerInternetType(uint8_t cid, Bearer_Con_type_t type);
   int16_t SIM808SetBearerApnName(uint8_t cid, char *apn);
   int16_t SIM808BearerClose(uint8_t cid);
   int16_t SIM808BearerOpen(uint8_t cid);
   int16_t SIM808GetBearerStatus(uint8_t cid, Bearer_Status_t *status, char *ip);
   int16_t SIM808QueryIPAddressFromDomain(char *domain, char *ip1);
   int16_t SIM808HttpInitialize(void);
   int16_t SIM808SetHttpCid(uint8_t cid);
   int16_t SIM808SetHttpURL(char *serverAddress, char *serverPort, char *path);
   int16_t SIM808SetHttpCntentType(char *content);
   int16_t SIM808HttpGet(uint16_t *statusCode, uint16_t *dataLenth);
   int16_t SIM808HttpRead(uint16_t startIndex, uint16_t size);
   int16_t SIM808HttpInputData(uint8_t *data, uint16_t length);
   int16_t SIM808HttpPost(uint16_t *statusCode, uint16_t *dataLenth);
   int16_t SIM808Test(void);
   int16_t SIM808ATTest(char *at, uint32_t timeout);
   int16_t SIM808SetDTEBaudRate(uint32_t baudRate);
   int16_t SIM808SetPowerState(FunctionalState state);
   int16_t SIM808Reset(void);
   uint32_t Sim808CheckResponse(char *pStr, uint16_t length);
   void Sim808CheckReceiveByte(uint8_t byte);

   int16_t Sim808TrueReceiveData(void);

   void Sim808Interrupt(void);

   void Sim808Lock(void);
   void Sim808UnLock(void);

   /**
  * @}
  */

   /**
  * @}
  */

#ifdef __cplusplus
}
#endif

#endif /* __SIM808_DRIVER_H */

/*********************************END OF FILE****************************/
