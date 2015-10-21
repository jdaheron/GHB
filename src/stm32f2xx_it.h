/**
  ******************************************************************************
  * @file    Project/Template/stm32f10x_it.h
  * @author  MCD Application Team
  * @version  V3.0.0
  * @date    04/06/2009
  * @brief   This file contains the headers of the interrupt handlers.
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __STM32F20x_IT_H
#define __STM32F20x_IT_H


/* Includes ------------------------------------------------------------------*/
#include "stm32f2xx.h"


/* Exported types ------------------------------------------------------------*/


/* Exported constants --------------------------------------------------------*/


/* Exported macro ------------------------------------------------------------*/


/* Exported functions ------------------------------------------------------- */
void NMI_Handler(void);
void HardFault_Handler(void);
void MemManage_Handler(void);
void BusFault_Handler(void);
void UsageFault_Handler(void);
void SVC_Handler(void);
void DebugMon_Handler(void);
void PendSV_Handler(void);
void SysTick_Handler(void);


#endif /* __STM32F20x_IT_H */
