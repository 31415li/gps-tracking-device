/**
  ******************************************************************************
  * @file    app_cfg.h
  * @author  Mahdad Ghasemian
  * @version V0.0.0
  * @date    3-March-2018
  * @brief   
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __APP_CFG_H
#define __APP_CFG_H

#ifdef __cplusplus
extern "C"
{
#endif

  /** @defgroup APP_CFG 
  * @{
  */

  /** @defgroup APP_CFG_Configures
  * @{
  */

#define GPIO_PORT_(num) \
  ((num == 0) ? GPIOA : (num == 1) ? GPIOB : (num == 2) ? GPIOC : (num == 3) ? GPIOD : (num == 4) ? GPIOE : (num == 5) ? GPIOF : GPIOF)

#define GPIO_PIN_(num) \
  ((num == 0) ? GPIO_Pin_0 : (num == 1) ? GPIO_Pin_1 : (num == 2) ? GPIO_Pin_2 : (num == 3) ? GPIO_Pin_3 : (num == 4) ? GPIO_Pin_4 : (num == 5) ? GPIO_Pin_5 : (num == 6) ? GPIO_Pin_6 : (num == 7) ? GPIO_Pin_7 : (num == 8) ? GPIO_Pin_8 : (num == 9) ? GPIO_Pin_9 : (num == 10) ? GPIO_Pin_10 : (num == 11) ? GPIO_Pin_11 : (num == 12) ? GPIO_Pin_12 : (num == 13) ? GPIO_Pin_13 : (num == 14) ? GPIO_Pin_14 : (num == 15) ? GPIO_Pin_15 : GPIO_Pin_15)

//-------- <<< Use Configuration Wizard in Context Menu >>> -----------------
//

// <h>Device Information
// =======================
//

// <s> Device Name
#define DEVICE_NAME "GPS_Tracking"

//   <s>Firmware Version Device Name
#ifndef FIRMEARE_VERSION_DEVICE_NAME
#define FIRMEARE_VERSION_DEVICE_NAME "GPS"
#endif
//
//   <o>Firmware Version MAJOR
#ifndef FIRMEARE_VERSION_MAJOR
#define FIRMEARE_VERSION_MAJOR 1u
#endif
//
//   <o>Firmware Version MINOR
#ifndef FIRMEARE_VERSION_MINOR
#define FIRMEARE_VERSION_MINOR 0u
#endif
//
//   <o>Firmware Version PATCH
#ifndef FIRMEARE_VERSION_PATCH
#define FIRMEARE_VERSION_PATCH 0u
#endif

#ifndef FIRMEARE_VERSION
#define FIRMEARE_VERSION ((FIRMEARE_VERSION_MAJOR * 10000) + (FIRMEARE_VERSION_MINOR * 100) + FIRMEARE_VERSION_PATCH)
#endif

// <o> Hardware Version
#define HARDWARE_VERSION 2u

// <s> TCP AES Key
// <i> Must be 16 characters
#define TCP_AES_KEY "1234567890abcdef"

// </h>

// <h>Thread Enable
// =======================
//     <q> POSITION_THREAD Enable
#define POSITION_THREAD_EN 1
//     <q> APP_THREAD Enable
#define APP_THREAD_EN 1
//     <q> SIM808_THREAD Enable
#define SIM808_THREAD_EN 1
//
// </h>

