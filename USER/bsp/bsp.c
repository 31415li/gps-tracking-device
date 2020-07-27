/**
  ******************************************************************************
  * @file    bsp.c
  * @author  Mahdad Ghasemian
  * @version V0.0.0
  * @date    3-March-2015
  * @brief   
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "bsp.h"
#include "stm32f10x.h" // Device header
#include "app_cfg.h"
#include "stm32f10x_rcc.h"   // Keil::Device:StdPeriph Drivers:RCC
#include "stm32f10x_gpio.h"  // Keil::Device:StdPeriph Drivers:GPIO
#include "stm32f10x_usart.h" // Keil::Device:StdPeriph Drivers:USART
#include "stm32f10x_i2c.h"   // Keil::Device:StdPeriph Drivers:I2C
#include "stm32f10x_pwr.h"   // Keil::Device:StdPeriph Drivers:PWR
#include "stm32f10x_iwdg.h"  // Keil::Device:StdPeriph Drivers:IWDG
#include "cmsis_os2.h"       // ::CMSIS:RTOS2
#include "GPIO_STM32F10x.h"  // Keil::Device:GPIO
#include <stdarg.h>
#include <stdio.h>
#include "eeprom.h"
#include "flash.h"
#include <time.h>
#include <string.h>

/** @defgroup BSP 
  * @brief 
  * @{
  */

/** @defgroup BSP_Private_Macros
  * @{
  */

/**
  * @}
  */

/** @defgroup BSP_Private_Variables
  * @{
  */

uint8_t resetSource = 0;

#if (APP_TRACE_LEVEL > 0)
static char bufTrace[APP_TRACE_BUFFER_SIZE + 1u];
#endif

/**
  * @}
  */

/** @defgroup BSP_Private_Functions
  * @{
  */

static void bspSim808Init(void);
static void bspSerial3Init(uint32_t baud_rate);
static void bspLedInit(void);
static void bspBuzzerInit(void);
void spiInit(void);
static void bspOutputInit(void);
static void bspInputInit(void);
static uint8_t debounceInput(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin, uint8_t *state);

#if (APP_TRACE_LEVEL > 0)
static void traceSerialInit(uint32_t baudRate);
#endif

/**
  * @brief  تنظيم اوليه سخت افزار
  * @param  ندارد
  * @return ندراد
  */
void bspInit(void)
{

   // enable watch for 26 s
   IWDG_Enable();
   IWDG_WriteAccessCmd(IWDG_WriteAccess_Enable);
   IWDG_SetPrescaler(IWDG_Prescaler_256);

   // read reset source
   if (RCC_GetFlagStatus(RCC_FLAG_PORRST))
   {
      resetSource |= 0x01;
   }
   if (RCC_GetFlagStatus(RCC_FLAG_IWDGRST))
   {
      resetSource |= 0x02;
   }
   if (RCC_GetFlagStatus(RCC_FLAG_WWDGRST))
   {
      resetSource |= 0x04;
   }
   if (RCC_GetFlagStatus(RCC_FLAG_LPWRRST))
   {
      resetSource |= 0x08;
   }
   if (RCC_GetFlagStatus(RCC_FLAG_SFTRST))
   {
      resetSource |= 0x10;
   }
   if (RCC_GetFlagStatus(RCC_FLAG_PINRST))
   {
      resetSource |= 0x20;
   }
   // The flags must be cleared manually after use
   RCC_ClearFlag();

#if (APP_TRACE_LEVEL > 0)
   traceSerialInit(APP_TRACE_BAUDRATE);
#endif

   /* قفل دسترسي به حافظه فلش را باز ميکند */
   FLASH_Unlock();

   bspLedInit();
   bspBuzzerInit();
   bspSim808Init();
   eepromInit();
   spiInit();
   bspOutputInit();
   bspInputInit();

   // reload watch
   IWDG_ReloadCounter();
}

/**
  * @brief  
  * @param  
  * @return 
  */
