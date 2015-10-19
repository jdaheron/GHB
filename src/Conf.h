/**
 ***************************************************************************************************
 * @file	app_Parametres.h
 * @author	j.daheron
 * @version	1.0.0
 * @date	26 août 2014
 * @brief   Gestion des parametres de la carte.
 ***************************************************************************************************
 */


/* Define to prevent recursive inclusion **********************************************************/

#ifndef APP_PARAMETRES_H_
#define APP_PARAMETRES_H_


/* Includes ***************************************************************************************/

#include "BSP.h"
#include "fct_IniFile.h"


/** @defgroup app_Parametres app_Parametres
  * @brief Gestion des parametres de la carte.
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

/** Liste des paramatres. */
typedef enum
{

	START_TEMPO_s				= 0,

	CH_SEUIL_START_DegC,
	CH_SEUIL_STOP_DegC,
	CH_TEMPO_APRES_CH_s,

	EXT_SEUIL_START_DegC,
	EXT_SEUIL_STOP_DegC,
	EXT_TEMPO_APRES_EXT_s,
	EXT_ACTIVER_PENDANT_CH,

	LOG_PERIODE_s,
	LOG_PERIODE_PENDANT_ACTION_s,

	NB_PARAM,
	
} Param_Liste_e;





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

static const Key_s ParamDefaultValue[] =
{
		//KeyType		SectionName		KeyName						Value				MinValue			MaxValue
		{KeyType_Int,	"START",		"Tempo_s",					(void*) (30),		(void*) (0),		(void*) (3600)				},

		{KeyType_Int,	"CHAUFFAGE",	"SeuilStart_DegC",			(void*) (22),		(void*) (10),		(void*) (50)				},
		{KeyType_Int,	"CHAUFFAGE",	"SeuilStop_DegC",			(void*) (25),		(void*) (10),		(void*) (50)				},
		{KeyType_Int,	"CHAUFFAGE",	"TempoApresCh_s",			(void*) (30),		(void*) (0),		(void*) (3600)				},

		{KeyType_Int,	"EXTRACT",		"SeuilStart_DegC",			(void*) (26),		(void*) (10),		(void*) (50)				},
		{KeyType_Int,	"EXTRACT",		"SeuilStop_DegC",			(void*) (24),		(void*) (10),		(void*) (50)				},
		{KeyType_Int,	"EXTRACT",		"TempoApresEXT_s",			(void*) (15),		(void*) (0),		(void*) (3600)				},
		{KeyType_Int,	"EXTRACT",		"ActiverPendantCh",			(void*) (0),		(void*) (0),		(void*) (1)					},

		{KeyType_Int,	"LOG",			"Periode_s",				(void*) (60),		(void*) (5),		(void*) (3600)				},
		{KeyType_Int,	"LOG",			"PeriodePendantAction_s",	(void*) (60),		(void*) (5),		(void*) (3600)				},
};

static const IniFile_s Param_IniFile = {
		.FileName	= "PARAM.ini",
		.KeyTable	= ParamDefaultValue,
		.NbKey		= NB_PARAM,
};

 /**
 * @}
 */ 

 
/** 
 ***************************************************************************************************
 * @defgroup Exported_Functions Exported Functions  
 * @{
 */

void FCT_VALIDATION_APP_PARAMETRES();

/**
 * @brief	Initialisation des parametre.
 * @return 	void
 */
Status_e
Parametres_Init(
	void
);

/**
 * @brief	Ouverture du fichier de parametre
 * @return 	Status (OK/KO)
 */
Status_e
Parametres_OpenReadFile(
	void
);

/**
 * @brief	Fermeture du fichier de parametre
 * @return 	Status (OK/KO)
 */
Status_e
Parametres_CloseFile(
	void
);

/**
 * @brief	Lecture d'un parametre.
 * @return 	Status (OK/KO)
 */
Status_e
Parametres_Read(
		Param_Liste_e NumParam,	/**<[in] Numero du parametre.*/
		void* pValue			/**<[out] Valeur lue.*/
);

/**
 * @brief	Ecriture d'un parametre.
 * @return 	Status (OK/KO)
 */
Status_e
Parametres_Write(
		Param_Liste_e NumParam,	/**<[in] Numero du parametre.*/
		void* Value				/**<[in] Valeur a ecrire.*/
);

/**
 * @brief	Ecriture d'une donnee dans la Table des donnees courante.
 * @return 	Status (OK/KO)
 */
Status_e
Parametres_EcrireDonnee(
		uint8_t	 	NumData,	/**<[in] Numero de la donnee.*/
		void* 		value		/**<[in] Valeur a ecrire (valeur de l'entier ou pointeur vers la chaine).*/
);

/**
 * @brief	Ecriture d'un  fichier de donnees Table.
 * @return 	Status (OK/KO)
 */
Status_e
Parametres_EcrireFichier(
	void
);
 /**
 * @}
 */


 /**
 * @}
 */


#endif /* APP_PARAMETRES_H_ */