// <h>Priorities Configuration
// =======================
//
//    <o> SIM808URC_THREAD priority
//       <8=> Low <16=> Below Normal <24=> Normal <32=> Above Normal
//       <40=> High <48=> Realtime (highest)
//    <i> Defines priority for SIM808URC Thread
//    <i> Default: Low
#define SIM808URC_THREAD_PRIO 48
//
//    <o> SIM808BASIC_THREAD priority
//       <8=> Low <16=> Below Normal <24=> Normal <32=> Above Normal
//       <40=> High <48=> Realtime (highest)
//    <i> Defines priority for SIM808BASIC Thread
//    <i> Default: Low
#define SIM808BASIC_THREAD_PRIO 40
//
//    <o> APP_THREAD_PRIO priority
//       <8=> Low <16=> Below Normal <24=> Normal <32=> Above Normal
//       <40=> High <48=> Realtime (highest)
//    <i> Defines priority for APP_THREAD_PRIO Thread
//    <i> Default: Low
#define APP_THREAD_PRIO 32
//
//    <o> POSITION_THREAD priority
//       <8=> Low <16=> Below Normal <24=> Normal <32=> Above Normal
//       <40=> High <48=> Realtime (highest)
//    <i> Defines priority for POSITION Thread
//    <i> Default: Low
#define POSITION_THREAD_PRIO 24
//
//    <o> APP_START_THREAD priority
//       <8=> Low <16=> Below Normal <24=> Normal <32=> Above Normal
//       <40=> High <48=> Realtime (highest)
//    <i> Defines priority for APP_START Thread
//    <i> Default: Low
#define APP_START_THREAD_PRIO 8
//
// </h>

// <h>Stack Sizes Configuration
// =======================
//
//     <o> SIM808BASIC_THREAD Stack size
//     <i> Set "SIM808BASIC_THREAD" Stack size
#define SIM808BASIC_THREAD_STK_SIZE 1536 / 8
//
//     <o> SIM808URC_THREAD Stack size
//     <i> Set "SIM808URC_THREAD" Stack size
#define SIM808URC_THREAD_STK_SIZE 1536 / 8
//
//     <o> APP_THREAD_STK_SIZE Stack size
//     <i> Set "APP_THREAD_STK_SIZE" Stack size
#define APP_THREAD_STK_SIZE 1024 / 8
//
//     <o> APP_THREAD_START Stack size
//     <i> Set "APP_THREAD_START" Stack size
#define APP_START_THREAD_STK_SIZE 1024 / 8
//
//     <o> POSITION_THREAD Stack size
//     <i> Set "POSITION_THREAD" Stack size
#define POSITION_THREAD_STK_SIZE 1536 / 8
//
// </h>

// <h>Size and Count
// =======================
//
//    <o> "SIM808_MSG_COUNT" Buffer size
//    <i> ""
#define SIM808_MSG_COUNT 2u
//
//    <o> "SIM808_DATA_BUFFER_SIZE" Buffer size
//    <i> ""
#define SIM808_DATA_BUFFER_SIZE (512u - 2u)
//
// </h>

