/**
  ******************************************************************************
  * @file    eeprom.c
  * @author  Mahdad Ghasemian
  * @version V0.0.0
  * @date    07-April-2018
  * @brief   
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "eeprom.h"
#include "stm32f10x_i2c.h" // Keil::Device:StdPeriph Drivers:I2C
#include "app_cfg.h"
#include "GPIO_STM32F10x.h" // Keil::Device:GPIO
#include "rtx_os.h"         // ARM::CMSIS:RTOS2:Keil RTX5
#include "cmsis_os2.h"      // ::CMSIS:RTOS2
#include "os_tick.h"

/** @defgroup EEPROM 
  * @brief 
  * @{
  */

/** @defgroup EEPROM_Private_Macros
  * @{
  */

#define WAIT_I2C_VALUE 1000

/**
  * @}
  */

/** @defgroup EEPROM_Private_Variables
  * @{
  */

extern osMutexId_t eeprom_mutex_id;

/**
  * @}
  */

/** @defgroup EEPROM_Private_Functions
  * @{
  */

/**
  * @brief  
  * @param  
  * @return 
  */
static int8_t eepromI2CStart(void)
{
   uint16_t tick;
   // Wait until I2C1 is not busy anymore
   //while (I2C_GetFlagStatus(I2C1, I2C_FLAG_BUSY));

   // Generate start condition
   I2C_GenerateSTART(I2C1, ENABLE);

   // Wait for I2C EV5.
   // It means that the start condition has been correctly released
   // on the I2C bus (the bus is free, no other devices is communicating))
   tick = 0;
   while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_MODE_SELECT))
   {
      EEPROM_I2C_DELAY(1);
      if (++tick > WAIT_I2C_VALUE)
         return -1;
   };

   return 0;
}

/**
  * @brief  
  * @param  
  * @return 
  */
static int8_t eepromI2CStop(void)
{
   uint16_t tick;
   // Generate I2C stop condition
   I2C_GenerateSTOP(I2C1, ENABLE);
   // Wait until I2C stop condition is finished
   tick = 0;
   while (I2C_GetFlagStatus(I2C1, I2C_FLAG_STOPF))
   {
      EEPROM_I2C_DELAY(1);
      if (++tick > WAIT_I2C_VALUE)
         return -1;
   };

   return 0;
}

/**
  * @brief  
  * @param  
  * @return 
  */
static int8_t eepromI2CAddressDirection(uint8_t address, uint8_t direction)
{
   uint16_t tick;
   // Send slave address
   I2C_Send7bitAddress(I2C1, address & 0xFE, direction);

   // Wait for I2C EV6
   // It means that a slave acknowledges his address
   if (direction == I2C_Direction_Transmitter)
   {
      tick = 0;
      while (!I2C_CheckEvent(I2C1,
                             I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED))
      {
         EEPROM_I2C_DELAY(1);
         if (++tick > WAIT_I2C_VALUE)
            return -1;
      };
   }
   else if (direction == I2C_Direction_Receiver)
   {
      tick = 0;
      while (!I2C_CheckEvent(I2C1,
                             I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED))
      {
         EEPROM_I2C_DELAY(1);
         if (++tick > WAIT_I2C_VALUE)
            return -1;
      };
   }

   return 0;
}

/**
  * @brief  
  * @param  
  * @return 
  */
static int8_t eepromI2CTransmit(uint8_t byte)
{
   uint16_t tick;
   // Send data byte
   I2C_SendData(I2C1, byte);
   // Wait for I2C EV8_2.
   // It means that the data has been physically shifted out and
   // output on the bus)
   tick = 0;
   while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_TRANSMITTED))
   {
      EEPROM_I2C_DELAY(1);
      if (++tick > WAIT_I2C_VALUE)
         return -1;
   };

   return 0;
}

/**
  * @brief  
  * @param  
  * @return 
  */
static uint8_t eepromI2CReceiveAck(void)
{
   uint16_t tick;
   // Enable ACK of received data
   I2C_AcknowledgeConfig(I2C1, ENABLE);
   // Wait for I2C EV7
   // It means that the data has been received in I2C data register
   tick = 0;
   while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_RECEIVED))
   {
      EEPROM_I2C_DELAY(1);
      if (++tick > WAIT_I2C_VALUE)
         return 0;
   };

   // Read and return data byte from I2C data register
   return I2C_ReceiveData(I2C1);
}

/**
  * @brief  
  * @param  
  * @return 
  */
