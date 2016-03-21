/**
 ***************************************************************************************************
 * @file	fct_DatabaseEeprom.h
 * @author	j.daheron
 * @version	1.0.0
 * @date	10 sept. 2014
 * @brief   Gestion de la base de donnees en EEPROM.
 ***************************************************************************************************
 */


/* Define to prevent recursive inclusion **********************************************************/

#ifndef FCT_DATABASEEEPROM_H_
#define FCT_DATABASEEEPROM_H_


/* Includes ***************************************************************************************/

#include "BSP.h"


/** @defgroup fct_DatabaseEeprom fct_DatabaseEeprom
  * @brief Gestion de la base de donnees en EEPROM.
  * @{
  */ 

/** 
 ***************************************************************************************************
 * @defgroup Exported_Defines Exported Defines
 * @{
 */
 
 /**
 * @}
 */ 
 
 
/** 
 ***************************************************************************************************
 * @defgroup Exported_TypesDefinitions Exported TypesDefinitions
 * @{
 */

/** Liste des donnees de la base de donnees. */
typedef enum
{
	DatabaseEeprom_Arrosage = 0,
	DatabaseEeprom_Chauffage,
	DatabaseEeprom_Ethernet,
	DatabaseEeprom_Ventilation,
	DatabaseEeprom_Hygrometrie,
	NB_DatabaseEeprom

} DatabaseEeprom_Data_e;





/** Definition d'une donnee de la base. */
typedef struct
{
	uint32_t	StartAddress;
	uint32_t	Size;
	Bool_e		First_Read_Done;
	Bool_e		UseBkp;
	uint8_t		CurrentSpace;
	void*		pData;
	void*		pDefaultData;

	// A conserver en fin de config
	uint8_t		Num;
	uint8_t		Checksum;

} DatabaseEeprom_s;


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

/**
 * @brief	Initialisation d'une donnee dans la base.
 * @return 	void.
 */
void
DatabaseEeprom_InitData(
		DatabaseEeprom_Data_e	DatabaseEeprom_Data,	/**<[in] Index de la donnees dans la base.*/
		void*					pDefaultData,			/**<[in] Valeurs par defaut.*/
		uint32_t				DataSize				/**<[in] Taille de la donnees dans la base.*/
);

/**
 * @brief	Lecture d'une donnee dans la base.
 * @return 	Status (OK/KO)
 */
Status_e
DatabaseEeprom_Read(
		DatabaseEeprom_Data_e	DatabaseEeprom_Data,	/**<[in] Index de la donnees dans la base.*/
		void*					pData					/**<[out] Pointeur vers les donnees lues.*/
);


/**
 * @brief	Ecriture d'une donnee dans la base.
 * @return 	Status (OK/KO)
 */
Status_e
DatabaseEeprom_Write(
		DatabaseEeprom_Data_e	DatabaseEeprom_Data,	/**<[in] Index de la donnees dans la base.*/
		void*					pData					/**<[out] Pointeur vers les donnees a ecrire.*/
);


void DatabaseEeprom_Display(DatabaseEeprom_Data_e DatabaseEeprom_Data, void* pData);

 /**
 * @}
 */ 
 
 
 /**
 * @}
 */ 


#endif /* FCT_DATABASEEEPROM_H_ */
