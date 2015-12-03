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
		.Cfg_SeuilStart_DegC	= 20,
		.Cfg_SeuilStop_DegC		= 23,
		.Cfg_TempoApresCh_s		= 300,
};


/*--------------------------------------------------------------------------------------------------
	FUNCTIONS DEFINITIONS
--------------------------------------------------------------------------------------------------*/


/*------------------------------------------------------------------------------------------------*/
void Chauffage_Init(void)
{
	Chauffage_t TmpThis;


	//------------------------------------------------------
	// Lecture des donnees flash
	//------------------------------------------------------
	DatabaseEeprom_InitData(DatabaseEeprom_Chauffage, NULL, sizeof(Chauffage_t));
	if (DatabaseEeprom_Read(DatabaseEeprom_Chauffage, &TmpThis) == Status_KO)
	{
		DatabaseEeprom_Write(DatabaseEeprom_Chauffage, &This);
		memcpy(&TmpThis, &This, sizeof(Chauffage_t));
	}
	else
	{
		memcpy(&This, &TmpThis, sizeof(Chauffage_t));
	}


	//------------------------------------------------------
	// Comparaison avec fichier ini
	//------------------------------------------------------
	if (ConfIni_Get()->IsValide == TRUE)
	{
		This.Cfg_SeuilStart_DegC	= ConfIni_Get()->CH_SeuilStart_DegC;
		This.Cfg_SeuilStop_DegC		= ConfIni_Get()->CH_SeuilStop_DegC;
		This.Cfg_TempoApresCh_s		= ConfIni_Get()->CH_TempoApresCh_s;

		if (memcmp(&TmpThis, &This, sizeof(Chauffage_t)) != 0)
		{
			DatabaseEeprom_Write(DatabaseEeprom_Chauffage, &This);
		}
	}


	//------------------------------------------------------
	// Affichage de la configuration
	//------------------------------------------------------
	_CONSOLE( LogId, "--- CONF CHAUFFAGE ---\n");
	_CONSOLE( LogId, "SeuilStart_DegC = %d\n"	,	This.Cfg_SeuilStart_DegC	);
	_CONSOLE( LogId, "SeuilStop_DegC  = %d\n"	,	This.Cfg_SeuilStop_DegC		);
	_CONSOLE( LogId, "TempoApresCh_s  = %d\n"	,	This.Cfg_TempoApresCh_s		);
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
