/**
 ***************************************************************************************************
 * @file	fct_DatabaseEeprom.c
 * @author	j.daheron
 * @version	1.0.0
 * @date	10 sept. 2014
 * @brief   Gestion de la base de donnees en EEPROM.
 ***************************************************************************************************
 */


/* Includes ***************************************************************************************/
#include "fct_DatabaseEeprom.h"

#include "util_Checksum.h"
#include "util_CONSOLE.h"



/* External Variables *****************************************************************************/


/** @addtogroup fct_DatabaseEeprom
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
 
/**
 * @}
 */ 

 
 /** 
 ***************************************************************************************************
 * @defgroup Private_Macros Private Macros
 * @{
 */
  
#define Read(a, b, c)	CAT24AA16_Read(a, b, c)
#define Write(a, b, c)	CAT24AA16_Write(a, b, c)
#define ALIGN_PAGE(a)	CAT24AA16_PAGE_SIZE * (1 + ((2+a) / CAT24AA16_PAGE_SIZE))

#define LogId			-1//"DB_EEPROM"

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

static DatabaseEeprom_s DatabaseEeprom[NB_DatabaseEeprom] = {

		// DatabaseEeprom_Arrosage
		{
				.StartAddress	= 0,	// (Size 128 octets)
				.UseBkp			= FALSE,
		},

		// DatabaseEeprom_Chauffage
		{
				.StartAddress	= 128,	 // (Size 64 octets)
				.UseBkp			= FALSE,
		},

		// DatabaseEeprom_Ethernet		 // (Size 64 octets)
		{
				.StartAddress	= 192,
				.UseBkp			= FALSE,
		},

		// DatabaseEeprom_Ventilation	// (Size 64  octets)
		{
				.StartAddress	= 256,
				.UseBkp			= FALSE,
		},
};


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
 

/**-----------------------------------------------------------------------------
 * @brief	Verification de la validite d'une donnee de la base.
 *
 */
static Bool_e IsValide(DatabaseEeprom_s* pDB)
{
	uint8_t Checksum_calcule;

	// Verification Checksum
	Checksum_calcule = (uint8_t) Checksum_CalculComplementA2((uint8_t*) pDB->pData, pDB->Size);
	if (pDB->Checksum != Checksum_calcule)
		return FALSE;

	return TRUE;
}

/**-----------------------------------------------------------------------------
 * @brief	Lecture d'une donnee.
 *
 */
static void ReadData(uint32_t StartAddress, DatabaseEeprom_s* pDB)
{
	uint8_t CrcNumBuffer[2];

	Read(StartAddress,				(uint8_t*) pDB->pData,	pDB->Size);
	Read(StartAddress + pDB->Size,	CrcNumBuffer,			2);

	pDB->Num		= CrcNumBuffer[0];
	pDB->Checksum	= CrcNumBuffer[1];
}

/**-----------------------------------------------------------------------------
 * @brief	Ecriture d'une donnee.
 *
 */
static void WriteData(uint32_t StartAddress, DatabaseEeprom_s* pDB)
{
	uint8_t CrcNumBuffer[2];

	pDB->Checksum = (uint8_t) Checksum_CalculComplementA2((uint8_t*) pDB->pData, pDB->Size);

	CrcNumBuffer[0] = pDB->Num;
	CrcNumBuffer[1] = pDB->Checksum;

	Write(StartAddress,				(uint8_t*) pDB->pData,	pDB->Size);
	Write(StartAddress + pDB->Size,	CrcNumBuffer,			2);
}

/**-----------------------------------------------------------------------------
 * @brief	Lecture d'une donnee.
 *
 */


/**
 ***************************************************************************************************
 * @todo Validation
 */
void
DatabaseEeprom_InitData(
		DatabaseEeprom_Data_e	DatabaseEeprom_Data,	/**<[in] Index de la donnees dans la base.*/
		void*					pDefaultData,			/**<[in] Valeurs par defaut.*/
		uint32_t				DataSize				/**<[in] Taille de la donnees dans la base.*/
)
{
	DatabaseEeprom_s* pDB;

	if (DatabaseEeprom_Data >= NB_DatabaseEeprom)
		return;

	pDB = &DatabaseEeprom[DatabaseEeprom_Data];
	pDB->Size				= DataSize;
	pDB->First_Read_Done	= FALSE;
	pDB->CurrentSpace		= 0;
	pDB->pDefaultData		= pDefaultData;
}


/** 
 ***************************************************************************************************
 * @todo Validation
 */
