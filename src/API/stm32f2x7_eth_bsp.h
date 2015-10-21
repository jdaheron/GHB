/**
  ******************************************************************************
  * @file    stm32f2x7_eth_bsp.h
  * @author  MCD Application Team
  * @version V1.1.0
  * @date    07-October-2011 
  * @brief   Header for stm32f2x7_eth_bsp.c file.
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __STM32F2x7_ETH_BSP_H
#define __STM32F2x7_ETH_BSP_H


/* Includes ------------------------------------------------------------------*/

#include "BSP.h"

//#include "stm322xg_eval.h"
//#include "stm322xg_eval_lcd.h"

/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/

/* PHY registers */
//	#define PHY_MICR                  0x11 /* MII Interrupt Control Register */
//	#define PHY_MICR_INT_EN           ((uint16_t)0x0002) /* PHY Enable interrupts */
//	#define PHY_MICR_INT_OE           ((uint16_t)0x0001) /* PHY Enable output interrupt events */
//	#define PHY_MISR                  0x12 /* MII Interrupt Status and Misc. Control Register */
//	#define PHY_MISR_LINK_INT_EN      ((uint16_t)0x0020) /* Enable Interrupt on change of link status */
//	#define PHY_LINK_STATUS           ((uint16_t)0x2000) /* PHY link status interrupt mask */


#define KS8721_PHY_ADDRESS				0x15 /* Relative to Olimex STM32-P207 Board */


/* Exported macro ------------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */
void  ETH_BSP_Config(void);
uint32_t Eth_Link_PHYITConfig(uint16_t PHYAddress);
void Eth_Link_EXTIConfig(void);
void Eth_Link_ITHandler(uint16_t PHYAddress);


#endif /* __STM32F2x7_ETH_BSP_H */

