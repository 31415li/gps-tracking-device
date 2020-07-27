/**
  ******************************************************************************
  * @file    input.h
  * @author  Mahdad Ghasemian
  * @version V0.0.1
  * @date    28-April-2018
  * @brief   
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __INPUT_H
#define __INPUT_H

#ifdef __cplusplus
extern "C"
{
#endif

/* Includes ------------------------------------------------------------------*/
#include "app_cfg.h"

  /** @defgroup INPUT
  * @{
  */

  /** @defgroup INPUT_Exported_Macros
  * @{
  */

  /**
  * @}
  */

  /** @defgroup INPUT_Exported_Types
  * @{
  */

  typedef enum
  {
    INPUT_READ_EVENT = 0,
    INPUT_NEW_EVENT
  } INPUT_New_Event_t;

  /**
  * @}
  */

  /** @defgroup INPUT_Exported_Variables
  * @{
  */

  /**
  * @}
  */

  /** @defgroup INPUT_Exported_Functions
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

#endif /* __INPUT_H */

/*********************************END OF FILE****************************/
