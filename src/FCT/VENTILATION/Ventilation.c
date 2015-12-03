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

static Ventilation_t This;


/*--------------------------------------------------------------------------------------------------
	FUNCTIONS DEFINITIONS
--------------------------------------------------------------------------------------------------*/


/*------------------------------------------------------------------------------------------------*/
void Ventilation_Init(void)
{
	Ventilation_t TmpThis;
	Bool_e ConfValide = TRUE;


	//----------------------------------------------------------
	// Initialisation des donnees
	DatabaseEeprom_InitData(DatabaseEeprom_Ventilation, NULL, sizeof(Ventilation_t));
	if (DatabaseEeprom_Read(DatabaseEeprom_Ventilation, &TmpThis) == Status_KO)
	{
		ConfValide = FALSE;
	}


	//------------------------------------------------------
	// Comparaison avec fichier ini
	//------------------------------------------------------
	if (ConfIni_Get()->IsValide == TRUE)
	{
		This.Cfg_SeuilStop_DegC				= ConfIni_Get()->EXT_SeuilStart_DegC;
		This.Cfg_SeuilStart_DegC			= ConfIni_Get()->EXT_SeuilStart_DegC;
		This.Cfg_TempoApresEXT_s			= ConfIni_Get()->EXT_TempoApresEXT_s;
		This.Cfg_ActiverPendantChauffage	= ConfIni_Get()->EXT_ActiverPendantCh;

		if (memcmp(&TmpThis, &This, sizeof(Ventilation_t)) != 0)
		{
			DatabaseEeprom_Write(DatabaseEeprom_Ventilation, &This);
		}
	}


	//------------------------------------------------------
	// Affichage de la configuration
	//------------------------------------------------------
	_CONSOLE( LogId, "--- CONF VENTILATION ---\n");
	_CONSOLE( LogId, "SeuilStop_DegC    = %d\n",	This.Cfg_SeuilStop_DegC				);
	_CONSOLE( LogId, "SeuilStart_DegC   = %d\n",	This.Cfg_SeuilStart_DegC			);
	_CONSOLE( LogId, "TempoApresEXT_s   = %d\n",	This.Cfg_TempoApresEXT_s			);
	_CONSOLE( LogId, "ActiverPendantCh. = %d\n",	This.Cfg_ActiverPendantChauffage	);
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
