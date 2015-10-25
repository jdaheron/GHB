/**
 ***************************************************************************************************
 * @file	fct_IniFile.C
 * @author	j.daheron
 * @version	1.0.0
 * @date	28 août 2014
 * @brief   Gestion de fichier INI.
 ***************************************************************************************************
 */

/**
 ***************************************************************************************************
 * NOTE:
 * 		Pour l'utilisation des fichiers Ini 2 types d'utilisation se distinguent :
 *
 * 		- fichiers courts sans section.
 * 		- fichiers volumineux avec section.
 *
 *		Pour les fichiers courts sans section:
 *		 -> la lecture se fait via les appels suivants:
 *			- IniFile_Init() (si besoin de recreer le fichier)
 *			- IniFile_OpenRead() ( IniFile_OpenReadIfExist suffit aussi largement !)
 *			- IniFile_ReadValueFromKey() (sur les clefs a lire)
 *					 /!\ Attention pour des raisons d'optimisations, les clefs doivent etre
 *					 appelees dans le meme ordre que le fichiers. /!\
 *			- IniFile_Close()
 *
 *		 -> l'ecriture se fait via les appels suivants:
 *		 	- IniFile_OpenWriteForSingleWrite()
 *		 	- IniFile_WriteValueFromKey() ( s'appui sur les fonctions de la lib minIni )
 *		 	- IniFile_Close()
 *
 *		Pour les fichiers volumineux avec section:
 *		 -> la lecture se fait via les appels suivants:
 *			- IniFile_Init() (si besoin de recreer le fichier)
 *			- IniFile_OpenReadIfExist()
 *			- IniFile_ReadValueFromKey() (sur les clefs a lire)
 *					 /!\ Attention pour des raisons d'optimisations, les clefs doivent etre
 *					 appelees dans le meme ordre que le fichiers. /!\
 *			- IniFile_Close()
 *
 *		 -> l'ecriture se fait via les appels suivants:
 *		 	- IniFile_OpenWrite()
 *		 	- Boucle de lecture sur l'ensemble des clefs du fichier via IniFile_ReadValueFromKey()
 *		 	- Mise a jour / synchro de ces donnees avec celles a ecrire.
 *		 	- Boucle d'ecritrue dans le fichier de l'ensemble des clefs du fichier via IniFile_WriteRawData()
 *		 	- IniFile_Close()
 *
 *		Toutes ces fonctions sont bloquantes.
 *		Une ouverture de fichier, via :
 *			- IniFile_OpenRead()
 *			- IniFile_OpenReadIfExist()
 *			- IniFile_OpenWrite()
 *			- IniFile_OpenWriteForSingleWrite()
 *		verouille l'access a une deuxièmre ouverture de fichiers, un IniFile_Close() est imperatif
 *
 ***************************************************************************************************
 */

/* Includes ***************************************************************************************/

#include "fct_IniFile.h"

#include "util_TSW.h"
//#include "util_CONSOLE.h"
#include "util_printf.h"

/* External Variables *****************************************************************************/


/** @addtogroup fct_IniFile
  * @{
  */ 

 
 /** 
 ***************************************************************************************************
 * @defgroup Private_Defines Private Defines
 * @{
 */

#define MAX_STR_SIZE	100
#define MODE_READ		0
#define MODE_WRITE		1

#define LogId			"IniFile"
//#define	DEBUG_LOG		NO_LOG

#define MAX_STRING_LENGTH	255
#define DATA_STR_MAX_SIZE	32
/**
 * @}
 */

 /**
 ***************************************************************************************************
 * @defgroup Private_TypesDefinitions Private TypesDefinitions
 * @{
 */

typedef struct {
    char     		pName[MAX_STRING_LENGTH];
    char     		pTempName[MAX_STRING_LENGTH];
    FIL      		File;
    FIL 	 		FileTmp;
    uint8_t			NbFichierOuvert;
    Bool_e			NewFile;
    unsigned long	Position;
    unsigned long	PositionSaved;
    char     		pSection[MAX_STRING_LENGTH];
    char 			Line[MAX_STRING_LENGTH];

}IniFileData_s;

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
Bool_e
IsFileValid(
	const IniFile_s*	IniFile		/**<[in] Fichier Ini.*/
);