#if (APP_TRACE_LEVEL > 0)
static void traceSerialInit(uint32_t baudRate)
{
   //FlagStatus                 tc_status;
   GPIO_InitTypeDef GPIO_Init_Structure;
   USART_InitTypeDef USART1_Init_Structure;
   NVIC_InitTypeDef NVIC_Init_Structure;

   /* فعال سازي فرکانس پورت مربوطه */
   RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_AFIO, ENABLE);
   /* فعال سازي فرکانس قسمت سريال */
   RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);

   /* تنظيم پايه فرستنده سريال به عنوان خروجي پوش پول */
   GPIO_Init_Structure.GPIO_Pin = GPIO_Pin_9;
   GPIO_Init_Structure.GPIO_Speed = GPIO_Speed_50MHz;
   GPIO_Init_Structure.GPIO_Mode = GPIO_Mode_AF_PP;
   GPIO_Init(GPIOA, &GPIO_Init_Structure);

   /* تنظيم پايه گيرنده سريال به عنوان ورودي با مقاومت بالاکش */
   GPIO_Init_Structure.GPIO_Pin = GPIO_Pin_10;
   GPIO_Init_Structure.GPIO_Mode = GPIO_Mode_IPU;
   GPIO_Init(GPIOA, &GPIO_Init_Structure);

   /* پيکره بندي ارتباط سريال */
   USART1_Init_Structure.USART_BaudRate = baudRate;
   USART1_Init_Structure.USART_WordLength = USART_WordLength_8b;
   USART1_Init_Structure.USART_StopBits = USART_StopBits_1;
   USART1_Init_Structure.USART_Parity = USART_Parity_No;
   USART1_Init_Structure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
   USART1_Init_Structure.USART_Mode = USART_Mode_Tx;
   USART_Init(USART1, &USART1_Init_Structure);

   /* فعال سازي ارتباط سريال شماره */
   USART_Cmd(USART1, ENABLE);

   //USART_ITConfig(USART1, USART_IT_TC, ENABLE);
   USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);

   /* فعال سازي وقفه ارتباط سريال شماره 1 */
   NVIC_Init_Structure.NVIC_IRQChannel = USART1_IRQn;
   NVIC_Init_Structure.NVIC_IRQChannelPreemptionPriority = 0;
   NVIC_Init_Structure.NVIC_IRQChannelSubPriority = 5;
   NVIC_Init_Structure.NVIC_IRQChannelCmd = ENABLE;
   NVIC_Init(&NVIC_Init_Structure);
}
#endif

/**
  * @brief  
  * @param  
  * @return 
  */
#if (APP_TRACE_LEVEL > 0)
void sendTraceSerial(const char *format, ...)
{
   va_list args;
   uint16_t i;

   va_start(args, format);
   vsnprintf((char *)&bufTrace[0],
             (size_t)sizeof(bufTrace),
             (char const *)format,
             args);
   va_end(args);

   i = 0;
   while (bufTrace[i] != '\0')
   {
      while (USART_GetFlagStatus(USART1, USART_FLAG_TXE) != SET)
         ;
      USART_SendData(USART1, bufTrace[i++]);
   }
}

void sendTraceSerialBinaray(uint8_t *ptr, uint16_t len)
{
   uint16_t i;

   for (i = 0; i < len; i++)
   {
      while (USART_GetFlagStatus(USART1, USART_FLAG_TXE) != SET)
         ;
      USART_SendData(USART1, *ptr++);
   }
}

#endif

/**
  * @brief  
  * @param  ندارد
  * @return ندراد
  */
