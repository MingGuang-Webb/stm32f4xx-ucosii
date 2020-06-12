/*
 * @Author: your name
 * @Date: 2020-06-07 11:16:14
 * @LastEditTime: 2020-06-07 11:21:41
 * @LastEditors: your name
 * @Description: In User Settings Edit
 * @FilePath: \stm32f767-uscoii\App\app_main.h
 */ 
/**
  ******************************************************************************
  * File Name          : gpio.h
  * Description        : This file contains all the functions prototypes for 
  *                      the gpio  
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2020 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __APP_MAIN_H
#define __APP_MAIN_H
#ifdef __cplusplus
 extern "C" {
#endif
#include "ucos_ii.h"
/* Includes ------------------------------------------------------------------*/
void create_app_task(void);
#ifdef __cplusplus
}
#endif
#endif /*__ pinoutConfig_H */

/**
  * @}
  */

/**
  * @}
  */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