Bool_e
IniFile_IsKeyExist(
	const char * pSection,
	const char *pKey
);

int
IniFile_ReadLine(
	char * pLine
);

void IniFile_SavePosition( void );
void IniFile_RestorePosition( void );

char * 		IniFile_SearchValue( const char * pSection, const char *pKey );
char * 		IniFile_ReadValueFromLine( void );
Status_e	IniFile_WriteValueInCurrentLine( const char * pValue );
Bool_e 		IniFile_ReadSectionIfPresent( char * pLine );
char *      IniFile_ReadKey( char * pLine, int * length);
char *		IniFile_ReadValue( char * pLine );
Status_e	IniFile_WriteValue( char * pLine, const char * pValues );

/**
 * @}
 */ 
 
 
  /** 
 ***************************************************************************************************
 * @defgroup Private_Variables Private Variables
 * @{
 */
 
//static FIL File;
static IniFileData_s IniData = {
		.pName				= "",
		.pTempName			= "",
		.NbFichierOuvert 	= 0,
		.NewFile			= FALSE,
		.Position			= 0,
		.PositionSaved		= 0,
		.pSection			= "",
		.Line				= "",
};

// static FIL FileTmp;
static int Mode = 0;
static Bool_e FileModified = FALSE;
static FIL* pCurrentWrFile;
static FIL* pCurrentRdFile;

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
 
/**
 ***************************************************************************************************
 * Validation OK - JD le 28/08/2014
 */
Bool_e
IsKeyValid(
		Key_s*	pKey	/**<[in] Cle a controler.*/
)
{
	Bool_e IsValid = FALSE;
	uint8_t car;

	switch(pKey->KeyType)
	{
		case KeyType_Int:
			if (pKey->MaxValue <  pKey->MinValue)
			{
				if (((int) pKey->Value <= (int)pKey->MaxValue) && ((int)pKey->Value >= (int)pKey->MinValue))
					IsValid = TRUE;
			}
			else
			{
				if ((pKey->Value <= pKey->MaxValue) && (pKey->Value >= pKey->MinValue))
					IsValid = TRUE;
			}
			break;

		case KeyType_Str:
			car = *((uint8_t*) pKey->Value);
			if (car <= 127)
				IsValid = TRUE;
			break;

		case KeyType_SectionName:
		default:
			break;
		//case KeyType_Float:
		//	ini_putf(pKeyValue[iKey].SectionName, pKeyValue[iKey].KeyName, (float) pKeyValue[iKey].DefaultValue, FileName);
		//	break;
	}

	return IsValid;
}

/** 
 ***************************************************************************************************
 * Validation OK - JD le 28/08/2014
 */
Bool_e
IsFileValid(
		const IniFile_s*	IniFile		/**<[in] Fichier Ini.*/
)
{
	Bool_e IsValid = TRUE;
	char buff[MAX_STR_SIZE];
	Key_s Key;

	for (uint32_t iKey=0; iKey<IniFile->NbKey; iKey++)
	{
		// Copie de la valeur par defaut de la cle en local
		memcpy(&Key, &(IniFile->KeyTable[iKey]), sizeof(Key_s));

		// Lecture de la valeur presente dans le fichier
		if (IniFile_ReadValueFromKey(&Key, buff) == Status_KO)
		{
			IsValid = FALSE;
			break;
		}

		// Verification de la validite de la cle
		if (Key.KeyType == KeyType_Str)
			Key.Value = &buff;

		if (IsKeyValid(&Key) == FALSE)
		{
			IsValid = FALSE;
			break;
		}
	}

	return IsValid;
}

/**
 ***************************************************************************************************
 * Validation OK - JD le 28/08/2014
 */
IniFile_Status_e
IniFile_Init(
		const IniFile_s*	IniFile		/**<[in] Descripteur du contenu du fichier.*/
)
{
	IniFile_Status_e Status = IniFile_StatusOK;

	// Ouverture du fichier, creation si besoin
	if (IniFile_OpenWrite(IniFile) == IniFile_StatusKO)
		return IniFile_StatusKO;

	// Verification du contenu du fichier
	if (IsFileValid(IniFile) == FALSE)
		Status = IniFile_StatusKO;

	IniFile_Close(IniFile);

	return Status;
}

/**
 ***************************************************************************************************
 * Validation OK - JD le 01/09/2014
 */
