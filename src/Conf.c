/**
 ***************************************************************************************************
 * @file	app_Parametres.c
 * @author	j.daheron
 * @version	1.0.0
 * @date	26 août 2014
 * @brief   Gestion des parametres de la carte.
 ***************************************************************************************************
 */


/* Includes ***************************************************************************************/

#include "Conf.h"


/* External Variables *****************************************************************************/


/** @addtogroup app_Parametres
  * @{
  */ 

 
 /** 
 ***************************************************************************************************
 * @defgroup Private_TypesDefinitions Private TypesDefinitions
 * @{
 */

/**
 * @}
 */ 

 
 /** 
 ***************************************************************************************************
 * @defgroup Private_Defines Private Defines
 * @{
 */
 
#define DATA_STR_MAX_SIZE		32


/**
 * @}
 */ 

 
 /** 
 ***************************************************************************************************
 * @defgroup Private_Macros Private Macros
 * @{
 */
  
/**
 * @}
 */ 

  
 /** 
 ***************************************************************************************************
 * @defgroup Private_FunctionPrototypes Private FunctionPrototypes
 * @{
 */

/**
 * @}
 */ 
 
 
  /** 
 ***************************************************************************************************
 * @defgroup Private_Variables Private Variables
 * @{
 */


static char ParamCurrentValue[NB_PARAM][DATA_STR_MAX_SIZE] = {{0},{0}};


/**
 * @}
 */


/**
 ***************************************************************************************************
 * @defgroup Private_Functions Private Functions
 * @{
 */

 /**
 * @}
 */


#define  VALIDATION_APP_PARAMETRES 0

#if VALIDATION_APP_PARAMETRES

#include "util_printf.h"

void FCT_VALIDATION_APP_PARAMETRES()
{
	Status_e Status;
	uint32_t UNIT = 100;
	uint32_t AUTO_OFF = 100;
	uint32_t AUTO_OFF_BT = 100;
	uint32_t LANGUE = 100;
	uint32_t ACTIV_ALS = 100;
	char TEST_STR[256] = {"AZERTY"};

	f_unlink(Param_IniFile.FileName);

	Parametres_Init();

	char buffer[100] = {0};
	strcpy(buffer, Param_IniFile.FileName);
	strcat(buffer, "~\0");

	MemoireFAT_PrintFile(Param_IniFile.FileName);
//	MemoireFAT_PrintFile(buffer);

	Status = Parametres_Read(PARAM_UNIT, &UNIT);
	Status = Parametres_Read(PARAM_AUTO_OFF, &AUTO_OFF);
	Status = Parametres_Read(PARAM_BOARD_SN, &TEST_STR);
/*
	//---------------------------------------
	UNIT = 0;
	Status = Parametres_Write(PARAM_UNIT, (void*) UNIT);
	Status = Parametres_Read(PARAM_UNIT, &UNIT);
	MemoireFAT_PrintFile(Param_IniFile.FileName);

	UNIT = 123;
	Status = Parametres_Write(PARAM_UNIT, (void*) UNIT);
	Status = Parametres_Read(PARAM_UNIT, &UNIT);
	MemoireFAT_PrintFile(Param_IniFile.FileName);

	UNIT = 987;
	Status = Parametres_Write(PARAM_UNIT, (void*) UNIT);
	Status = Parametres_Read(PARAM_UNIT, &UNIT);
	MemoireFAT_PrintFile(Param_IniFile.FileName);
*/
	//---------------------------------------
	AUTO_OFF = 0;
	Status = Parametres_Write(PARAM_AUTO_OFF, (void*) AUTO_OFF);
	Status = Parametres_Read(PARAM_AUTO_OFF, &AUTO_OFF);
	MemoireFAT_PrintFile(Param_IniFile.FileName);

	AUTO_OFF = 12;
	Status = Parametres_Write(PARAM_AUTO_OFF, (void*) AUTO_OFF);
	Status = Parametres_Read(PARAM_AUTO_OFF, &AUTO_OFF);
	MemoireFAT_PrintFile(Param_IniFile.FileName);

	AUTO_OFF = 123;
	Status = Parametres_Write(PARAM_AUTO_OFF, (void*) AUTO_OFF);
	Status = Parametres_Read(PARAM_AUTO_OFF, &AUTO_OFF);
	MemoireFAT_PrintFile(Param_IniFile.FileName);

	AUTO_OFF = 1234;
	Status = Parametres_Write(PARAM_AUTO_OFF, (void*) AUTO_OFF);
	Status = Parametres_Read(PARAM_AUTO_OFF, &AUTO_OFF);
	MemoireFAT_PrintFile(Param_IniFile.FileName);

	AUTO_OFF = 987;
	Status = Parametres_Write(PARAM_AUTO_OFF, (void*) AUTO_OFF);
	Status = Parametres_Read(PARAM_AUTO_OFF, &AUTO_OFF);
	MemoireFAT_PrintFile(Param_IniFile.FileName);


	//---------------------------------------
	strcpy(TEST_STR, "abcdefghi");
	Status = Parametres_Write(PARAM_BOARD_SN, (void*) TEST_STR);
	Status = Parametres_Read(PARAM_BOARD_SN, &TEST_STR);
	MemoireFAT_PrintFile(Param_IniFile.FileName);

	strcpy(TEST_STR, "");
	Status = Parametres_Write(PARAM_BOARD_SN, (void*) TEST_STR);
	Status = Parametres_Read(PARAM_BOARD_SN, &TEST_STR);
	MemoireFAT_PrintFile(Param_IniFile.FileName);

	strcpy(TEST_STR, "012345678901234567890123456789012345678901234567890123456789");
	Status = Parametres_Write(PARAM_BOARD_SN, (void*) TEST_STR);
	Status = Parametres_Read(PARAM_BOARD_SN, &TEST_STR);
	MemoireFAT_PrintFile(Param_IniFile.FileName);


	while (1)
	{
	}
}
#endif

