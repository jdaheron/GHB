/**************************************************************************************************/
/*																								  */
/* Gestion de l'arrosage																		  */
/*																								  */
/**************************************************************************************************/


/*--------------------------------------------------------------------------------------------------
	INCLUDES
--------------------------------------------------------------------------------------------------*/

#include "Ventilation.h"
#include "ConfIni.h"
#include "fct_DatabaseEeprom.h"
#include "util_CONSOLE.h"


/*--------------------------------------------------------------------------------------------------
	PRIVATE DEFINE
--------------------------------------------------------------------------------------------------*/

#define LogId			"VENTILATION"


/*--------------------------------------------------------------------------------------------------
	PRIVATE TYPEDEF
--------------------------------------------------------------------------------------------------*/


/*--------------------------------------------------------------------------------------------------
	PRIVATE DATA DECLARATION
--------------------------------------------------------------------------------------------------*/

static Ventilation_t This =
{
		.Cfg_Restored					= FALSE,
		.Cfg_SeuilStop_DegC				= 23,
		.Cfg_SeuilStart_DegC			= 22,
		.Cfg_TempoApresEXT_s			= 300,
		.Cfg_ActiverPendantChauffage	= 0,
};


/*--------------------------------------------------------------------------------------------------
	FUNCTIONS DEFINITIONS
--------------------------------------------------------------------------------------------------*/


/*------------------------------------------------------------------------------------------------*/
void Ventilation_Init(Bool_e UseConfIni)
{
	Ventilation_t TmpThis;
	Status_e ReadStatus;


	_CONSOLE(LogId, "---------- VENTILATION INIT ----------\n");

	//----------------------------------------------------------
	// Initialisation des donnees
	DatabaseEeprom_InitData(DatabaseEeprom_Ventilation, NULL, sizeof(Ventilation_t));
	ReadStatus = DatabaseEeprom_Read(DatabaseEeprom_Ventilation, &TmpThis);
	DatabaseEeprom_Display(DatabaseEeprom_Ventilation, &TmpThis);
	if (ReadStatus == Status_KO)
	{
		_CONSOLE(LogId, "Invalid Eeprom Read: Default restored\n");
		DatabaseEeprom_Write(DatabaseEeprom_Ventilation, &This);
		memcpy(&TmpThis, &This, sizeof(Ventilation_t));
		This.Cfg_Restored = TRUE;
	}
	else
	{
		_CONSOLE(LogId, "Valid Eeprom Read \n");
		memcpy(&This, &TmpThis, sizeof(Ventilation_t));
		This.Cfg_Restored = FALSE;
	}


	//------------------------------------------------------
	// Comparaison avec fichier ini
	//------------------------------------------------------
	if ((ConfIni_Get()->IsValide == TRUE) && (UseConfIni == TRUE))
	{
		_CONSOLE(LogId, "Check SD cfg\n");

		This.Cfg_SeuilStop_DegC				= ConfIni_Get()->EXT_SeuilStop_DegC;
		This.Cfg_SeuilStart_DegC			= ConfIni_Get()->EXT_SeuilStart_DegC;
		This.Cfg_TempoApresEXT_s			= ConfIni_Get()->EXT_TempoApresEXT_s;
		This.Cfg_ActiverPendantChauffage	= ConfIni_Get()->EXT_ActiverPendantCh;

		if (memcmp(&TmpThis, &This, sizeof(Ventilation_t)) != 0)
		{
			_CONSOLE(LogId, "Cfg SD copied\n");
			DatabaseEeprom_Write(DatabaseEeprom_Ventilation, &This);
		}
		else
		{
			_CONSOLE(LogId, "Cfg unchanged\n");
		}
	}


	//------------------------------------------------------
	// Affichage de la configuration
	//------------------------------------------------------
	_CONSOLE(LogId, "SeuilStop_DegC    = %d\n",	This.Cfg_SeuilStop_DegC				);
	_CONSOLE(LogId, "SeuilStart_DegC   = %d\n",	This.Cfg_SeuilStart_DegC			);
	_CONSOLE(LogId, "TempoApresEXT_s   = %d\n",	This.Cfg_TempoApresEXT_s			);
	_CONSOLE(LogId, "ActiverPendantCh. = %d\n",	This.Cfg_ActiverPendantChauffage	);
}


/*------------------------------------------------------------------------------------------------*/
void Ventilation_Management(void)
{

}


/*------------------------------------------------------------------------------------------------*/
Ventilation_t* Ventilation_Get(void)
{
	return &This;
}


/*------------------------------------------------------------------------------------------------*/