// <h>Pin Configuration
// =======================
//
//  <h>LED Pin Configuration
//  -----------------------
//
//   <h> LED Green Pin
//   <i> GPIO Pxy (x = A..F, y = 0..15)
//     <o> Port <0=>GPIOA <1=>GPIOB <2=>GPIOC <3=>GPIOD
//              <4=>GPIOE <5=>GPIOF
//     <i>  Selects Port Name
#define LED_GREEN_PORT GPIO_PORT_(2)
//     <o> Bit <0-15>
//     <i> Selects Port Bit
#define LED_GREEN_PIN GPIO_PIN_(13)
//   </h>
//  </h>
//
//  <h>Output Pin Configuration
//  -----------------------
//
//   <h> Output 1 Pin
//   <i> GPIO Pxy (x = A..F, y = 0..15)
//     <o> Port <0=>GPIOA <1=>GPIOB <2=>GPIOC <3=>GPIOD
//              <4=>GPIOE <5=>GPIOF
//     <i>  Selects Port Name
#define OUTPUT_1_PORT GPIO_PORT_(2)
//     <o> Bit <0-15>
//     <i> Selects Port Bit
#define OUTPUT_1_PIN GPIO_PIN_(15)
//   </h>
//
//   <h> Output 2 Pin
//   <i> GPIO Pxy (x = A..F, y = 0..15)
//     <o> Port <0=>GPIOA <1=>GPIOB <2=>GPIOC <3=>GPIOD
//              <4=>GPIOE <5=>GPIOF
//     <i>  Selects Port Name
#define OUTPUT_2_PORT GPIO_PORT_(2)
//     <o> Bit <0-15>
//     <i> Selects Port Bit
#define OUTPUT_2_PIN GPIO_PIN_(15)
//   </h>
//  </h>
//
// <h>Buzzer Pin Configuration
//  -----------------------
//    <i> GPIO Pxy (x = A..F, y = 0..15)
//    <o> Port <0=>GPIOA <1=>GPIOB <2=>GPIOC <3=>GPIOD
//              <4=>GPIOE <5=>GPIOF
//    <i>  Selects Port Name
#define LED_BUZZER_PORT GPIO_PORT_(0)
//    <o> Bit <0-15>
//    <i> Selects Port Bit
#define LED_BUZZER_PIN GPIO_PIN_(1)
// </h>
//
//  <h>SIM808 Pin Configuration
//  -----------------------
//   <h> SIM808 Reset Pin
//   <i> GPIO Pxy (x = A..F, y = 0..15)
//     <o> Port <0=>GPIOA <1=>GPIOB <2=>GPIOC <3=>GPIOD
//              <4=>GPIOE <5=>GPIOF
//     <i>  Selects Port Name
#define SIM808_RESET_N_PORT GPIO_PORT_(1)
//     <o> Bit <0-15>
//     <i> Selects Port Bit
#define SIM808_RESET_N_PIN GPIO_PIN_(9)
//   </h>
//
//   <h> SIM808 Power on Pin
//   <i> GPIO Pxy (x = A..F, y = 0..15)
//     <o> Port <0=>GPIOA <1=>GPIOB <2=>GPIOC <3=>GPIOD
//              <4=>GPIOE <5=>GPIOF
//     <i>  Selects Port Name
#define SIM808_POWER_ON_N_PORT GPIO_PORT_(1)
//     <o> Bit <0-15>
//     <i> Selects Port Bit
#define SIM808_POWER_ON_N_PIN GPIO_PIN_(8)
//   </h>
//
//   <h> SIM808 Status Pin
//   <i> GPIO Pxy (x = A..F, y = 0..15)
//     <o> Port <0=>GPIOA <1=>GPIOB <2=>GPIOC <3=>GPIOD
//              <4=>GPIOE <5=>GPIOF
//     <i>  Selects Port Name
#define SIM808_STATUS_PORT GPIO_PORT_(1)
//     <o> Bit <0-15>
//     <i> Selects Port Bit
#define SIM808_STATUS_PIN GPIO_PIN_(12)
//   </h>
//
//   <h> SIM808 Interrupt Pin
//   <i> GPIO Pxy (x = A..F, y = 0..15)
//     <o> Port <0=>GPIOA <1=>GPIOB <2=>GPIOC <3=>GPIOD
//              <4=>GPIOE <5=>GPIOF
//     <i>  Selects Port Name
#define SIM808_INT_PORT GPIO_PORT_(0)
//     <o> Bit <0-15>
//     <i> Selects Port Bit
#define SIM808_INT_PIN GPIO_PIN_(8)
//     <o> Port <0=>GPIOA <1=>GPIOB <2=>GPIOC <3=>GPIOD
//              <4=>GPIOE <5=>GPIOF
//     <i>  Selects Port Name
#define SIM808_INT_EXTI_PORT 0
//     <o> Bit <0-15>
//     <i> Selects Port Bit
#define SIM808_INT_EXTI_PIN 8
//   </h>
//  </h>
//
//  <h>EEPROM I2C Pin Configuration
//  -----------------------
//    <o> I2C Port <0=>GPIOA <1=>GPIOB <2=>GPIOC <3=>GPIOD
//                 <4=>GPIOE <5=>GPIOF
//    <i>  Selects Port Name
#define EEPROM_I2C_PORT GPIO_PORT_(1)
//    <o> I2C SCL Pin <0-15>
#define EEPROM_I2C_SCL_PIN GPIO_PIN_(6)
//    <o> I2C SDA Pin <0-15>
#define EEPROM_I2C_SDA_PIN GPIO_PIN_(7)
//  </h>
//
//  <h>Flash SPI Pin Configuration
//  -----------------------
//    <o> SPI Port <0=>GPIOA <1=>GPIOB <2=>GPIOC <3=>GPIOD
//                      <4=>GPIOE <5=>GPIOF
//    <i>  Selects Port Name
#define FLASH_SPI_PORT GPIO_PORT_(0)
//    <o> SPI Number <1=>SPI1 <2=>SPI2 <3=>SPI3
//    <i> Selects SPI Number
#define FLASH_SPI_NUMBER 1
//    <o> SPI MOSI Pin <0-15>
#define FLASH_SPI_MOSI_PIN GPIO_PIN_(7)
//    <o> SPI MISO Pin <0-15>
#define FLASH_SPI_MISO_PIN GPIO_PIN_(6)
//    <o> SPI SCK Pin <0-15>
#define FLASH_SPI_SCK_PIN GPIO_PIN_(5)
//    <o> SPI CS Pin <0-15>
#define FLASH_SPI_CS_PIN GPIO_PIN_(4)
//  </h>
//
//  <h>Input Pin Configuration
//  -----------------------
//   <h> ACC Pin
//   <i> GPIO Pxy (x = A..F, y = 0..15)
//     <o> Port <0=>GPIOA <1=>GPIOB <2=>GPIOC <3=>GPIOD
//              <4=>GPIOE <5=>GPIOF
//     <i>  Selects Port Name
#define ACC_INPUT_PORT GPIO_PORT_(1)
//     <o> Bit <0-15>
//     <i> Selects Port Bit
#define ACC_INPUT_PIN GPIO_PIN_(1)
//     <o> Port <0=>GPIOA <1=>GPIOB <2=>GPIOC <3=>GPIOD
//              <4=>GPIOE <5=>GPIOF
//     <i>  Selects Port Name
#define ACC_INPUT_EXTI_PORT 1
//     <o> Bit <0-15>
//     <i> Selects Port Bit
#define ACC_INPUT_EXTI_PIN 1
//   </h>
//
//   <h> Power Pin
//   <i> GPIO Pxy (x = A..F, y = 0..15)
//     <o> Port <0=>GPIOA <1=>GPIOB <2=>GPIOC <3=>GPIOD
//              <4=>GPIOE <5=>GPIOF
//     <i>  Selects Port Name
#define POWER_INPUT_PORT GPIO_PORT_(0)
//     <o> Bit <0-15>
//     <i> Selects Port Bit
#define POWER_INPUT_PIN GPIO_PIN_(0)
//     <o> Port <0=>GPIOA <1=>GPIOB <2=>GPIOC <3=>GPIOD
//              <4=>GPIOE <5=>GPIOF
//     <i>  Selects Port Name
#define POWER_INPUT_EXTI_PORT 0
//     <o> Bit <0-15>
//     <i> Selects Port Bit
#define POWER_INPUT_EXTI_PIN 0
//   </h>
//
//   <h> Door Pin
//   <i> GPIO Pxy (x = A..F, y = 0..15)
//     <o> Port <0=>GPIOA <1=>GPIOB <2=>GPIOC <3=>GPIOD
//              <4=>GPIOE <5=>GPIOF
//     <i>  Selects Port Name
#define DOOR_INPUT_PORT GPIO_PORT_(1)
//     <o> Bit <0-15>
//     <i> Selects Port Bit
#define DOOR_INPUT_PIN GPIO_PIN_(14)
//     <o> Port <0=>GPIOA <1=>GPIOB <2=>GPIOC <3=>GPIOD
//              <4=>GPIOE <5=>GPIOF
//     <i>  Selects Port Name
#define DOOR_INPUT_EXTI_PORT 1
//     <o> Bit <0-15>
//     <i> Selects Port Bit
#define DOOR_INPUT_EXTI_PIN 14
//   </h>
// </h>
//
// </h>

