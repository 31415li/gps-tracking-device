/**
  ******************************************************************************
  * @file    includes.h
  * @author  Mahdad Ghasemian
  * @version V0.0.1
  * @date    18-February-2018
  * @brief   
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __INCLUDES_H
#define __INCLUDES_H

#ifdef __cplusplus
extern "C"
{
#endif

  /* Includes ------------------------------------------------------------------*/

  /** @defgroup INCLUDES 
  * @{
  */

  /** @defgroup INCLUDES_Exported_Macros
  * @{
  */

  /**
  * @}
  */

  /** @defgroup INCLUDES_Exported_Types
  * @{
  */

  typedef union {
    uint8_t byte[2];
    uint16_t self;
  } valueUint16;

  typedef union {
    uint8_t byte[4];
    uint32_t self;
  } valueUint32;

  typedef union {
    uint8_t byte[8];
    uint64_t self;
  } valueUint64;

  typedef struct
  {
    uint8_t type;
    uint16_t len;
    uint8_t *data;
  } comminucation_t;

  /**
  * @}
  */

  /** @defgroup INCLUDES_Exported_Variables
  * @{
  */

  /**
  * @}
  */

  /**
  * @}
  */

#ifdef __cplusplus
}
#endif

#endif /* __BSP_H */

/*********************************END OF FILE****************************/
