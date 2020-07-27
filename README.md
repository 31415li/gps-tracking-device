# GpsTrackinDevice

Gps program for stm32 (RTX-CMSIS)

- Chips

  - STM32F103C8T6 (MCU)
  - SIM808 (GPS/GSM/GPRS - SIMCOM)
  - AT24C32D (I2C Serial Eeprom)
  - AT45DB161E (SPI Serial Flash)

- USER (folder)

  - aes (aes128 encription/decription library)
  - bsp (board support package - init functions)
  - eeprom (eeprom driver AT24C32D)
  - flash (flash dirver for AT45DB161E)
  - input (input optical driver)
  - record (library for init/write/read a circular record space - record avl/event data)
  - sim808 (driver for manage sim808 compatible with RTX - and response to SMS commands)
  - system (driver for manage device settings)
  - thread (all thread)
    - App thread
    - Position thread
    - Sim808 thread
  - app_cfs.h file (all config project)

- Thread's
  - App thread : check inputs and system
  - Position thread : get position detail from sim808 and record in record space
  - Sim808 thread : manage sim808 for always on and setting it
