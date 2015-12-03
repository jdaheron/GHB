/**************************************************************************************************/
/*																								  */
/* Parametres																							  */
/*																								  */
/**************************************************************************************************/


/*--------------------------------------------------------------------------------------------------
	INCLUDES
--------------------------------------------------------------------------------------------------*/

#include <FILES/fct_MemoireFAT.h>
#include <FILES/FatFs/ff.h>
#include <FILES/Parametres.h>
#include "util_printf.h"


/*--------------------------------------------------------------------------------------------------
	PRIVATE DEFINE
--------------------------------------------------------------------------------------------------*/


/*--------------------------------------------------------------------------------------------------
	PRIVATE TYPEDEF
--------------------------------------------------------------------------------------------------*/


/*--------------------------------------------------------------------------------------------------
	PRIVATE DATA DECLARATION
--------------------------------------------------------------------------------------------------*/


/*--------------------------------------------------------------------------------------------------
	FUNCTIONS DEFINITIONS
--------------------------------------------------------------------------------------------------*/

Status_e Parametres_EcrireDonnee(IniFile_s* IniFile, uint8_t NumData, void* value);
Status_e Parametres_EcrireFichier(IniFile_s* IniFile);



/*------------------------------------------------------------------------------------------------*/
Status_e Parametres_Init(IniFile_s* IniFile)
{
	//_CONSOLE(LogId, "Init\n");

	// Si erreur d'init du fichier, creation d'un nouveau avec les valeurs par defaut
	if (IniFile_Init(IniFile) == IniFile_StatusKO)
	{
		if (IniFile_OpenWrite(IniFile) == IniFile_StatusKO)
			return Status_KO;

		for (uint32_t i=0; i<IniFile->NbKey; i++)
		{
			IniFile_WriteRawData(&IniFile->KeyTable[i], IniFile->KeyTable[i].Value);
		}
		IniFile_Close(IniFile);
		return Status_Fini;
	}

	return Status_OK;
}


/*------------------------------------------------------------------------------------------------*/
Status_e Parametres_OpenReadFile(IniFile_s* IniFile)
{
	Status_e Statut;


	// Ouverture du fichier
	if (IniFile_OpenRead(IniFile) == IniFile_StatusOK)
		Statut = Status_OK;
	else
		Statut = Status_KO;

	return(Statut);
}


/*------------------------------------------------------------------------------------------------*/
Status_e Parametres_CloseFile(IniFile_s* IniFile)
{
	// Fermeture du fichier
	IniFile_Close(IniFile);

	return(Status_OK);
}


/*------------------------------------------------------------------------------------------------*/
Status_e Parametres_Read(IniFile_s* IniFile, uint32_t NumParam, void* pValue)
{
	Status_e Status;


	// Verification des parametres d'entree
	if (NumParam >= IniFile->NbKey)
		return Status_KO;

	if (pValue == NULL)
		return Status_KO;

	// Lecture dans le fichier
	Status = IniFile_ReadValueFromKey(&IniFile->KeyTable[NumParam], pValue);

	return Status;
}


/*------------------------------------------------------------------------------------------------*/
Status_e Parametres_Write(IniFile_s* IniFile, uint32_t NumParam, void* Value)
{
	Status_e Statut;
	//uint32_t Time = TSW_GetTimestamp_ms();

	// Verification des parametres d'entree
	if (NumParam >= IniFile->NbKey)
		return Status_KO;

	// Ouverture du fichier en ecriture
	if (IniFile_OpenWrite(IniFile) == IniFile_StatusKO)
		return Status_KO;

	// Lecture de l'ensemble de donnees
	for (uint32_t i=0; i<IniFile->NbKey; i++)
	{
		if (IniFile_ReadValueFromKey(&IniFile->KeyTable[i], (void*) &IniFile->CurrentValue[i*INIFILE_DATA_STR_MAX_SIZE]) == Status_KO)
			Statut = Status_KO;
	}

	// Mise a jour des donnees lues avec la donnee passee en parametre
	Parametres_EcrireDonnee(IniFile, NumParam, Value);

	// Ecriture des donnees dans le ficher
	Parametres_EcrireFichier(IniFile);

	// Fermeture du fichier
	IniFile_Close(IniFile);

	// Affichage des parametres
	//Systeme_PrintParam();

	//_CONSOLE( LogId, "Parametres Sauvegardes en %dms\n", TSW_GetTimestamp_ms() - Time );

	return Statut;
}


/*------------------------------------------------------------------------------------------------*/
Status_e Parametres_EcrireDonnee(IniFile_s* IniFile, uint8_t NumData, void* value)
{
	Status_e Status = Status_OK;

	// Verification des parametres d'entree
	if (NumData >= IniFile->NbKey)
		return Status_KO;

	// Ecriture de la donnee
	switch(IniFile->KeyTable[NumData].KeyType)
	{
		case KeyType_Int:
			*((long*) &IniFile->CurrentValue[NumData]) = (long) value;
			break;

		case KeyType_Str:
			strncpy((char*) IniFile->CurrentValue[NumData], (char*) value, INIFILE_DATA_STR_MAX_SIZE);
			break;

		//case KeyType_Float:
		//	ini_putf(pKeyValue[iKey].SectionName, pKeyValue[iKey].KeyName, (float) pKeyValue[iKey].DefaultValue, FileName);
		//	break;

		case KeyType_SectionName:
			strncpy((char*) IniFile->CurrentValue[NumData], (char*) value, INIFILE_DATA_STR_MAX_SIZE);
			break;

		default:
			// Nothing
			break;
	}

	return Status;
}


/*------------------------------------------------------------------------------------------------*/
Status_e Parametres_EcrireFichier(IniFile_s* IniFile)
{
	// Ecriture des valeurs courantes
	for (uint32_t i=0; i<IniFile->NbKey; i++)
	{
		switch(IniFile->KeyTable[i].KeyType)
		{
			case KeyType_SectionName:
				IniFile_WriteRawData(&IniFile->KeyTable[i], (char*) IniFile->KeyTable[i].SectionName);
				break;

			case KeyType_Int:
				IniFile_WriteRawData(&IniFile->KeyTable[i], (const void *) *((long*) &IniFile->CurrentValue[i]));
				break;

			case KeyType_Str:
				IniFile_WriteRawData(&IniFile->KeyTable[i], (char*) &IniFile->CurrentValue[i]);
				break;
		}
	}

	return Status_OK;
}


/*------------------------------------------------------------------------------------------------*/