static void bspSim808Init(void)
{
   GPIO_InitTypeDef GPIO_InitStructure;
   EXTI_InitTypeDef EXTI_Init_Structure;
   NVIC_InitTypeDef NVIC_Init_Structure;

   // Sim808 Reset Pin
   GPIO_PortClock(SIM808_RESET_N_PORT, ENABLE);
   GPIO_InitStructure.GPIO_Pin = SIM808_RESET_N_PIN;
   GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
   GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
   GPIO_Init(SIM808_RESET_N_PORT, &GPIO_InitStructure);
   GPIO_SetBits(SIM808_RESET_N_PORT, SIM808_RESET_N_PIN);

   // Sim808 Power on Pin
   GPIO_PortClock(SIM808_POWER_ON_N_PORT, ENABLE);
   GPIO_InitStructure.GPIO_Pin = SIM808_POWER_ON_N_PIN;
   GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
   GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
   GPIO_Init(SIM808_POWER_ON_N_PORT, &GPIO_InitStructure);
   GPIO_ResetBits(SIM808_POWER_ON_N_PORT, SIM808_POWER_ON_N_PIN);

   // Sim808 Status Pin
   GPIO_PortClock(SIM808_STATUS_PORT, ENABLE);
   GPIO_InitStructure.GPIO_Pin = SIM808_STATUS_PIN;
   GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
   GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
   GPIO_Init(SIM808_STATUS_PORT, &GPIO_InitStructure);

   // Sim808 Interrupt Pin
   GPIO_PortClock(SIM808_INT_PORT, ENABLE);
   GPIO_InitStructure.GPIO_Pin = SIM808_INT_PIN;
   GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
   GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
   GPIO_Init(SIM808_INT_PORT, &GPIO_InitStructure);

   /* فعال سازي کلاک قابليت تغيير کارکرد پين ها */
   RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);

   /* اتصال خط وقفه خارجي */
   GPIO_EXTILineConfig(SIM808_INT_EXTI_PORT, SIM808_INT_EXTI_PIN);

   /* تنظيمات خط وقفه */
   EXTI_Init_Structure.EXTI_Line = SIM808_INT_PIN;
   EXTI_Init_Structure.EXTI_Mode = EXTI_Mode_Interrupt;
   EXTI_Init_Structure.EXTI_Trigger = EXTI_Trigger_Falling;
   EXTI_Init_Structure.EXTI_LineCmd = ENABLE;
   EXTI_Init(&EXTI_Init_Structure);

   /* فعال سازي وقفه */
   NVIC_Init_Structure.NVIC_IRQChannel = EXTI9_5_IRQn;
   NVIC_Init_Structure.NVIC_IRQChannelPreemptionPriority = 1;
   NVIC_Init_Structure.NVIC_IRQChannelSubPriority = 4;
   NVIC_Init_Structure.NVIC_IRQChannelCmd = ENABLE;
   NVIC_Init(&NVIC_Init_Structure);

   bspSerial3Init(SIM808_BAUDRATE);
}

/**
  * @brief  
  * @param  ندارد
  * @return ندراد
  */
void bspSim808Reset(void)
{
   GPIO_ResetBits(SIM808_RESET_N_PORT, SIM808_RESET_N_PIN);
   osDelay(1000);
   GPIO_SetBits(SIM808_RESET_N_PORT, SIM808_RESET_N_PIN);
}

/**
  * @brief  
  * @param  ندارد
  * @return ندراد
  */
void bspSim808PowerOn(void)
{
   GPIO_ResetBits(SIM808_POWER_ON_N_PORT, SIM808_POWER_ON_N_PIN);
   osDelay(2000);
   GPIO_SetBits(SIM808_POWER_ON_N_PORT, SIM808_POWER_ON_N_PIN);
}

/**
  * @brief  
  * @param  
  * @return 
  */
FunctionalState bspSim808ReadStatus(void)
{
   return (GPIO_ReadInputDataBit(SIM808_STATUS_PORT, SIM808_STATUS_PIN));
}

/**
  * @brief  
  * @param  
  * @return 
  */
void bspSim808SendBinaray(uint8_t *ptr, uint16_t len)
{
   uint16_t i;

   for (i = 0; i < len; i++)
   {
      while (USART_GetFlagStatus(USART3, USART_FLAG_TXE) != SET)
         ;
      USART_SendData(USART3, *ptr++);
   }
}

/**
  * @brief  
  * @param  
  * @return 
  */