IniFile_Status_e
IniFile_OpenRead(
		const IniFile_s*	IniFile		/**<[in] Descripteur du contenu du fichier.*/
)
{
	IniFile_Status_e Status = IniFile_StatusOK;

	// Verif presence repertoire, creation si besoin
	if (MemoireFAT_IsDirectoryPresent((char*) IniFile->FileName, TRUE) == MemFAT_Error)
		return IniFile_StatusKO;

	// Ouverture du fichier, creation si besoin
	switch (MemoireFAT_IsFilePresent(&IniData.File, (char*) IniFile->FileName, FA_READ | FA_OPEN_EXISTING, TRUE, TRUE))
	{
		case MemFAT_OK:
			//_CONSOLE( DEBUG_LOG, "File present (%s)\n", IniFile->FileName);
			strcpy( IniData.pName, IniFile->FileName);
			IniData.pName[ strlen(IniFile->FileName)] = '\0';
			IniData.Position = 0;
			IniData.PositionSaved = 0;
			IniData.NbFichierOuvert++;
			Status = IniFile_StatusOK;
			break;

		case MemFAT_FileCreated:
			//_CONSOLE( LogId, "File created (%s)\n", IniFile->FileName);
			strcpy( IniData.pName, IniFile->FileName);
			IniData.pName[ strlen(IniFile->FileName)] = '\0';
			IniData.Position = 0;
			IniData.PositionSaved = 0;
			IniData.NbFichierOuvert++;
			Status = IniFile_StatusFileCreated;
			break;

		case MemFAT_Error:
		default:
			return IniFile_StatusKO;
	}

	Mode = MODE_READ;

	return Status;
}

/**
 ***************************************************************************************************
 * Validation OK - JD le 01/09/2014
 */
IniFile_Status_e
IniFile_OpenReadIfExist(
		const IniFile_s*	IniFile		/**<[in] Descripteur du contenu du fichier.*/
)
{
	IniFile_Status_e Status = IniFile_StatusOK;

	// Verif presence repertoire, creation si besoin
	if (MemoireFAT_IsDirectoryPresent((char*) IniFile->FileName, TRUE) == MemFAT_Error)
		return IniFile_StatusKO;

	// Ouverture du fichier
	switch (MemoireFAT_IsFilePresent(&IniData.File, (char*) IniFile->FileName, FA_READ | FA_OPEN_EXISTING, FALSE, TRUE))
	{
		case MemFAT_OK:
			//_CONSOLE( DEBUG_LOG, "File present (%s)\n", IniFile->FileName);
			strcpy( IniData.pName, IniFile->FileName);
			IniData.pName[ strlen(IniFile->FileName)] = '\0';
			IniData.Position = 0;
			IniData.PositionSaved = 0;
			IniData.NbFichierOuvert++;
			Status = IniFile_StatusOK;
			break;

		case MemFAT_FileCreated:
			//_CONSOLE( LogId, "File created (%s)\n", IniFile->FileName);
			Status = IniFile_StatusKO;
			break;

		case MemFAT_Error:
		default:
			return IniFile_StatusKO;
	}

	Mode = MODE_READ;

	return Status;
}


/**
 ***************************************************************************************************
 * Validation OK - JD le 01/09/2014
 */
