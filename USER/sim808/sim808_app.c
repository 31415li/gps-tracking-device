/**
  ******************************************************************************
  * @file    sim808_app.c
  * @author  Mahdad Ghasemian
  * @version V0.0.0
  * @date    25-March-2018
  * @brief   
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "app_cfg.h"
#include "sim808_app.h"
#include "bsp.h"
#include <string.h>
#include "sim808_driver.h"
#include "system.h"
#include "record.h"
#include "flash.h"
#include "Position_Thread.h"

/** @defgroup SIM808_APP
  * @brief 
  * @{
  */

/** @defgroup SIM808_APP_Private_Macros
  * @{
  */

/**
  * @}
  */

/** @defgroup SIM808_APP_Private_Variables
  * @{
  */

/**
  * @}
  */

/** @defgroup SIM808_APP_Static_Functions
  * @{
  */

/**
  * @}
  */

/** @defgroup SIM808_APP_Private_Functions
  * @{
  */

void AppGsmPSDClear(void);
void AppGsmTCPClear(void);
extern void clearIntervals(void);

/**
  * @brief  
  * @param  
  * @return 
  */
void TcpCmd(uint8_t *data, uint16_t length)
{
	int16_t status;
	uint8_t Class;

	APP_TRACE_DATA(("TCP : %s\r", data));

	// Class
	Class = data[0];

	// Send TCP
	/*
   Sim808Lock();
   status = SIM808TcpSend("Hello\r", 6);
   Sim808UnLock();
   if (status == 0)
      APP_TRACE_INFO(("Send \r"));
   */
}

/**
  * @brief  
  * @param  
  * @return 
  */