static void bspSerial3Init(uint32_t baud_rate)
{
   //FlagStatus                 tc_status;
   GPIO_InitTypeDef GPIO_Init_Structure;
   USART_InitTypeDef USART3_Init_Structure;
   NVIC_InitTypeDef NVIC_Init_Structure;

   /* فعال سازي فرکانس پورت مربوطه */
   RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB | RCC_APB2Periph_AFIO, ENABLE);
   /* فعال سازي فرکانس قسمت سريال */
   RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE);

   /* تنظيم پايه فرستنده سريال به عنوان خروجي پوش پول */
   GPIO_Init_Structure.GPIO_Pin = GPIO_Pin_10;
   GPIO_Init_Structure.GPIO_Speed = GPIO_Speed_50MHz;
   GPIO_Init_Structure.GPIO_Mode = GPIO_Mode_AF_PP;
   GPIO_Init(GPIOB, &GPIO_Init_Structure);

   /* تنظيم پايه گيرنده سريال به عنوان ورودي با مقاومت بالاکش */
   GPIO_Init_Structure.GPIO_Pin = GPIO_Pin_11;
   GPIO_Init_Structure.GPIO_Mode = GPIO_Mode_IPU;
   GPIO_Init(GPIOB, &GPIO_Init_Structure);

   /* پيکره بندي ارتباط سريال */
   USART3_Init_Structure.USART_BaudRate = baud_rate;
   USART3_Init_Structure.USART_WordLength = USART_WordLength_8b;
   USART3_Init_Structure.USART_StopBits = USART_StopBits_1;
   USART3_Init_Structure.USART_Parity = USART_Parity_No;
   USART3_Init_Structure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
   USART3_Init_Structure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
   USART_Init(USART3, &USART3_Init_Structure);

   /* فعال سازي ارتباط سريال شماره */
   USART_Cmd(USART3, ENABLE);

   //USART_ITConfig(USART3, USART_IT_TC, ENABLE);
   USART_ITConfig(USART3, USART_IT_RXNE, ENABLE);

   /* فعال سازي وقفه ارتباط سريال شماره 1 */
   NVIC_Init_Structure.NVIC_IRQChannel = USART3_IRQn;
   NVIC_Init_Structure.NVIC_IRQChannelPreemptionPriority = 0;
   NVIC_Init_Structure.NVIC_IRQChannelSubPriority = 4;
   NVIC_Init_Structure.NVIC_IRQChannelCmd = ENABLE;
   NVIC_Init(&NVIC_Init_Structure);
}

/**
  * @brief  
  * @param  
  * @return 
  */
static void bspBuzzerInit(void)
{
   GPIO_InitTypeDef GPIO_InitStructure;

   GPIO_PortClock(LED_BUZZER_PORT, ENABLE);
   GPIO_InitStructure.GPIO_Pin = LED_BUZZER_PIN;
   GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
   GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
   GPIO_Init(LED_BUZZER_PORT, &GPIO_InitStructure);
   GPIO_ResetBits(LED_BUZZER_PORT, LED_BUZZER_PIN);
}

/**
  * @brief  
  * @param  
  * @return 
  */
static void bspLedInit(void)
{
   GPIO_InitTypeDef GPIO_InitStructure;

   /* فعال سازي توان و فرکانس قسمت پشتيان شده با باطري */
   RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR | RCC_APB1Periph_BKP, ENABLE);
   /* فعال سازي دسرسي نوشتن در حافه پشتيان شده با باطري */
   PWR_BackupAccessCmd(ENABLE);

   /* LSE فعال سازي اسيلاتور */
   RCC_LSEConfig(RCC_LSE_ON);

   /* RTC انتخاب منبع کلاک */
   RCC_RTCCLKConfig(RCC_RTCCLKSource_LSE);

   /* RTC قعال سازي کلاک */
   RCC_RTCCLKCmd(ENABLE);

   // LED Green Pin
   GPIO_PortClock(LED_GREEN_PORT, ENABLE);
   GPIO_InitStructure.GPIO_Pin = LED_GREEN_PIN;
   GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
   GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
   GPIO_Init(LED_GREEN_PORT, &GPIO_InitStructure);
   GPIO_ResetBits(LED_GREEN_PORT, LED_GREEN_PIN);
}

/**
  * @brief  
  * @param  
  * @return 
  */
void bspLedGreenOFF(void)
{
   GPIO_ResetBits(LED_GREEN_PORT, LED_GREEN_PIN);
}

/**
  * @brief  
  * @param  
  * @return 
  */
void bspLedGreenON(void)
{
   GPIO_SetBits(LED_GREEN_PORT, LED_GREEN_PIN);
}

/**
  * @brief  
  * @param  
  * @return 
  */