#include "ff.h"
#include "util_printf.h"
#include "fct_MemoireFAT.h"

/**
 ***************************************************************************************************
* Validation OK - JD le 08/09/2014
 */
Status_e
Parametres_Init(
	void
)
{
	//_CONSOLE(LogId, "Init\n");

	// Si erreur d'init du fichier, creation d'un nouveau avec les valeurs par defaut
	if (IniFile_Init(&Param_IniFile) == IniFile_StatusKO)
	{
		if (IniFile_OpenWrite(&Param_IniFile) == IniFile_StatusKO)
			return Status_KO;

		for (int i=0; i<NB_PARAM; i++)
		{
			IniFile_WriteRawData(&Param_IniFile.KeyTable[i], Param_IniFile.KeyTable[i].Value);
		}
		IniFile_Close(&Param_IniFile);
	}

	return Status_OK;
}

/**
 ***************************************************************************************************
 * @todo Validation
 */
Status_e
Parametres_OpenReadFile( void )
{
	Status_e Statut;

	// Ouverture du fichier
	if (IniFile_OpenRead(&Param_IniFile) == IniFile_StatusOK)
		Statut = Status_OK;
	else
		Statut = Status_KO;

	return(Statut);
}

/***************************************************************************************************
* @todo Validation
*/
Status_e
Parametres_CloseFile( void  )
{
	// Fermeture du fichier
	IniFile_Close(&Param_IniFile);

	return(Status_OK);
}

/**
 ***************************************************************************************************
 * Validation OK - JD le 08/09/2014
 */
Status_e
Parametres_Read(Param_Liste_e NumParam, void* pValue)
{
	Status_e Status;

	// Verification des parametres d'entree
	if (NumParam >= NB_PARAM)
		return Status_KO;

	if (pValue == NULL)
		return Status_KO;

	// Lecture dans le fichier
	Status = IniFile_ReadValueFromKey(&Param_IniFile.KeyTable[NumParam], pValue);

	return Status;
}

/**
 ***************************************************************************************************
 * Validation OK - JD le 08/09/2014
 */
Status_e
Parametres_Write(Param_Liste_e NumParam, void* Value)
{
	Status_e Statut;
	uint32_t Time = TSW_GetTimestamp_ms();

	// Verification des parametres d'entree
	if (NumParam >= NB_PARAM)
		return Status_KO;

	// Ouverture du fichier en ecriture
	if (IniFile_OpenWrite(&Param_IniFile) == IniFile_StatusKO)
		return Status_KO;

	// Lecture de l'ensemble de donnees
	for (int i=0; i<NB_PARAM; i++)
	{
		if (IniFile_ReadValueFromKey(&Param_IniFile.KeyTable[i], (void*) &ParamCurrentValue[i][0]) == Status_KO)
			Statut = Status_KO;
	}

	// Mise a jour des donnees lues avec la donnee passee en parametre
	Parametres_EcrireDonnee (NumParam, Value );

	// Ecriture des donnees dans le ficher
	Parametres_EcrireFichier();

	// Fermeture du fichier
	IniFile_Close(&Param_IniFile);

	// Affichage des parametres
	Systeme_PrintParam();

	//_CONSOLE( LogId, "Parametres Sauvegardes en %dms\n", TSW_GetTimestamp_ms() - Time );

	return Statut;
}


/**
 ***************************************************************************************************
 * @todo Validation
 */
Status_e
Parametres_EcrireDonnee(
		uint8_t NumData,	/**<[in] Numero de la donnee.*/
		void* value					/**<[in] Valeur a ecrire (valeur de l'entier ou pointeur vers la chaine).*/
)
{
	Status_e Status = Status_OK;

	// Verification des parametres d'entree
	if (NumData >= NB_PARAM)
		return Status_KO;

	// Ecriture de la donnee
	switch(Param_IniFile.KeyTable[NumData].KeyType)
	{
		case KeyType_Int:
			*((long*) &ParamCurrentValue[NumData][0]) = (long) value;
			break;

		case KeyType_Str:
			strncpy((char*) ParamCurrentValue[NumData], (char*) value, DATA_STR_MAX_SIZE);
			break;

		//case KeyType_Float:
		//	ini_putf(pKeyValue[iKey].SectionName, pKeyValue[iKey].KeyName, (float) pKeyValue[iKey].DefaultValue, FileName);
		//	break;

		case KeyType_SectionName:
		default:
			// Nothing
			break;
	}

	return Status;
}

/**
 ***************************************************************************************************
 * @todo Validation
 */
Status_e
Parametres_EcrireFichier(
	void
)
{
	// Ecriture des valeurs courantes
	for (int i=0; i<NB_PARAM; i++)
	{
		switch(Param_IniFile.KeyTable[i].KeyType)
		{
			case KeyType_SectionName:
				IniFile_WriteRawData(&Param_IniFile.KeyTable[i], (char*) Param_IniFile.KeyTable[i].SectionName);
				break;

			case KeyType_Int:
				IniFile_WriteRawData(&Param_IniFile.KeyTable[i], (const void *) *((long*) &ParamCurrentValue[i][0]));
				break;

			case KeyType_Str:
				IniFile_WriteRawData(&Param_IniFile.KeyTable[i], (char*) &ParamCurrentValue[i][0]);
				break;
		}
	}

	return Status_OK;
}
 /**
 * @}
 */

/* End Of File ************************************************************************************/