void SmsCmd(char *number, char *text)
{
	static char buffer[SIM808_SEND_SMS_MAX_SIZE];
	char phoneNumber[16];
	char pattern[16] = {0};
	int16_t status;
	uint8_t rssi;
	uint8_t ber;
	uint8_t userStatus = 0;
	uint8_t i;

	if (number[0] == '+')
	{
		phoneNumber[0] = '0';
		memcpy(&phoneNumber[1], &number[3], 10);
		phoneNumber[11] = '\0';
	}
	else
	{
		strcpy(phoneNumber, number);
	}

	APP_TRACE_INFO(("TEXT : %s\r", text));
	APP_TRACE_INFO(("NUMBER : %s\r", number));

	buffer[0] = '\0';

	// Private Commands
	if (memcmp(phoneNumber, FACTORY_NUMBER_1, 11) == 0 ||
		memcmp(phoneNumber, FACTORY_NUMBER_2, 11) == 0 ||
		memcmp(phoneNumber, FACTORY_NUMBER_3, 11) == 0)
	{

		// Request command list
		if (memcmp(text, "?", 1) == 0)
		{
			snprintf(buffer, SIM808_SEND_SMS_MAX_SIZE,
					 "%s\n%s\n%s\n%s\n%s\n%s\n%s\n%s\n%s\n%s\n%s\n%s\n%s\n%s\n%s\n%s\n",
					 "?", "reset", "offon", "Ser=", "Info", "EraseRecords",
					 "EraseEeprom", "ClearAdmin", "GetIndex", "SetTS=", "SetIndex=", "SetSAT,on/off",
					 "FlatState=", "Retry", "ResetFactory", "EraseIndexes");
		}
		// OFF and ON Module
		else if (memcmp(text, "offon", 3) == 0)
		{
			APP_TRACE_INFO(("OFF and ON By SMS\r"));
			Sim808Lock();
			SIM808SetPowerState(DISABLE);
			osDelay(500);
			SIM808SetPowerState(ENABLE);
			Sim808UnLock();
		}
		// Set SerialNumber
		else if (memcmp(text, "Ser=", 4) == 0)
		{
			uint32_t value;

			strcpy(buffer, text);
			sscanf(&buffer[4], "%d", &value);
			SystemSetup.SerialNumber = value;
			parameterWrite();
			parameterLoad();
			snprintf(buffer, SIM808_SEND_SMS_MAX_SIZE,
					 "Set Serial is %i",
					 SystemSetup.SerialNumber);
		}
		// Erase All Records
		else if (memcmp(text, "EraseRecords", 12) == 0)
		{
			uint32_t flag;

			flag = osEventFlagsGet(app_eventflag_id);
			if (flag & APP_FLASH_INIT_RECORD_FLAG)
			{
				recordLock();
				sFlashChipErase();
				if (recordClearIndex() == RECORD_ERROR_EEPROM)
					osEventFlagsClear(app_eventflag_id, APP_FLASH_INIT_RECORD_FLAG);
				else
					osEventFlagsSet(app_eventflag_id, APP_FLASH_INIT_RECORD_FLAG);
				recordUnLock();

				snprintf(buffer, SIM808_SEND_SMS_MAX_SIZE,
						 "EraseRecords Success");
			}
			else
				snprintf(buffer, SIM808_SEND_SMS_MAX_SIZE,
						 "This device dont have spi flash");
		}
		// Erase Indexes
		else if (memcmp(text, "EraseIndexes", 12) == 0)
		{
			uint32_t flag, read, write;

			flag = osEventFlagsGet(app_eventflag_id);
			if (flag & APP_FLASH_INIT_RECORD_FLAG)
			{
				recordLock();
				if (recordClearIndex() == RECORD_ERROR_EEPROM)
					osEventFlagsClear(app_eventflag_id, APP_FLASH_INIT_RECORD_FLAG);
				else
					osEventFlagsSet(app_eventflag_id, APP_FLASH_INIT_RECORD_FLAG);
				recordGetIndex(&read, &write);
				recordUnLock();

				snprintf(buffer, SIM808_SEND_SMS_MAX_SIZE,
						 "Read=%d, Write=%d", read, write);
			}
			else
				snprintf(buffer, SIM808_SEND_SMS_MAX_SIZE,
						 "This device dont have spi flash");
		}
		// Erase All Eeprom
		else if (memcmp(text, "EraseEeprom", 11) == 0)
		{
			setParameterToVirgin();
			parameterWrite();
			parameterLoad();
			snprintf(buffer, SIM808_SEND_SMS_MAX_SIZE,
					 "%s",
					 "All Setting Erased!");
		}
		// Clear admin phone number
		else if (memcmp(text, "ClearAdmin", 10) == 0)
		{
			memset(SystemSetup.adminPhoneNumber, 0, SIM808_PHONE_NUMBER_MAX_SIZE);
			parameterWrite();
			parameterLoad();
			snprintf(buffer, SIM808_SEND_SMS_MAX_SIZE,
					 "%s",
					 "Admin Cleared!");
		}
		// Get Record Index
		else if (memcmp(text, "GetIndex", 8) == 0)
		{
			uint32_t flag, read, write;

			flag = osEventFlagsGet(app_eventflag_id);
			if (flag & APP_FLASH_INIT_RECORD_FLAG)
			{
				recordLock();
				recordGetIndex(&read, &write);
				recordUnLock();

				snprintf(buffer, SIM808_SEND_SMS_MAX_SIZE,
						 "Read=%d, Write=%d", read, write);
			}
			else
				snprintf(buffer, SIM808_SEND_SMS_MAX_SIZE,
						 "This device dont have spi flash");
		}
		// Set Record Read Index
		else if (memcmp(text, "SetIndex=", 9) == 0)
		{
			uint32_t flag, index, read, write;

			strcpy(buffer, text);
			sscanf(&buffer[9], "%d", &index);

			flag = osEventFlagsGet(app_eventflag_id);
			if (flag & APP_FLASH_INIT_RECORD_FLAG)
			{
				recordLock();
				if (recordSetReadIndex(index) == RECORD_ERROR_EEPROM)
					osEventFlagsClear(app_eventflag_id, APP_FLASH_INIT_RECORD_FLAG);
				else
					osEventFlagsSet(app_eventflag_id, APP_FLASH_INIT_RECORD_FLAG);
				recordGetIndex(&read, &write);
				recordUnLock();

				snprintf(buffer, SIM808_SEND_SMS_MAX_SIZE,
						 "Read=%d, Write=%d", read, write);
			}
			else
				snprintf(buffer, SIM808_SEND_SMS_MAX_SIZE,
						 "This device dont have spi flash");
		}
		// Set Record Read Index
		else if (memcmp(text, "SetWIndex=", 10) == 0)
		{
			uint32_t flag, index, read, write;

			strcpy(buffer, text);
			sscanf(&buffer[10], "%d", &index);

			flag = osEventFlagsGet(app_eventflag_id);
			if (flag & APP_FLASH_INIT_RECORD_FLAG)
			{
				recordLock();
				if (recordSetWriteIndex(index) == RECORD_ERROR_EEPROM)
					osEventFlagsClear(app_eventflag_id, APP_FLASH_INIT_RECORD_FLAG);
				else
					osEventFlagsSet(app_eventflag_id, APP_FLASH_INIT_RECORD_FLAG);
				recordGetIndex(&read, &write);
				recordUnLock();

				snprintf(buffer, SIM808_SEND_SMS_MAX_SIZE,
						 "Read=%d, Write=%d", read, write);
			}
			else
				snprintf(buffer, SIM808_SEND_SMS_MAX_SIZE,
						 "This device dont have spi flash");
		}
		// Set TSLimit
		else if (memcmp(text, "SetTS=", 6) == 0)
		{
			uint32_t value;

			strcpy(buffer, text);
			sscanf(&buffer[6], "%d", &value);
			SystemSetup.TSLimit = value;
			parameterWrite();
			parameterLoad();
			snprintf(buffer, SIM808_SEND_SMS_MAX_SIZE,
					 "TS = %i",
					 SystemSetup.TSLimit);
		}
		// Set Alarm status
		else if ((memcmp(text, "SetSAT,", 7) == 0))
		{
			char *token;
			char state[7];
			uint8_t status = 1;

			token = strtokSingle(text, ",");
			if (token != NULL)
			{
				token = strtokSingle(NULL, "\0");
				if (token != NULL)
				{
					snprintf(state, sizeof(state), "%s", token);

					if (memcmp(state, "on", 2) == 0)
					{
						SystemSetup.SaveAnyTime = 1;
						parameterWrite();
						parameterLoad();
					}
					else if (memcmp(state, "off", 3) == 0)
					{
						SystemSetup.SaveAnyTime = 0;
						parameterWrite();
						parameterLoad();

						clearCheckMovingIntervals();
					}
				}
			}

			snprintf(buffer, SIM808_SEND_SMS_MAX_SIZE,
					 "Save Record Any Time is %s\n",
					 (SystemSetup.SaveAnyTime) ? "on" : "off");
		}
		// Reset Factory
		else if (memcmp(text, "ResetFactory", 12) == 0)
		{
			setParameterToFactory();
			parameterWrite();
			parameterLoad();
			snprintf(buffer, SIM808_SEND_SMS_MAX_SIZE,
					 "%s",
					 "Reset Factory Success!");
		}
		// Flat State
		else if (memcmp(text, "FlatState=", 10) == 0)
		{
			uint32_t value;
			uint32_t read, write;
			uint32_t flag;

			strcpy(buffer, text);
			sscanf(&buffer[10], "%d", &value);
			setParameterToFlatState(value);
			parameterWrite();

			flag = osEventFlagsGet(app_eventflag_id);
			if (flag & APP_FLASH_INIT_RECORD_FLAG)
			{
				recordLock();
				if (recordClearIndex() == RECORD_ERROR_EEPROM)
					osEventFlagsClear(app_eventflag_id, APP_FLASH_INIT_RECORD_FLAG);
				else
					osEventFlagsSet(app_eventflag_id, APP_FLASH_INIT_RECORD_FLAG);
				recordUnLock();
			}

			NVIC_SystemReset();
		}
		// Retry
		else if (memcmp(text, "Retry", 5) == 0)
		{
			NVIC_SystemReset();
		}
		// Eeprom State temp change
		else if (memcmp(text, "Eeprom=", 7) == 0)
		{
			uint32_t value;

			strcpy(buffer, text);
			sscanf(&buffer[7], "%d", &value);

			if (value == 0)
				osEventFlagsClear(app_eventflag_id, APP_FLASH_INIT_RECORD_FLAG);
			else
				osEventFlagsSet(app_eventflag_id, APP_FLASH_INIT_RECORD_FLAG);

			if (osEventFlagsGet(app_eventflag_id) & APP_FLASH_INIT_RECORD_FLAG)
				value = 1;
			else
				value = 0;

			snprintf(buffer, SIM808_SEND_SMS_MAX_SIZE,
					 "FLASH_INIT %i",
					 value);
		}
	}

	// First Command
	if ((SystemSetup.adminPhoneNumber[0] == 0) && (SystemSetup.adminPhoneNumber[SIM808_PHONE_NUMBER_MAX_SIZE - 1] == 0))
	{
		// Set admin phone number
		if (memcmp(text, "admin", 5) == 0)
		{
			char *token;
			char phoneNumber[SIM808_PHONE_NUMBER_MAX_SIZE];
			uint8_t status = 0;

			token = strtokSingle(text, ",");
			if (token != NULL)
			{
				token = strtokSingle(NULL, ",");
				if (token != NULL)
				{
					snprintf(phoneNumber, SIM808_PHONE_NUMBER_MAX_SIZE, "%s", token);

					memcpy(SystemSetup.adminPhoneNumber, phoneNumber, SIM808_PHONE_NUMBER_MAX_SIZE);
					parameterWrite();
					parameterLoad();
					status = 1;
				}
			}
			if (status == 1)
			{
				snprintf(buffer, SIM808_SEND_SMS_MAX_SIZE,
						 "%s",
						 "Admin phone number Set OK");
			}
			else
			{
				snprintf(buffer, SIM808_SEND_SMS_MAX_SIZE,
						 "%s",
						 "Admin phone number No Set");
			}
		}
		// Get List of customer commands
		else if ((memcmp(text, "help", 4) == 0) || (memcmp(text, "Help", 4) == 0))
		{
			snprintf(buffer, SIM808_SEND_SMS_MAX_SIZE,
					 "%s\n%s\n%s\n%s\n%s\n%s\n%s\n%s\n%s\n%s\n%s\n%s\n",
					 "help", "123", "alarm,on/off", "speed,value", "status",
					 "reset", "Info",
					 "server,ip,port,id", "where?", "time",
					 "admin,phone", "user,phone,id");
		}
	}
	else
	{
		// Admin Commands
		//if( memcmp(SystemSetup.adminPhoneNumber, pattern, 16) == 0 ||
		if (memcmp(phoneNumber, SystemSetup.adminPhoneNumber, 11) == 0 ||
			memcmp(phoneNumber, FACTORY_NUMBER_1, 11) == 0 ||
			memcmp(phoneNumber, FACTORY_NUMBER_2, 11) == 0 ||
			memcmp(phoneNumber, FACTORY_NUMBER_3, 11) == 0)
		{

			// Get Position
			if ((memcmp(text, "where?", 6) == 0) ||
				(memcmp(text, "123", 3) == 0))
			{
				Sim808Lock();
				SIM808GetGNSSNMEAParameter();
				Sim808UnLock();
				snprintf(buffer, SIM808_SEND_SMS_MAX_SIZE,
						 "http://maps.google.com/?q=%d.%06d,%d.%06d",
						 gpsData.latitude / 1000000, gpsData.latitude % 1000000,
						 gpsData.longitude / 1000000, gpsData.longitude % 1000000);
			}
			// Get Date and Time
			else if (memcmp(text, "time", 4) == 0)
			{
				Sim808Lock();
				SIM808GetGNSSNMEAParameter();
				Sim808UnLock();
				snprintf(buffer, SIM808_SEND_SMS_MAX_SIZE,
						 "UTC: %04d/%02d/%02d %02d:%02d:%02d",
						 gpsData.year,
						 gpsData.month,
						 gpsData.day,
						 gpsData.hour,
						 gpsData.min,
						 gpsData.sec);
			}
			// Reset Module
			else if (memcmp(text, "reset", 3) == 0)
			{
				APP_TRACE_INFO(("Reset By SMS\r"));
				Sim808Lock();
				SIM808Reset();
				Sim808UnLock();
			}
			// Get Info
			else if (memcmp(text, "Info", 4) == 0)
			{
				snprintf(buffer, SIM808_SEND_SMS_MAX_SIZE,
						 "serial=%i\n"
						 "version=%s.%d.%d.%d",
						 SystemSetup.SerialNumber,
						 FIRMEARE_VERSION_DEVICE_NAME,
						 FIRMEARE_VERSION_MAJOR,
						 FIRMEARE_VERSION_MINOR,
						 FIRMEARE_VERSION_PATCH);
			}
			// Get Device Status
			else if (memcmp(text, "status", 6) == 0)
			{
				uint8_t server = 0, acc = 0, power = 0, door = 0;
				GNSS_Power_Mode_t gnssPowermode;
				GPRS_Attach_Status_t gprsStatus;
				int16_t status;

				if (osEventFlagsGet(app_eventflag_id) & APP_GSM_TCP_OK_FLAG)
					server = 1;
				bspAccReadStatus(&acc);
				bspPowerReadStatus(&power);
				bspDoorReadStatus(&door);

				Sim808Lock();
				SIM808GetSignalStrength(&rssi, &ber);
				SIM808GetGNSSNMEAParameter();
				SIM808GetGNSSPowerMode(&gnssPowermode);
				status = SIM808GetGPRSAttachStatus(&gprsStatus);
				Sim808UnLock();

				snprintf(buffer, SIM808_SEND_SMS_MAX_SIZE,
						 "Power=%s\n"
						 "ACC=%s\n"
						 "Door=%s\n"
						 "GPRS=%s\n"
						 "GPS=%s\n"
						 "Server=%s\n"
						 "CSQ=%d\n"
						 "Fix=%s\n"
						 "UTC: %04d/%02d/%02d %02d:%02d:%02d\n",
						 (power == 1) ? "ON" : "OFF",
						 (acc == 1) ? "ON" : "OFF",
						 (door == 1) ? "Open" : "Close",
						 (status == 0 && gprsStatus == GPRS_ATTACHED) ? "ON" : "OFF",
						 (gnssPowermode == GNSS_POWER_MODE_TURN_ON) ? "OK" : "No Gps",
						 (server == 1) ? "Connect" : "Disconnect",
						 rssi,
						 (gpsData.fix != 0) ? "OK" : "No Fix",
						 gpsData.year,
						 gpsData.month,
						 gpsData.day,
						 gpsData.hour,
						 gpsData.min,
						 gpsData.sec);
			}
			// Set server
			else if (memcmp(text, "server", 6) == 0)
			{
				char *token;
				char ip[SIM808_IP_MAX_SIZE];
				char port[SIM808_PORT_MAX_SIZE];
				uint8_t status = 0;

				token = strtokSingle(text, ",");
				if (token != NULL)
				{
					token = strtokSingle(NULL, ",");
					if (token != NULL)
					{
						snprintf(ip, SIM808_IP_MAX_SIZE, "%s", token);
						token = strtokSingle(NULL, ",");
						if (token != NULL)
						{
							snprintf(port, SIM808_PORT_MAX_SIZE, "%s", token);

							if ((ip[0] != 0) ||
								(ip[SIM808_IP_MAX_SIZE - 1] != 0))
								SystemSetup.ServerModeActive = 1;
							else
								SystemSetup.ServerModeActive = 0;

							memcpy(SystemSetup.ServerIP, ip, SIM808_IP_MAX_SIZE);
							memcpy(SystemSetup.ServerPort, port, SIM808_PORT_MAX_SIZE);
							parameterWrite();
							parameterLoad();
							status = 1;
						}
					}
				}
				if (status == 1)
				{
					snprintf(buffer, SIM808_SEND_SMS_MAX_SIZE,
							 "Server Set Ok.\nIP:%s\nPort:%s",
							 SystemSetup.ServerIP, SystemSetup.ServerPort);
					// Shut down pdp for init state
					AppGsmPSDClear();
					AppGsmTCPClear();
					Sim808Lock();
					status = SIM808DeactiveGPRSPDP();
					Sim808UnLock();
				}
				else
				{
					snprintf(buffer, SIM808_SEND_SMS_MAX_SIZE,
							 "%s",
							 "Server No Set");
				}
			}
			// Set admin phone number
			else if (memcmp(text, "admin", 5) == 0)
			{
				char *token;
				char phoneNumber[SIM808_PHONE_NUMBER_MAX_SIZE];
				uint8_t status = 0;

				token = strtokSingle(text, ",");
				if (token != NULL)
				{
					token = strtokSingle(NULL, ",");
					if (token != NULL)
					{
						snprintf(phoneNumber, SIM808_PHONE_NUMBER_MAX_SIZE, "%s", token);

						memcpy(SystemSetup.adminPhoneNumber, phoneNumber, SIM808_PHONE_NUMBER_MAX_SIZE);
						parameterWrite();
						parameterLoad();
						status = 1;
					}
				}
				if (status == 1)
				{
					snprintf(buffer, SIM808_SEND_SMS_MAX_SIZE,
							 "%s",
							 "Admin phone number Set OK");
				}
				else
				{
					snprintf(buffer, SIM808_SEND_SMS_MAX_SIZE,
							 "%s",
							 "Admin phone number No Set");
				}
			}
			// Set user phone number
			else if (memcmp(text, "user", 4) == 0)
			{
				char *token;
				char phoneNumber[SIM808_IP_MAX_SIZE];
				uint32_t id;
				uint8_t status = 0;

				token = strtokSingle(text, ",");
				if (token != NULL)
				{
					token = strtokSingle(NULL, ",");
					if (token != NULL)
					{
						snprintf(phoneNumber, SIM808_PHONE_NUMBER_MAX_SIZE, "%s", token);
						token = strtokSingle(NULL, ",");
						if (token != NULL)
						{
							sscanf(token, "%d", &id);

							if (id > 0 && id <= USER_MAX_COUNT)
								id--;
							if (id < USER_MAX_COUNT)
							{
								memcpy(SystemSetup.userPhoneNumber[id], phoneNumber, SIM808_PHONE_NUMBER_MAX_SIZE);
								parameterWrite();
								parameterLoad();
								status = 1;
							}
						}
					}
				}
				if (status == 1)
				{
					snprintf(buffer, SIM808_SEND_SMS_MAX_SIZE,
							 "%s",
							 "User add new OK");
				}
				else
				{
					snprintf(buffer, SIM808_SEND_SMS_MAX_SIZE,
							 "%s",
							 "User add new Error");
				}
			}
			// Get List of customer commands
			else if ((memcmp(text, "help", 4) == 0) || (memcmp(text, "Help", 4) == 0))
			{
				snprintf(buffer, SIM808_SEND_SMS_MAX_SIZE,
						 "%s\n%s\n%s\n%s\n%s\n%s\n%s\n%s\n%s\n%s\n%s\n%s\n%s\n%s\n%s\n",
						 "help", "123",
						 "server,ip,port,id",
						 "admin,phone", "user,phone,id",
						 "speed,value",
						 "alarm,on/off", "alarmmode,sms,on/off", "alarmmode,call,on/off",
						 "status", "where?", "time", "reset", "Info", "loop,1");
			}
			// Set Alarm status
			else if ((memcmp(text, "alarm,", 6) == 0) || (memcmp(text, "Alarm,", 6) == 0))
			{
				char *token;
				char state[6];
				uint8_t status = 1;

				token = strtokSingle(text, ",");
				if (token != NULL)
				{
					token = strtokSingle(NULL, "\0");
					if (token != NULL)
					{
						snprintf(state, sizeof(state), "%s", token);

						if (memcmp(state, "on", 2) == 0)
						{
							SystemSetup.AlarmStatusFlag = 1;
							if (SystemSetup.AlarmModeSms == 0 && SystemSetup.AlarmModeCall == 0)
							{
								SystemSetup.AlarmModeSms = 1;
								//SystemSetup.AlarmModeCall = 1;
							}

							parameterWrite();
							parameterLoad();
						}
						else if (memcmp(state, "off", 3) == 0)
						{
							SystemSetup.AlarmStatusFlag = 0;
							parameterWrite();
							parameterLoad();

							clearIntervals();
						}
					}
				}

				if (SystemSetup.AlarmStatusFlag)
				{
					snprintf(buffer, SIM808_SEND_SMS_MAX_SIZE,
							 "alarm is %s\nalarm by sms is %s and by call is %s",
							 (SystemSetup.AlarmStatusFlag) ? "on" : "off",
							 (SystemSetup.AlarmModeSms) ? "on" : "off",
							 (SystemSetup.AlarmModeCall) ? "on" : "off");
				}
				else
				{
					snprintf(buffer, SIM808_SEND_SMS_MAX_SIZE,
							 "alarm is %s",
							 (SystemSetup.AlarmStatusFlag) ? "on" : "off");
				}
			}
			// Set Alarm mode (sms, call)
			else if ((memcmp(text, "alarmmode,", 10) == 0) || (memcmp(text, "Alarmmode,", 10) == 0))
			{
				char *token;
				char mode[6];
				char state[6];
				uint8_t status = 0;

				token = strtokSingle(text, ",");
				if (token != NULL)
				{
					token = strtokSingle(NULL, ",");
					if (token != NULL)
					{
						snprintf(mode, sizeof(mode), "%s", token);
						token = strtokSingle(NULL, "\0");
						if (token != NULL)
						{
							snprintf(state, sizeof(state), "%s", token);

							if (memcmp(mode, "sms", 3) == 0)
							{
								if (memcmp(state, "on", 2) == 0)
								{
									SystemSetup.AlarmModeSms = 1;
									status = 1;
								}
								else if (memcmp(state, "off", 3) == 0)
								{
									if ((SystemSetup.AlarmStatusFlag == 0) ||
										((SystemSetup.AlarmStatusFlag == 1) && (SystemSetup.AlarmModeCall == 1)))
									{
										SystemSetup.AlarmModeSms = 0;
										status = 1;
									}
								}
							}
							else if (memcmp(mode, "call", 4) == 0)
							{
								if (memcmp(state, "on", 2) == 0)
								{
									SystemSetup.AlarmModeCall = 1;
									status = 1;
								}
								else if (memcmp(state, "off", 3) == 0)
								{
									if ((SystemSetup.AlarmStatusFlag == 0) ||
										((SystemSetup.AlarmStatusFlag == 1) && (SystemSetup.AlarmModeSms == 1)))
									{
										SystemSetup.AlarmModeCall = 0;
										status = 1;
									}
								}
							}

							if (status == 1)
							{
								parameterWrite();
								parameterLoad();
							}
						}
					}
				}

				snprintf(buffer, SIM808_SEND_SMS_MAX_SIZE,
						 "alarm by sms is %s and by call is %s",
						 (SystemSetup.AlarmModeSms) ? "on" : "off",
						 (SystemSetup.AlarmModeCall) ? "on" : "off");
			}
			// Set Maximum Speed
			else if ((memcmp(text, "speed,", 6) == 0) || (memcmp(text, "Speed,", 6) == 0))
			{
				uint32_t value;

				strcpy(buffer, text);
				sscanf(&buffer[6], "%d", &value);
				SystemSetup.MaximumSpeed = value;
				parameterWrite();
				parameterLoad();
				snprintf(buffer, SIM808_SEND_SMS_MAX_SIZE,
						 "max speed is %i km/h",
						 SystemSetup.MaximumSpeed);
			}
			// Set Save Record Interval
			else if ((memcmp(text, "loop,", 5) == 0) || (memcmp(text, "Loop,", 5) == 0))
			{
				uint32_t value;

				strcpy(buffer, text);
				sscanf(&buffer[5], "%d", &value);
				SystemSetup.SaveRecordIntervalFactor = (value > 0 || value < 2) ? value : 1;
				;
				parameterWrite();
				parameterLoad();
				snprintf(buffer, SIM808_SEND_SMS_MAX_SIZE,
						 "time out is %i S",
						 SystemSetup.SaveRecordIntervalFactor);
			}
		}

		// User Search
		for (i = 0; i < USER_MAX_COUNT; i++)
			if (memcmp(phoneNumber, SystemSetup.userPhoneNumber[i], 11) == 0)
				userStatus = 1;

		// User Commands
		if (userStatus == 1)
		{

			// Get List of user commands
			if ((memcmp(text, "help", 4) == 0) || (memcmp(text, "Help", 4) == 0))
			{
				snprintf(buffer, SIM808_SEND_SMS_MAX_SIZE,
						 "%s\n%s\n%s\n%s\n",
						 "help", "123", "where?", "alarm");
			}
			// Get Position
			else if ((memcmp(text, "where?", 6) == 0) ||
					 (memcmp(text, "123", 3) == 0))
			{
				Sim808Lock();
				SIM808GetGNSSNMEAParameter();
				Sim808UnLock();
				snprintf(buffer, SIM808_SEND_SMS_MAX_SIZE,
						 "http://maps.google.com/?q=%i.%06i,%i.%06i",
						 gpsData.latitude / 1000000, gpsData.latitude % 1000000,
						 gpsData.longitude / 1000000, gpsData.longitude % 1000000);
			}
			// Set Alarm status
			else if ((memcmp(text, "alarm,", 6) == 0) || (memcmp(text, "Alarm,", 6) == 0))
			{
				char *token;
				char state[6];
				uint8_t status = 1;

				token = strtokSingle(text, ",");
				if (token != NULL)
				{
					token = strtokSingle(NULL, "\0");
					if (token != NULL)
					{
						snprintf(state, sizeof(state), "%s", token);

						if (memcmp(state, "on", 2) == 0)
						{
							SystemSetup.AlarmStatusFlag = 1;
							if (SystemSetup.AlarmModeSms == 0 && SystemSetup.AlarmModeCall == 0)
							{
								SystemSetup.AlarmModeSms = 1;
								//SystemSetup.AlarmModeCall = 1;
							}

							parameterWrite();
							parameterLoad();
						}
						else if (memcmp(state, "off", 3) == 0)
						{
							SystemSetup.AlarmStatusFlag = 0;
							parameterWrite();
							parameterLoad();

							clearIntervals();
						}
					}
				}

				snprintf(buffer, SIM808_SEND_SMS_MAX_SIZE,
						 "alarm is %s\nalarm by sms is %s and by call is %s",
						 (SystemSetup.AlarmStatusFlag) ? "on" : "off",
						 (SystemSetup.AlarmModeSms) ? "on" : "off",
						 (SystemSetup.AlarmModeCall) ? "on" : "off");
			}
		}
	}

	osDelay(1000);

	// Send Response
	if (strlen(buffer) > 0)
	{

		Sim808Lock();
		SIM808SmsSend(phoneNumber, buffer);
		Sim808UnLock();
		if (status == 0)
			APP_TRACE_INFO(("Send SMS"));
	}
}