IniFile_Status_e
IniFile_OpenWrite(
		const IniFile_s*	IniFile		/**<[in] Descripteur du contenu du fichier.*/
)
{
	IniFile_Status_e Status = IniFile_StatusOK;

	// Verif presence repertoire, creation si besoin
	if (MemoireFAT_IsDirectoryPresent((char*) IniFile->FileName, TRUE) == MemFAT_Error)
	{
		//_CONSOLE( LogId, "**ERROR** MemoireFAT_IsDirectoryPresent KO %s\n", IniFile->FileName );
		return IniFile_StatusKO;
	}

	// Ouverture du fichier, creation si besoin
	switch (MemoireFAT_IsFilePresent(&IniData.File, (char*) IniFile->FileName, FA_READ | FA_WRITE, TRUE, TRUE))
	{
		case MemFAT_OK:
			strcpy( IniData.pName, IniFile->FileName);
			IniData.pName[ strlen(IniFile->FileName)] = '\0';
			IniData.Position = 0;
			IniData.PositionSaved = 0;
			IniData.NewFile = FALSE;
			IniData.NbFichierOuvert++;
			Status = IniFile_StatusOK;
			break;

		case MemFAT_FileCreated:
			strcpy( IniData.pName, IniFile->FileName);
			IniData.pName[ strlen(IniFile->FileName)] = '\0';
			IniData.Position = 0;
			IniData.PositionSaved = 0;
			IniData.NewFile = TRUE;
			IniData.NbFichierOuvert++;
			Status = IniFile_StatusFileCreated;
			break;

		case MemFAT_Error:
		default:
			//_CONSOLE( LogId, "**ERROR** MemoireFAT_IsFilePresent KO %s\n", IniFile->FileName );
			return IniFile_StatusKO;
	}

	if ( IniData.NewFile == FALSE )
	{
		// --- Creation d'un fichier temporaire, si le fichier existe deja
		strcpy(IniData.pTempName, IniFile->FileName);
		strcat(IniData.pTempName, "~\0");
		switch (MemoireFAT_IsFilePresent(&IniData.FileTmp, IniData.pTempName, FA_READ | FA_WRITE, TRUE, TRUE))
		{
			case MemFAT_OK:
				Status = IniFile_StatusOK;
				break;

			case MemFAT_FileCreated:
				Status = IniFile_StatusFileCreated;
				break;

			case MemFAT_Error:
			default:
				//_CONSOLE( LogId, "**ERROR** MemoireFAT_IsFilePresent KO %s\n", IniData.pTempName );
				return IniFile_StatusKO;
		}

		pCurrentRdFile = &IniData.File;
		pCurrentWrFile = &IniData.FileTmp;
	}
	else
		pCurrentWrFile = &IniData.File;

	FileModified = FALSE;
	Mode = MODE_WRITE;

	return Status;
}

/**
 ***************************************************************************************************
 * Validation OK - JD le 01/09/2014
 */
void
IniFile_Close(
		const IniFile_s*	IniFile		/**<[in] Descripteur du contenu du fichier.*/
)
{
	// Fermeture des fichiers
	f_close(&IniData.File);
	f_close(&IniData.FileTmp);


	if ((Mode == MODE_WRITE) && (FileModified == TRUE))
	{
		if (pCurrentWrFile == &IniData.FileTmp)
		{
			f_unlink(IniFile->FileName);

			f_rename(IniData.pTempName, IniFile->FileName);
		}
	}
	f_unlink(IniData.pTempName);

	Mode = MODE_READ;

	IniData.NbFichierOuvert--;
}


/**
 ***************************************************************************************************
 * @todo Validation
 */
IniFile_Status_e
IniFile_OpenWriteFile(
		const IniFile_s*	IniFile		/**<[in] Descripteur du contenu du fichier.*/
)
{
	IniFile_Status_e Status = IniFile_StatusOK;

	// Verif presence repertoire, creation si besoin
	if (MemoireFAT_IsDirectoryPresent((char*) IniFile->FileName, TRUE) == MemFAT_Error)
	{
		_CONSOLE( LogId, "**ERROR** MemoireFAT_IsDirectoryPresent KO %s\n", IniFile->FileName );
		return IniFile_StatusKO;
	}

	// Ouverture du fichier, creation si besoin
	switch (MemoireFAT_IsFilePresent(&IniData.File, (char*) IniFile->FileName, FA_READ | FA_WRITE, TRUE, TRUE))
	{
		case MemFAT_OK:
			strcpy( IniData.pName, IniFile->FileName);
			IniData.pName[ strlen(IniFile->FileName)] = '\0';
			IniData.Position = 0;
			IniData.PositionSaved = 0;
			IniData.NewFile = FALSE;
			IniData.NbFichierOuvert++;
			Status = IniFile_StatusOK;
			break;

		case MemFAT_FileCreated:
			strcpy( IniData.pName, IniFile->FileName);
			IniData.pName[ strlen(IniFile->FileName)] = '\0';
			IniData.Position = 0;
			IniData.PositionSaved = 0;
			IniData.NewFile = TRUE;
			IniData.NbFichierOuvert++;
			Status = IniFile_StatusFileCreated;
			break;

		case MemFAT_Error:
		default:
			_CONSOLE( LogId, "**ERROR** MemoireFAT_IsFilePresent KO %s\n", IniFile->FileName );
			return IniFile_StatusKO;
	}

	FileModified = FALSE;
	Mode = MODE_WRITE;

	return Status;
}