// =======================
//     <o> DATA_PARAMETER_INTERFACE_TYPE <0=>INTERNAL_FLASH <1=>I2C_EEPROM
#define DATA_PARAMETER_INTERFACE_TYPE 0
#define DATA_PARAMETER_INTERFACE_INTERNAL_FLASH 0
#define DATA_PARAMETER_INTERFACE_I2C_EEPROM 1
//
// <c1> Fault Report Mode
// <i> Fault Report Enable or Disable
//#define FAULT_REPORT_MODE_EN
// </c>
//
// <o> Trace Level      <0=>TRACE_LEVEL_OFF
//                      <1=>TRACE_LEVEL_DATA
//                      <2=>TRACE_LEVEL_INFO
//                      <3=>TRACE_LEVEL_DBG
//                      <4=>TRACE_LEVEL_LOG
#define APP_TRACE_LEVEL 4
//
//	<o> APP TRACE BAUDRATE <9600=>9600 <115200=>115200
#define APP_TRACE_BAUDRATE 115200
//
//    <o> Printf Buffer size
//    <i> ""
#define APP_TRACE_BUFFER_SIZE 128u

//
//	<o> SIM808 BAUDRATE <9600=>9600 <115200=>115200
#define SIM808_BAUDRATE 115200

//
// <s> Factory number 1
#define FACTORY_NUMBER_1 "0912xxxyyyy"
//
// <s> Factory number 2
#define FACTORY_NUMBER_2 "0912xxxyyyy"
//
// <s> Factory number 2
#define FACTORY_NUMBER_3 "0912xxxyyyy"

