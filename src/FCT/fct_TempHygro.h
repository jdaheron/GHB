/**
 ***************************************************************************************************
 * @file	fct_Temperature.h
 * @author	Julien
 * @version	1.0.0
 * @date	12 oct. 2014
 * @brief   Gestion de la temperature.
 ***************************************************************************************************
 */


/* Define to prevent recursive inclusion **********************************************************/

#ifndef FCT_TEMPERATURE_H_
#define FCT_TEMPERATURE_H_


/* Includes ***************************************************************************************/

#include "BSP.h"
#include "util_TSW.h"


/** 
 ***************************************************************************************************
 * @defgroup Exported_Defines Exported Defines
 * @{
 */

#define TEMPHYGRO_NB_ECH				4
#define TEMPHYGRO_NB_READ_ERROR_MAX		10


 /**
 * @}
 */ 
 
 
/** 
 ***************************************************************************************************
 * @defgroup Exported_TypesDefinitions Exported TypesDefinitions
 * @{
 */


/** Definition breve de la structure. */
typedef struct {

	Bool_e		IsValide;
	uint8_t		NbReadError;
	float		CurrentTemp_DegC;
	float		CurrentHygro_pr100;
	int16_t		TempAcquisition[TEMPHYGRO_NB_ECH];
	int16_t		HygroAcquisition[TEMPHYGRO_NB_ECH];
	uint16_t	iAcquisition;
	uint32_t	PeriodeAcquisition_ms;
	TSW_s		TmrAcquisition;

} TempHygro_s;


/**
 * @}
 */ 
 
 
/** 
 ***************************************************************************************************
 * @defgroup Exported_Macros Exported Macros 
 * @{
 */
 
 /**
 * @}
 */ 
 
 
/** 
 ***************************************************************************************************
 * @defgroup Exported_Variables Exported Variables
 * @{
 */
 
 /**
 * @}
 */ 

 
/** 
 ***************************************************************************************************
 * @defgroup Exported_Functions Exported Functions  
 * @{
 */


void 	TempHygro_Init(uint32_t PeriodeAcquisition_ms);
void 	TempHygro_Thread(void);
Bool_e 	TempHygro_IsValide(void);
float 	TempHygro_GetTemperature(void);
float 	TempHygro_GetHygrometrie(void);

 
 /**
 * @}
 */ 


#endif /* FCT_TEMPERATURE_H_ */