/**
 ***************************************************************************************************
 * @todo Validation
 */
IniFile_Status_e
IniFile_OpenWriteTempFile(
		const IniFile_s*	IniFile		/**<[in] Descripteur du contenu du fichier.*/
)
{
	IniFile_Status_e Status = IniFile_StatusOK;

	if ( IniData.NewFile == FALSE )
	{
		// --- Creation d'un fichier temporaire, si le fichier existe deja
		strcpy(IniData.pTempName, IniFile->FileName);
		strcat(IniData.pTempName, "~\0");
		switch (MemoireFAT_IsFilePresent(&IniData.FileTmp, IniData.pTempName, FA_READ | FA_WRITE, TRUE, TRUE))
		{
			case MemFAT_OK:
				Status = IniFile_StatusOK;
				break;

			case MemFAT_FileCreated:
				Status = IniFile_StatusFileCreated;
				break;

			case MemFAT_Error:
			default:
				_CONSOLE( LogId, "**ERROR** MemoireFAT_IsFilePresent KO %s\n", IniData.pTempName );
				return IniFile_StatusKO;
		}

		pCurrentRdFile = &IniData.File;
		pCurrentWrFile = &IniData.FileTmp;
	}
	else
		pCurrentWrFile = &IniData.File;

	return Status;
}

/**
 ***************************************************************************************************
 * @todo Validation
 */
void IniFile_CloseFile (
	void
)
{
	f_close(&IniData.File);
	f_close(&IniData.FileTmp);

	IniData.NbFichierOuvert--;
}

/**
 ***************************************************************************************************
 * @todo Validation
 */
void IniFile_DeleteFile(
	const IniFile_s*	IniFile		/**<[in] Descripteur du contenu du fichier.*/
)
{
	if ((Mode == MODE_WRITE) && (FileModified == TRUE))
	{
		if (pCurrentWrFile == &IniData.FileTmp)
		{
			f_unlink(IniFile->FileName);
		}
	}
}

/**
 ***************************************************************************************************
 * @todo Validation
 */
void IniFile_RenameFile(
	const IniFile_s*	IniFile		/**<[in] Descripteur du contenu du fichier.*/
)
{
	if ((Mode == MODE_WRITE) && (FileModified == TRUE))
	{
		if (pCurrentWrFile == &IniData.FileTmp)
		{
			f_rename(IniData.pTempName, IniFile->FileName);
		}
	}
}

#if 0 // TODO __NON_UTILISEE__ (PG)
/**
 ***************************************************************************************************
 * Validation OK - JD le 28/08/2014
 */
void
IniFile_SetDefaultValue(
		const IniFile_s*	IniFile		/**<[in] Descripteur du contenu du fichier.*/
)
{
	Key_s* pKey;

	for (uint32_t iKey=0; iKey<IniFile->NbKey; iKey++)
	{
		pKey = (Key_s*) &(IniFile->KeyTable[iKey]);
		IniFile_WriteValueFromKey(pKey, pKey->Value);
	}
}
#endif

/**
 ***************************************************************************************************
 * Validation OK - JD le 28/08/2014
 */
Status_e
IniFile_ReadValueFromKey(
		const Key_s*		pKey,		/**<[in] Cle a lire.*/
		const void*			pValue		/**<[in] Pointeur vers la valeur lue.*/
)
{
	if (pValue == NULL)
		return Status_KO;

	// Verification de l'existance de la cle
	if (IniFile_IsKeyExist(pKey->SectionName, pKey->KeyName) == FALSE )
	{
		switch(pKey->KeyType)
		{
			case KeyType_Int:
				*((long*) pValue) = (long) pKey->Value;
				break;

			case KeyType_Str:
				strncpy( (TCHAR*) pValue, pKey->Value, DATA_STR_MAX_SIZE);
				break;

			case KeyType_SectionName:
			default:
				break;
		}
	}
	else
	{
		// Ecriture de la donnee
		switch(pKey->KeyType)
		{
			case KeyType_Int:
				*((long*) pValue) = strtoul( IniFile_ReadValueFromLine(), NULL ,10);
				break;

			case KeyType_Str:
				strncpy( (TCHAR*) pValue, IniFile_ReadValueFromLine(), DATA_STR_MAX_SIZE );
				break;

			case KeyType_SectionName:
			default:
				// Nothing to do.
				break;

			//case KeyType_Float:
			//	ini_putf(pKeyValue[iKey].SectionName, pKeyValue[iKey].KeyName, (float) pKeyValue[iKey].DefaultValue, FileName);
			//	break;
		}

		return Status_OK;
	}

	return Status_KO;
}