void bspLedGreenToggle(void)
{
   if ((BitAction)GPIO_ReadOutputDataBit(LED_GREEN_PORT, LED_GREEN_PIN) == Bit_RESET)
      GPIO_WriteBit(LED_GREEN_PORT, LED_GREEN_PIN, Bit_SET);
   else
      GPIO_WriteBit(LED_GREEN_PORT, LED_GREEN_PIN, Bit_RESET);
}

/**
  * @brief  
  * @param  
  * @return 
  */
void spiInit(void)
{
   SPI_InitTypeDef SPI_Init_Structure;
   GPIO_InitTypeDef GPIO_Init_Structure;

   // Enable SPI1 and Port clocks
   GPIO_PortClock(FLASH_SPI_PORT, ENABLE);
#if (FLASH_SPI_NUMBER == 1)
   RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1, ENABLE);
#elif (FLASH_SPI_NUMBER == 2)
   RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI2, ENABLE);
#elif (FLASH_SPI_NUMBER == 3)
   RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI3, ENABLE);
#else
#error "please select spi number"
#endif

   // Configure SPI pins: MOSI, MISO and SCK
   GPIO_Init_Structure.GPIO_Pin = FLASH_SPI_MOSI_PIN |
                                  FLASH_SPI_MISO_PIN |
                                  FLASH_SPI_SCK_PIN;
   GPIO_Init_Structure.GPIO_Speed = GPIO_Speed_50MHz;
   GPIO_Init_Structure.GPIO_Mode = GPIO_Mode_AF_PP;
   GPIO_Init(FLASH_SPI_PORT, &GPIO_Init_Structure);

   // Configure Chip select pin
   GPIO_Init_Structure.GPIO_Pin = FLASH_SPI_CS_PIN;
   GPIO_Init_Structure.GPIO_Speed = GPIO_Speed_50MHz;
   GPIO_Init_Structure.GPIO_Mode = GPIO_Mode_Out_PP;
   GPIO_Init(FLASH_SPI_PORT, &GPIO_Init_Structure);
   GPIO_SetBits(FLASH_SPI_PORT, FLASH_SPI_CS_PIN);

   // Reset SPI Interface
#if (FLASH_SPI_NUMBER == 1)
   SPI_I2S_DeInit(SPI1);
#elif (FLASH_SPI_NUMBER == 2)
   SPI_I2S_DeInit(SPI2);
#elif (FLASH_SPI_NUMBER == 3)
   SPI_I2S_DeInit(SPI3);
#else
#error "please select spi number"
#endif

   // SPI configuration
   SPI_Init_Structure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
   SPI_Init_Structure.SPI_Mode = SPI_Mode_Master;
   SPI_Init_Structure.SPI_DataSize = SPI_DataSize_8b;
   SPI_Init_Structure.SPI_CPOL = SPI_CPOL_Low;
   SPI_Init_Structure.SPI_CPHA = SPI_CPHA_1Edge;
   SPI_Init_Structure.SPI_NSS = SPI_NSS_Soft;
   SPI_Init_Structure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_2;
   SPI_Init_Structure.SPI_FirstBit = SPI_FirstBit_MSB;
   SPI_Init_Structure.SPI_CRCPolynomial = 7;
#if (FLASH_SPI_NUMBER == 1)
   SPI_Init(SPI1, &SPI_Init_Structure);
#elif (FLASH_SPI_NUMBER == 2)
   SPI_Init(SPI2, &SPI_Init_Structure);
#elif (FLASH_SPI_NUMBER == 3)
   SPI_Init(SPI3, &SPI_Init_Structure);
#else
#error "please select spi number"
#endif

   // Enable SPI
#if (FLASH_SPI_NUMBER == 1)
   SPI_Cmd(SPI1, ENABLE);
#elif (FLASH_SPI_NUMBER == 2)
   SPI_Cmd(SPI2, ENABLE);
#elif (FLASH_SPI_NUMBER == 3)
   SPI_Cmd(SPI3, ENABLE);
#else
#error "please select spi number"
#endif
}

/**
  * @brief  
  * @param  
  * @return 
  */
