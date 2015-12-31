/**
 ***************************************************************************************************
 * @file	equ_AM23xx.h
 * @author	j.daheron
 * @version	1.0.0
 * @date	2015.10.18
 * @brief   Gestion du capteur Temperature + Humidité.
 ***************************************************************************************************
 */


/* Define to prevent recursive inclusion **********************************************************/

#ifndef EQU_AM23xx_H_
#define EQU_AM23xx_H_


/* Includes ***************************************************************************************/

#include "BSP.h"


Bool_e AM23xx_Read(int16_t* pTemp, uint16_t* pHumid);
void AM23xx_Test(void);


#endif /* EQU_AM23xx_H_ */