#if 0 // TODO __NON_UTILISEE__ (PG)
/**
 ***************************************************************************************************
 * Validation OK - JD le 28/08/2014
 */
Status_e
IniFile_WriteValueFromKey(
		const Key_s*		pKey,		/**<[in] Cle a ecrire.*/
		const void*			Value		/**<[in] Valeur a ecrire.*/
)
{
	Key_s Key;
	Status_e WriteStatus;
	char Buffer[32];

	// Copie du contenu de la cle en local
	memcpy(&Key, pKey, sizeof(Key_s));

	// Pointeur vers la nouvelle valeur
	Key.Value = Value;

	// Verification de la validite de la donnee
	if (IsKeyValid(&Key) == FALSE)
		return Status_KO;

	// Verification de l'existance de la cle dans le fichier
	if ( IniFile_IsKeyExist(pKey->SectionName, pKey->KeyName) == FALSE )
		return Status_KO;

	// Ecriture de la donnee
	switch(pKey->KeyType)
	{
		case KeyType_Int:
			_sprintf(Buffer, "%d\n", (int) Value);
			WriteStatus = ini_putl_OpenedFile(pKey->SectionName, pKey->KeyName, (long) Value, pCurrentRdFile, pCurrentWrFile);
			break;

		case KeyType_Str:
			WriteStatus = ini_puts_OpenedFile(pKey->SectionName, pKey->KeyName, (TCHAR*) Value, pCurrentRdFile, pCurrentWrFile);
			break;

		case KeyType_SectionName:
			// Do Nothing
			break;
		//case KeyType_Float:
		//	ini_putf(pKeyValue[iKey].SectionName, pKeyValue[iKey].KeyName, (float) pKeyValue[iKey].DefaultValue, FileName);
		//	break;
	}

	if ( WriteStatus == Status_OK)
	{
		FileModified = TRUE;
		// Changement du fichier temporaire cible
		if (pCurrentWrFile == &IniData.FileTmp)
		{
			pCurrentWrFile = &IniData.File;
			pCurrentRdFile = &IniData.FileTmp;
		}
		else
		{
			pCurrentWrFile = &IniData.FileTmp;
			pCurrentRdFile = &IniData.File;
		}
	}

	return Status_OK;
}
#endif

/**
 * @brief	Ecriture d'une valeur brute dans un fichier ouvert.
 * @return 	Status.
 */
Status_e
IniFile_WriteRawData(
		const Key_s*		pKey,		/**<[in] Cle a ecrire.*/
		const void*			Value		/**<[in] Valeur a ecrire.*/
)
{
	char Buffer[32];

	FileModified = TRUE;

	// Ecriture de la donnee
	switch(pKey->KeyType)
	{
		case KeyType_SectionName:
			f_puts("[", pCurrentWrFile);
			f_puts(pKey->SectionName, pCurrentWrFile);
			f_puts("]\n", pCurrentWrFile);
			break;

		case KeyType_Int:
			f_puts(pKey->KeyName, pCurrentWrFile);
			f_putc('=', pCurrentWrFile);
			_sprintf(Buffer, "%d\n", (int) Value);
			f_puts(Buffer, pCurrentWrFile);
			break;

		case KeyType_Str:
			f_puts(pKey->KeyName, pCurrentWrFile);
			f_putc('=', pCurrentWrFile);
			f_puts((char*) Value, pCurrentWrFile);
			f_putc('\n', pCurrentWrFile);
			break;
	}

	return Status_OK;
}

/**
 * @brief	Verification si une cle existe.
 * @return 	Status.
 */