uint32_t timeToSecond(uint16_t year,
                      uint8_t month,
                      uint8_t day,
                      uint8_t hour,
                      uint8_t min,
                      uint8_t sec)
{
   time_t now;
   struct tm *ti;

   now = 0;
   ti = localtime(&now);

   if (ti != NULL)
   {
      ti->tm_year = year - 1900;
      ti->tm_mon = month - 1;
      ti->tm_mday = day;
      ti->tm_hour = hour;
      ti->tm_min = min;
      ti->tm_sec = sec;

      return (mktime(ti));
   }

   return 0;
}

/**
  * @brief  
  * @param  ندارد
  * @return ندراد
  */
static void bspOutputInit(void)
{
   GPIO_InitTypeDef GPIO_InitStructure;

   // Output 1 Pin
   GPIO_PortClock(OUTPUT_1_PORT, ENABLE);
   GPIO_InitStructure.GPIO_Pin = OUTPUT_1_PIN;
   GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
   GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
   GPIO_Init(OUTPUT_1_PORT, &GPIO_InitStructure);
   GPIO_ResetBits(OUTPUT_1_PORT, OUTPUT_1_PIN);

   // Output 2 Pin
   GPIO_PortClock(OUTPUT_2_PORT, ENABLE);
   GPIO_InitStructure.GPIO_Pin = OUTPUT_2_PIN;
   GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
   GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
   GPIO_Init(OUTPUT_2_PORT, &GPIO_InitStructure);
   GPIO_ResetBits(OUTPUT_2_PORT, OUTPUT_2_PIN);
}

/**
  * @brief  
  * @param  ندارد
  * @return ندراد
  */
static void bspInputInit(void)
{
   GPIO_InitTypeDef GPIO_InitStructure;
   EXTI_InitTypeDef EXTI_Init_Structure;
   NVIC_InitTypeDef NVIC_Init_Structure;

   // ACC Interrupt Pin ------------------------
   GPIO_PortClock(ACC_INPUT_PORT, ENABLE);
   GPIO_InitStructure.GPIO_Pin = ACC_INPUT_PIN;
   GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
   GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
   GPIO_Init(ACC_INPUT_PORT, &GPIO_InitStructure);

   /* فعال سازي کلاک قابليت تغيير کارکرد پين ها */
   RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);

   /* اتصال خط وقفه خارجي */
   GPIO_EXTILineConfig(ACC_INPUT_EXTI_PORT, ACC_INPUT_EXTI_PIN);

   /* تنظيمات خط وقفه */
   EXTI_Init_Structure.EXTI_Line = ACC_INPUT_PIN;
   EXTI_Init_Structure.EXTI_Mode = EXTI_Mode_Interrupt;
   EXTI_Init_Structure.EXTI_Trigger = EXTI_Trigger_Rising_Falling;
   EXTI_Init_Structure.EXTI_LineCmd = ENABLE;
   EXTI_Init(&EXTI_Init_Structure);

   /* فعال سازي وقفه */
   NVIC_Init_Structure.NVIC_IRQChannel = EXTI1_IRQn;
   NVIC_Init_Structure.NVIC_IRQChannelPreemptionPriority = 0;
   NVIC_Init_Structure.NVIC_IRQChannelSubPriority = 3;
   NVIC_Init_Structure.NVIC_IRQChannelCmd = ENABLE;
   NVIC_Init(&NVIC_Init_Structure);
   // -----------------------------------------

   // Power Interrupt Pin
   GPIO_PortClock(POWER_INPUT_PORT, ENABLE);
   GPIO_InitStructure.GPIO_Pin = POWER_INPUT_PIN;
   GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
   GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
   GPIO_Init(POWER_INPUT_PORT, &GPIO_InitStructure);

   /* فعال سازي کلاک قابليت تغيير کارکرد پين ها */
   RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);

   /* اتصال خط وقفه خارجي */
   GPIO_EXTILineConfig(POWER_INPUT_EXTI_PORT, POWER_INPUT_EXTI_PIN);

   /* تنظيمات خط وقفه */
   EXTI_Init_Structure.EXTI_Line = POWER_INPUT_PIN;
   EXTI_Init_Structure.EXTI_Mode = EXTI_Mode_Interrupt;
   EXTI_Init_Structure.EXTI_Trigger = EXTI_Trigger_Rising_Falling;
   EXTI_Init_Structure.EXTI_LineCmd = ENABLE;
   EXTI_Init(&EXTI_Init_Structure);

   /* فعال سازي وقفه */
   NVIC_Init_Structure.NVIC_IRQChannel = EXTI0_IRQn;
   NVIC_Init_Structure.NVIC_IRQChannelPreemptionPriority = 1;
   NVIC_Init_Structure.NVIC_IRQChannelSubPriority = 2;
   NVIC_Init_Structure.NVIC_IRQChannelCmd = ENABLE;
   NVIC_Init(&NVIC_Init_Structure);
   // -----------------------------------------

   // Door Interrupt Pin
   GPIO_PortClock(DOOR_INPUT_PORT, ENABLE);
   GPIO_InitStructure.GPIO_Pin = DOOR_INPUT_PIN;
   GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
   GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
   GPIO_Init(DOOR_INPUT_PORT, &GPIO_InitStructure);

   /* فعال سازي کلاک قابليت تغيير کارکرد پين ها */
   RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);

   /* اتصال خط وقفه خارجي */
   GPIO_EXTILineConfig(DOOR_INPUT_EXTI_PORT, DOOR_INPUT_EXTI_PIN);

   /* تنظيمات خط وقفه */
   EXTI_Init_Structure.EXTI_Line = DOOR_INPUT_PIN;
   EXTI_Init_Structure.EXTI_Mode = EXTI_Mode_Interrupt;
   EXTI_Init_Structure.EXTI_Trigger = EXTI_Trigger_Rising_Falling;
   EXTI_Init_Structure.EXTI_LineCmd = ENABLE;
   EXTI_Init(&EXTI_Init_Structure);

   /* فعال سازي وقفه */
   NVIC_Init_Structure.NVIC_IRQChannel = EXTI15_10_IRQn;
   NVIC_Init_Structure.NVIC_IRQChannelPreemptionPriority = 2;
   NVIC_Init_Structure.NVIC_IRQChannelSubPriority = 1;
   NVIC_Init_Structure.NVIC_IRQChannelCmd = ENABLE;
   NVIC_Init(&NVIC_Init_Structure);
   // -----------------------------------------
}