//
// <s> APN name
#define APN_NAME "mtnirancell"

// <o> user count
// <i> ""
#define USER_MAX_COUNT 3

  // <<< end of configuration section >>>

#define TRACE_LEVEL_OFF 0
#define TRACE_LEVEL_DATA 1
#define TRACE_LEVEL_INFO 2
#define TRACE_LEVEL_DEBUG 3
#define TRACE_LEVEL_LOG 4

#define APP_TRACE sendTraceSerial
#define APP_TRACE_BINARRAY sendTraceSerialBinaray

#define APP_TRACE_DATA(x) ((APP_TRACE_LEVEL >= TRACE_LEVEL_DATA) ? (void)(APP_TRACE x) : (void)0)
#define APP_TRACE_INFO(x) ((APP_TRACE_LEVEL >= TRACE_LEVEL_INFO) ? (void)(APP_TRACE x) : (void)0)
#define APP_TRACE_DEBUG(x) ((APP_TRACE_LEVEL >= TRACE_LEVEL_DEBUG) ? (void)(APP_TRACE x) : (void)0)
#define APP_TRACE_LOG(x) ((APP_TRACE_LEVEL >= TRACE_LEVEL_LOG) ? (void)(APP_TRACE x) : (void)0)
#define APP_TRACE_LOG_BINARRAY(x) ((APP_TRACE_LEVEL >= TRACE_LEVEL_LOG) ? (void)(APP_TRACE_BINARRAY x) : (void)0)

#if (SERVER_MAIN_EN == 1)
#define SERVER_IP SERVER_IP_MAIN
#define SERVER_PORT SERVER_PORT_MAIN
#else
#define SERVER_IP SERVER_IP_TEST
#define SERVER_PORT SERVER_PORT_TEST
#endif

  /**
  * @}
  */

  /**
  * @}
  */

#ifdef __cplusplus
}
#endif

#endif /* __APP_CFG_H */

/*********************************END OF FILE****************************/