Bool_e  IniFile_IsKeyExist( const char * pSection, const char *pKey )
{
	Bool_e Result = FALSE;
	Bool_e Continue = TRUE;
    char * pResult = NULL;
    int Etape = 0;
    int length = 0;

    IniFile_SavePosition();

    while (Continue)
    {
      switch (Etape)
      {
      case 0: // --- INIT ---
        if ( (strcmp( pSection, "" ) != 0) )
        {
            if ( (strcmp( pKey, "" ) != 0) )
            {
				if ( strcmp( IniData.pSection, pSection) == 0 )
					Etape = 2;
				else
					Etape = 1;
            }
            else
            	Etape = 4;
        }
        else
            Etape = 3;
        break;

      case 1: // --- RECHERCHE SECTION ---
    	memset( IniData.Line, '\0', MAX_STRING_LENGTH );
        if ( IniFile_ReadLine(IniData.Line) != 0 )
        {
            if ( IniFile_ReadSectionIfPresent(IniData.Line) )
                if ( strcmp( IniData.pSection, pSection) == 0 )
                    Etape = 2;
        }
        else
          Continue = FALSE;
        break;

      case 2: // --- RECHERCHE CLEF ---
    	  memset( IniData.Line, '\0', MAX_STRING_LENGTH );
         if ( IniFile_ReadLine(IniData.Line) != 0 )
         {
            if ( IniFile_ReadSectionIfPresent(IniData.Line) == FALSE )
            {
                pResult = IniFile_ReadKey(IniData.Line, &length);
                if ( strncmp( pResult, pKey, length) == 0 )
                {
                    Result = TRUE;
                    Continue = FALSE;
                }
            }
            else
                if ( strcmp( IniData.pSection, pSection) != 0 )
                    Continue = FALSE;
        }
        else
          Continue = FALSE;
        break;

    case 3: // --- RECHERCHE CLEF sans SECTION ---
    	memset( IniData.Line, '\0', MAX_STRING_LENGTH );
         if ( IniFile_ReadLine(IniData.Line) != 0 )
         {
            if ( IniFile_ReadSectionIfPresent(IniData.Line) == FALSE )
            {
                pResult = IniFile_ReadKey(IniData.Line, &length);
                if ( strncmp( pResult, pKey, length) == 0 )
                {
                    Result = TRUE;
                    Continue = FALSE;
                }
            }
        }
        else
          Continue = FALSE;
        break;

    case 4: // --- RECHERCHE SECTION sans CLEF---
      memset( IniData.Line, '\0', MAX_STRING_LENGTH );
      if ( IniFile_ReadLine(IniData.Line) != 0 )
      {
          if ( IniFile_ReadSectionIfPresent(IniData.Line) )
              if ( strcmp( IniData.pSection, pSection) == 0 )
              {
                  Result = TRUE;
                  Continue = FALSE;
              }
      }
      else
        Continue = FALSE;
      break;

      default:
          //_CONSOLE( LogId, "**ERROR** Invalid case (%d)\n", Etape);
        break;

      }
    }

    //IniFile_RestorePosition();

    return (Result);
}

int IniFile_ReadLine( char * pLine )
{
	int n = 0;

	f_gets( pLine, 255, &IniData.File );
	n = strlen(pLine);
	IniData.Position += n;

	return(n);

#if 0
    char c;
    int i = 0;
    UINT n = 0;

    // f_lseek( &IniData.File , IniData.Position );

	if( f_read( &IniData.File, &c, 1, &n) == FR_OK)
		IniData.Position++;

	while ( c != '\0' )
	{
		pLine[i++] = c;

		if (c == '\n')
		{
			pLine[i] = '\0';
			c = '\0';
			i++;
		}
		else
		{
			if( f_read( &IniData.File, &c, 1, &n) == FR_OK)
				IniData.Position++;
		}
	}

    return (i);
#endif
}