/**
  * @brief  
  * @param  
  * @return 
  */
void AppGsmINITSet(void)
{
	osEventFlagsSet(app_eventflag_id, APP_GSM_INIT_OK_FLAG);
}

/**
  * @brief  
  * @param  
  * @return 
  */
void AppGsmINITClear(void)
{
	osEventFlagsClear(app_eventflag_id, APP_GSM_INIT_OK_FLAG);
}

/**
  * @brief  
  * @param  
  * @return 
  */
void AppGsmPSDSet(void)
{
	osEventFlagsSet(app_eventflag_id, APP_GSM_PSD_OK_FLAG);
}

/**
  * @brief  
  * @param  
  * @return 
  */
void AppGsmPSDClear(void)
{
	osEventFlagsClear(app_eventflag_id, APP_GSM_PSD_OK_FLAG);
}

/**
  * @brief  
  * @param  
  * @return 
  */
void AppGsmTCPSet(void)
{
	osEventFlagsSet(app_eventflag_id, APP_GSM_TCP_OK_FLAG);
}

/**
  * @brief  
  * @param  
  * @return 
  */
void AppGsmTCPClear(void)
{
	osEventFlagsClear(app_eventflag_id, APP_GSM_TCP_OK_FLAG);
}

/**
  * @brief  
  * @param  
  * @return 
  */
void AppGsmBearerSet(void)
{
	osEventFlagsSet(app_eventflag_id, APP_GSM_BEARER_OK_FLAG);
}

/**
  * @brief  
  * @param  
  * @return 
  */
void AppGsmBearerClear(void)
{
	osEventFlagsClear(app_eventflag_id, APP_GSM_BEARER_OK_FLAG);
}

/**
  * @}
  */

/**
  * @}
  */

/*********************************END OF FILE****************************/