Status_e
DatabaseEeprom_Read(
		DatabaseEeprom_Data_e	DatabaseEeprom_Data,	/**<[in] Index de la donnees dans la base.*/
		void*					pData					/**<[out] Pointeur vers les donnees lues.*/
)
{
	DatabaseEeprom_s* pDB;
	Bool_e Data0_valide;
	Bool_e Data1_valide;
	uint8_t Num0;
	uint8_t Num1;
	int8_t ChoixData = (-1);

	if (DatabaseEeprom_Data >= NB_DatabaseEeprom)
		return Status_KO;

	//--------------------------------------
	// Preparation
	pDB = &DatabaseEeprom[DatabaseEeprom_Data];
	pDB->pData = pData;

	if(!pDB->First_Read_Done) {
		pDB->First_Read_Done = TRUE;
	}

	//--------------------------------------
	// Lecture des donnees
	_CONSOLE(LogId, "---DatabaseEeprom_Read---\n");

	// Lecture et verification Data0
	ReadData(pDB->StartAddress, pDB);
	Num0 = pDB->Num;
	Data0_valide = IsValide(pDB);

	// Si pas de backup utilise on sort de suite
	if (pDB->UseBkp == FALSE)
	{
		if (Data0_valide)
			return Status_OK;
		else
			return Status_KO;
	}

	// Lecture et verification Data1 si utilise
	ReadData(pDB->StartAddress + ALIGN_PAGE(pDB->Size+2), pDB);
	Num1 = pDB->Num;
	Data1_valide = IsValide(pDB);

	_CONSOLE(LogId, "Data0: Num=%d | IsValide=%d\n", Num0, Data0_valide);
	_CONSOLE(LogId, "Data1: Num=%d | IsValide=%d\n", Num1, Data1_valide);

	//--------------------------------------
	// Choix de la data a conserver
	if (Data0_valide && Data1_valide)		// 2 Data valides (on prend le plus recente)
	{
		_CONSOLE(LogId, "Data0 et Data1 valides: ");
		if ((Num0 == 0) && (Num1 == 0xFF))
			ChoixData = 0;
		else if ((Num0 == 0xFF) && (Num1 == 0))
			ChoixData = 1;
		else if (Num0 > Num1)
			ChoixData = 0;
		else
			ChoixData = 1;
	}
	else if (Data0_valide && !Data1_valide)	// Seule la Data0 est valide
		ChoixData = 0;
	else if (!Data0_valide && Data1_valide)	// Seule la Data1 est valide
		ChoixData = 1;
	else									// Aucune Data valide...
		ChoixData = (-1);

	switch (ChoixData)
	{
		case 0:
			_CONSOLE(LogId, "utilisation de Data0.\n");
			ReadData(pDB->StartAddress, pDB);	// Relecture de Data0
			pDB->CurrentSpace = 0;
			break;

		case 1:
			_CONSOLE(LogId, "utilisation de Data1.\n");
			// Relecture inutile (deja stockee dans pData)
			pDB->CurrentSpace = 1;
			break;

		case (-1):
		default:
			_CONSOLE(LogId, "Aucune Data valide. Restauration des valeurs par defaut.\n");
			if (pDB->pDefaultData != NULL)
			{
				memcpy(pDB->pData, pDB->pDefaultData, pDB->Size);
				pDB->CurrentSpace = 1;
				DatabaseEeprom_Write(DatabaseEeprom_Data, pData);
			}
			return Status_KO;
			break;
	}

	return Status_OK;

}


/**
 ***************************************************************************************************
 * @todo Validation
 */
Status_e
DatabaseEeprom_Write(
		DatabaseEeprom_Data_e	DatabaseEeprom_Data,	/**<[in] Index de la donnees dans la base.*/
		void*					pData					/**<[out] Pointeur vers les donnees a ecrire.*/
)
{
	DatabaseEeprom_s* pDB;

	if (DatabaseEeprom_Data >= NB_DatabaseEeprom)
		return Status_KO;

	//--------------------------------------
	// Preparation
	pDB = &DatabaseEeprom[DatabaseEeprom_Data];
	pDB->pData = pData;

	//--------------------------------------
	// Verif premiere lecture effectuee
	if(pDB->First_Read_Done == FALSE)
		DatabaseEeprom_Read(DatabaseEeprom_Data, pData);

	//--------------------------------------
	// Ecriture des donnees

	_CONSOLE(LogId, "---DatabaseEeprom_Write---\n");

	// Incrementation du numero de sauvegarde
	pDB->Num++;

	// Selection de l'emplacement de destination
	pDB->CurrentSpace++;
	if (pDB->CurrentSpace > 1)
		pDB->CurrentSpace = 0;

	_CONSOLE(LogId, "CurrentSpace=%d\n", pDB->CurrentSpace);

	// Enregistrement en EEPROM
	if ((pDB->CurrentSpace == 0) || (pDB->UseBkp == FALSE))
		WriteData(pDB->StartAddress, pDB);
	else
		WriteData(pDB->StartAddress + ALIGN_PAGE(pDB->Size), pDB);

	return Status_OK;
}








 /**
 * @}
 */ 

/* End Of File ************************************************************************************/