static uint8_t eepromI2CReceiveNack(void)
{
   uint16_t tick;
   // Disable ACK of received data
   I2C_AcknowledgeConfig(I2C1, DISABLE);
   // Wait for I2C EV7
   // It means that the data has been received in I2C data register
   tick = 0;
   while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_RECEIVED))
   {
      EEPROM_I2C_DELAY(1);
      if (++tick > WAIT_I2C_VALUE)
         return 0;
   };

   // Read and return data byte from I2C data register
   return I2C_ReceiveData(I2C1);
}

/**
  * @brief  
  * @param  
  * @return 
  */
void eepromInit(void)
{
   GPIO_InitTypeDef GPIO_InitStructure;
   I2C_InitTypeDef I2C_InitStructure;

   // Enable I2C peripheral clock
   RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C1, ENABLE);

   I2C_DeInit(I2C1);

   // Setting I2C
   I2C_InitStructure.I2C_ClockSpeed = EEPROM_I2C_SPEED;
   I2C_InitStructure.I2C_Mode = I2C_Mode_I2C;
   I2C_InitStructure.I2C_DutyCycle = I2C_DutyCycle_2;
   I2C_InitStructure.I2C_OwnAddress1 = 0x00;
   I2C_InitStructure.I2C_Ack = I2C_Ack_Enable;
   I2C_InitStructure.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;
   I2C_Init(I2C1, &I2C_InitStructure);
   I2C_Cmd(I2C1, ENABLE);

   // Initialize GPIO as open drain alternate function
   GPIO_PortClock(EEPROM_I2C_PORT, ENABLE);
   GPIO_InitStructure.GPIO_Pin = EEPROM_I2C_SCL_PIN | EEPROM_I2C_SDA_PIN;
   GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_OD;
   GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
   GPIO_Init(EEPROM_I2C_PORT, &GPIO_InitStructure);
}

/**
  * @brief  
  * @param  
  * @return 
  */
int16_t eepromWrite(uint16_t address, uint8_t *data, uint16_t length)
{
   uint16_t cycle, i, len, size, j, total;

   if ((address + length) >= EEPROM_I2C_SIZE)
      return -1;

   i = 0;
   len = length;
   total = length / EEPROM_I2C_SAME_WRITE_CYCLE_SIZE;
   total += (length % EEPROM_I2C_SAME_WRITE_CYCLE_SIZE) ? 1 : 0;
   for (cycle = 0; cycle < total; cycle++)
   {
      if (eepromI2CStart() == -1)
         return -1;
      if (eepromI2CAddressDirection(EEPROM_I2C_ADDRESS, I2C_Direction_Transmitter) == -1)
         return -1;
      if (eepromI2CTransmit((address + (cycle * EEPROM_I2C_SAME_WRITE_CYCLE_SIZE)) >> 8) == -1)
         return -1;
      if (eepromI2CTransmit((address + (cycle * EEPROM_I2C_SAME_WRITE_CYCLE_SIZE))) == -1)
         return -1;
      size = (len > EEPROM_I2C_SAME_WRITE_CYCLE_SIZE) ? EEPROM_I2C_SAME_WRITE_CYCLE_SIZE : len;
      for (j = 0; j < size; j++)
         if (eepromI2CTransmit(data[i++]) == -1)
            return -1;
      len -= size;
      if (eepromI2CStop() == -1)
         return -1;
      EEPROM_I2C_DELAY(100);
   }
   return 0;
}

/**
  * @brief  
  * @param  
  * @return 
  */
int16_t eepromRead(uint16_t address, uint8_t *data, uint16_t length)
{
   uint16_t i;

   if ((address + length) >= EEPROM_I2C_SIZE)
      return -1;

   if (eepromI2CStart() == -1)
      return -1;
   if (eepromI2CAddressDirection(EEPROM_I2C_ADDRESS, I2C_Direction_Transmitter) == -1)
      return -1;
   if (eepromI2CTransmit(address >> 8) == -1)
      return -1;
   if (eepromI2CTransmit(address) == -1)
      return -1;
   if (eepromI2CStart() == -1)
      return -1;
   if (eepromI2CAddressDirection(EEPROM_I2C_ADDRESS, I2C_Direction_Receiver) == -1)
      return -1;
   for (i = 0; i < length; i++)
   {
      if (i + 1 == length)
         data[i] = eepromI2CReceiveNack();
      else
         data[i] = eepromI2CReceiveAck();
   }
   if (eepromI2CStop() == -1)
      return -1;

   return 0;
}

/**
  * @brief  
  * @param  ندارد
  * @return ندراد
  */
void eepromLock(void)
{
   osStatus_t status;

   if (eeprom_mutex_id != NULL)
   {
      status = osMutexAcquire(eeprom_mutex_id, osWaitForever);
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
void eepromUnLock(void)
{
   osStatus_t status;

   if (eeprom_mutex_id != NULL)
   {
      status = osMutexRelease(eeprom_mutex_id);
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