char * IniFile_SearchValue( const char * pSection, const char *pKey )
{
	char * pResult = NULL;
	int Continue = 1;
	int Etape = 0;
	int length = 0;

	while (Continue)
	{
	  switch (Etape)
	  {
	  case 0: // --- INIT ---
		if ( ( pSection != NULL ) && (strcmp( pSection,"" ) != 0) )
		{
			if ( strcmp( IniData.pSection, pSection) == 0 )
				Etape = 2;
			else
				Etape = 1;
		}
		else
			Etape = 3;
		break;

	  case 1: // --- RECHERCHE SECTION ---
		memset( IniData.Line, '\0', MAX_STRING_LENGTH );
		if ( IniFile_ReadLine(IniData.Line) != 0 )
		{
			if ( IniFile_ReadSectionIfPresent(IniData.Line) )
				if ( strcmp( IniData.pSection, pSection) == 0 )
					Etape = 2;
		}
		else
		  Continue = 0;
		break;

	  case 2: // --- RECHERCHE CLEF ---
		  memset( IniData.Line, '\0', MAX_STRING_LENGTH );
		 if ( IniFile_ReadLine(IniData.Line) != 0 )
		 {
			if ( IniFile_ReadSectionIfPresent(IniData.Line) == 0 )
			{
				pResult = IniFile_ReadKey(IniData.Line, &length);
				if ( strncmp( pResult, pKey, length) == 0 )
				{
					pResult = IniFile_ReadValue(IniData.Line);
					Continue = 0;
				}
			}
			else
				Continue = 0;
		}
		else
		  Continue = 0;
		break;

	  case 3: // --- RECHERCHE CLEF sans SECTION ---
		 memset( IniData.Line, '\0', MAX_STRING_LENGTH );
		 if ( IniFile_ReadLine(IniData.Line) != 0 )
		 {
			if ( IniFile_ReadSectionIfPresent(IniData.Line) == 0 )
			{
				pResult = IniFile_ReadKey(IniData.Line, &length);
				if ( strncmp( pResult, pKey, length) == 0 )
				{
					pResult = IniFile_ReadValue(IniData.Line);
					Continue = 0;
				}
			}
		}
		else
		  Continue = 0;
		break;

	  default:
		  _CONSOLE( LogId, " **ERROR** Invalid case (%d)\n", Etape);
		break;

	  }
	}
	return (pResult);
}

char * IniFile_ReadValueFromLine( void )
{
  char * pResult = NULL;

  pResult = IniFile_ReadValue(IniData.Line);

  return (pResult);
}

Status_e IniFile_WriteValueInCurrentLine( const char * pValue )
{
	return IniFile_WriteValue( IniData.Line, pValue );
}

char* IniFile_ReadKey( char * pLine, int * length )
{
    char * pfc = pLine;
    char * plc = NULL;

    plc = strpbrk ( pLine, "=");
    *length = plc - pfc;

    return (pfc);
}

Bool_e IniFile_ReadSectionIfPresent( char * pLine )
{
   Bool_e result = FALSE;
   char * pfc = NULL;
   char * plc = NULL;

   pfc = strchr( pLine, '[');
   if ( pfc != NULL )
   {
        plc = strrchr( pLine, ']');
        if ( plc != NULL )
        {
            *plc = '\0';
            strcpy( IniData.pSection, ++pfc );
            IniFile_SavePosition();
            result = TRUE;
        }
   }

   return (result);
}

char* IniFile_ReadValue( char * pLine )
{
   char * pfc = NULL;
   char * plc = NULL;

   pfc = strchr( pLine, '=');
   if ( pfc != NULL )
   {
        plc = strpbrk ( pLine, " #\r\n");
        if ( plc != NULL )
            *plc = '\0';
   }

   return (++pfc);
}

Status_e IniFile_WriteValue( char * pLine, const char * pValue )
{
	Status_e Statut = Status_KO;
	char * pfc = NULL;

   pfc = strchr( pLine, '=');
   if ( pfc != NULL )
   {
	   pfc++;
	   IniData.Position += ( pfc - pLine );
	   f_lseek( pCurrentWrFile , IniData.Position );
	   f_puts(pValue, pCurrentWrFile);
	   Statut = Status_OK;
   }

   return (Statut);
}

void IniFile_SavePosition( void )
{
    IniData.PositionSaved = IniData.Position;
}

void IniFile_RestorePosition( void )
{
    IniData.Position = IniData.PositionSaved;
    f_lseek( &IniData.File , IniData.Position );
}

Bool_e
IniFile_IsAFileOpened(
	void
)
{
	Bool_e Result;

	if( IniData.NbFichierOuvert == 0)
		Result = FALSE;
	else
		Result = TRUE;

	return ( Result );
}
 /**
 * @}
 */ 

/* End Of File ************************************************************************************/