/**
  * @brief  
  * @param  ندارد
  * @return ندراد
  */
uint8_t bspAccReadStatus(uint8_t *state)
{
   return (debounceInput(ACC_INPUT_PORT, ACC_INPUT_PIN, state));
}

/**
  * @brief  
  * @param  ندارد
  * @return ندراد
  */
uint8_t bspPowerReadStatus(uint8_t *state)
{
   return (debounceInput(POWER_INPUT_PORT, POWER_INPUT_PIN, state));
}

/**
  * @brief  
  * @param  ندارد
  * @return ندراد
  */
uint8_t bspDoorReadStatus(uint8_t *state)
{
   return (debounceInput(DOOR_INPUT_PORT, DOOR_INPUT_PIN, state));
}

/**
  * @brief  
  * @param  ندارد
  * @return ندراد
  */
static uint8_t debounceInput(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin, uint8_t *state)
{
   uint8_t count = 0;
   uint8_t button_state = 0;
   uint8_t i;

   for (i = 0; i < 100; i++)
   {

      uint8_t current_state = GPIO_ReadInputDataBit(GPIOx, GPIO_Pin);
      if (current_state == button_state)
      {
         count++;
         if (count >= 18)
         {
            *state = (button_state) ? 0 : 1;
            return 1;
         }
      }
      else
      {
         button_state = current_state;
         count = 0;
      }

      osDelay(10);
   }

   return 0;
}

/**
  * @brief  strtok_fixed - fixed variation of strtok_single
  * @param  
  * @return 
  */
char *strtokSingle(char *str, const char *delims)
{
   static char *src = NULL;
   char *p, *ret = 0;

   if (str != NULL)
      src = str;

   if (src == NULL || *src == '\0') // Fix 1
      return NULL;

   ret = src; // Fix 2
   if ((p = strpbrk(src, delims)) != NULL)
   {
      *p = 0;
      src = ++p;
   }
   else
      src += strlen(src);

   return ret;
}

/**
  * @}
  */

/**
  * @}
  */

/*********************************END OF FILE****************************/
