/**************************************************************************************************/
/*																								  */
/* Gestion de l'arrosage																		  */
/*																								  */
/**************************************************************************************************/


/*--------------------------------------------------------------------------------------------------
	INCLUDES
--------------------------------------------------------------------------------------------------*/

#include "Chauffage.h"
#include "ConfIni.h"
#include "fct_DatabaseEeprom.h"
#include "util_CONSOLE.h"


/*--------------------------------------------------------------------------------------------------
	PRIVATE DEFINE
--------------------------------------------------------------------------------------------------*/

#define LogId			"CHAUFFAGE"


/*--------------------------------------------------------------------------------------------------
	PRIVATE TYPEDEF
--------------------------------------------------------------------------------------------------*/


/*--------------------------------------------------------------------------------------------------
	PRIVATE DATA DECLARATION
--------------------------------------------------------------------------------------------------*/

static Chauffage_t This =
{
		.Cfg_Restored			= FALSE,
		.Cfg_SeuilStart_DegC	= 20,
		.Cfg_SeuilStop_DegC		= 21,
		.Cfg_TempoApresCh_s		= 300,
};


/*--------------------------------------------------------------------------------------------------
	FUNCTIONS DEFINITIONS
--------------------------------------------------------------------------------------------------*/


/*------------------------------------------------------------------------------------------------*/
void Chauffage_Init(void)
{
	Chauffage_t TmpThis;
	Status_e ReadStatus;


	_CONSOLE(LogId, "---------- CHAUFFAGE INIT ----------\n");

	//------------------------------------------------------
	// Lecture des donnees flash
	//------------------------------------------------------
	DatabaseEeprom_InitData(DatabaseEeprom_Chauffage, NULL, sizeof(Chauffage_t));
	ReadStatus = DatabaseEeprom_Read(DatabaseEeprom_Chauffage, &TmpThis);
	DatabaseEeprom_Display(DatabaseEeprom_Chauffage, &TmpThis);
	if (ReadStatus == Status_KO)
	{
		_CONSOLE(LogId, "Invalid Eeprom Read: Default restored\n");
		DatabaseEeprom_Write(DatabaseEeprom_Chauffage, &This);
		memcpy(&TmpThis, &This, sizeof(Chauffage_t));
		This.Cfg_Restored = TRUE;
	}
	else
	{
		_CONSOLE(LogId, "Valid Eeprom Read \n");
		memcpy(&This, &TmpThis, sizeof(Chauffage_t));
		This.Cfg_Restored = FALSE;
	}


	//------------------------------------------------------
	// Comparaison avec fichier ini
	//------------------------------------------------------
	if (ConfIni_Get()->IsValide == TRUE)
	{
		_CONSOLE(LogId, "Check SD cfg\n");

		This.Cfg_SeuilStart_DegC	= ConfIni_Get()->CH_SeuilStart_DegC;
		This.Cfg_SeuilStop_DegC		= ConfIni_Get()->CH_SeuilStop_DegC;
		This.Cfg_TempoApresCh_s		= ConfIni_Get()->CH_TempoApresCh_s;

		if (memcmp(&TmpThis, &This, sizeof(Chauffage_t)) != 0)
		{
			_CONSOLE(LogId, "Cfg SD copied\n");
			DatabaseEeprom_Write(DatabaseEeprom_Chauffage, &This);
		}
		else
		{
			_CONSOLE(LogId, "Cfg unchanged\n");
		}
	}


	//------------------------------------------------------
	// Affichage de la configuration
	//------------------------------------------------------
	_CONSOLE(LogId, "SeuilStart_DegC = %d\n"	,	This.Cfg_SeuilStart_DegC	);
	_CONSOLE(LogId, "SeuilStop_DegC  = %d\n"	,	This.Cfg_SeuilStop_DegC		);
	_CONSOLE(LogId, "TempoApresCh_s  = %d\n"	,	This.Cfg_TempoApresCh_s		);
}


/*------------------------------------------------------------------------------------------------*/
void Chauffage_Management(void)
{

}


/*------------------------------------------------------------------------------------------------*/
Chauffage_t* Chauffage_Get(void)
{
	return &This;
}


/*------------------------------------------------------------------------------------------------*/
