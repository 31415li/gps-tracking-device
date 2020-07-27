/**
  ******************************************************************************
  * @file    Position_Thread.h
  * @author  Mahdad Ghasemian
  * @version V0.0.0
  * @date    3-March-2018
  * @brief   
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __POSITION_THREAD_H
#define __POSITION_THREAD_H

#ifdef __cplusplus
extern "C"
{
#endif

  /* Includes ------------------------------------------------------------------*/

  /** @defgroup POSITION_THREAD
  * @{
  */

  /** @defgroup POSITION_THREAD_Exported_Types
  * @{
  */

  /**
  * @}
  */

  /** @defgroup POSITION_THREAD_Exported_Variables
  * @{
  */

  /**
  * @}
  */

  /** @defgroup POSITION_THREAD_Exported_Functions
  * @{
  */

  void Position_ObjCreate(void);
  void Position_ThreadCreate(void);
  void clearCheckMovingIntervals(void);

  /**
  * @}
  */

  /**
  * @}
  */

#ifdef __cplusplus
}
#endif

#endif /* __POSITION_THREAD_H */

/*********************************END OF FILE****************************/
