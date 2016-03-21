/**************************************************************************************************/
/*																								  */
/* Gestion de l'Hygrometrie																		  */
/*																								  */
/**************************************************************************************************/


/*--------------------------------------------------------------------------------------------------
	INCLUDES
--------------------------------------------------------------------------------------------------*/

#include <HYGROMETRIE/Hygrometrie.h>
#include "ConfIni.h"
#include "fct_DatabaseEeprom.h"
#include "util_CONSOLE.h"


/*--------------------------------------------------------------------------------------------------
	PRIVATE DEFINE
--------------------------------------------------------------------------------------------------*/

#define LogId			"HYGROMETRIE"


/*--------------------------------------------------------------------------------------------------
	PRIVATE TYPEDEF
--------------------------------------------------------------------------------------------------*/


/*--------------------------------------------------------------------------------------------------
	PRIVATE DATA DECLARATION
--------------------------------------------------------------------------------------------------*/

static Hygrometrie_t This =
{
		.Cfg_Restored					= FALSE,
		.Cfg_SeuilStop_Deg				= 70,
		.Cfg_SeuilStart_Deg				= 40,
		.Cfg_TempoApresProd_s			= 600,
};

extern float	Hygrometrie;


/*--------------------------------------------------------------------------------------------------
	FUNCTIONS DEFINITIONS
--------------------------------------------------------------------------------------------------*/


/*------------------------------------------------------------------------------------------------*/
void Hygrometrie_Init(Bool_e UseConfIni)
{
	Hygrometrie_t TmpThis;
	Status_e ReadStatus;


	_CONSOLE(LogId, "---------- HYGROMETRIE INIT ----------\n");

	//----------------------------------------------------------
	// Initialisation des donnees
	DatabaseEeprom_InitData(DatabaseEeprom_Hygrometrie, NULL, sizeof(Hygrometrie_t));
	ReadStatus = DatabaseEeprom_Read(DatabaseEeprom_Hygrometrie, &TmpThis);
	DatabaseEeprom_Display(DatabaseEeprom_Hygrometrie, &TmpThis);
	if (ReadStatus == Status_KO)
	{
		_CONSOLE(LogId, "Invalid Eeprom Read: Default restored\n");
		DatabaseEeprom_Write(DatabaseEeprom_Hygrometrie, &This);
		memcpy(&TmpThis, &This, sizeof(Hygrometrie_t));
		This.Cfg_Restored = TRUE;
	}
	else
	{
		_CONSOLE(LogId, "Valid Eeprom Read \n");
		memcpy(&This, &TmpThis, sizeof(Hygrometrie_t));
		This.Cfg_Restored = FALSE;
	}


	//------------------------------------------------------
	// Comparaison avec fichier ini
	//------------------------------------------------------
/*	if ((ConfIni_Get()->IsValide == TRUE) && (UseConfIni == TRUE))
	{
		_CONSOLE(LogId, "Check SD cfg\n");

		This.Cfg_SeuilStop_Deg				= ConfIni_Get()->EXT_SeuilStop_DegC;
		This.Cfg_SeuilStart_Deg				= ConfIni_Get()->EXT_SeuilStart_DegC;
		This.Cfg_TempoApresProd_s			= ConfIni_Get()->EXT_TempoApresProd_s;
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
*/

	//------------------------------------------------------
	// Affichage de la configuration
	//------------------------------------------------------
	_CONSOLE(LogId, "SeuilStop_DegC    = %d\n",	This.Cfg_SeuilStop_Deg				);
	_CONSOLE(LogId, "SeuilStart_DegC   = %d\n",	This.Cfg_SeuilStart_Deg				);
	_CONSOLE(LogId, "TempoApresEXT_s   = %d\n",	This.Cfg_TempoApresProd_s			);


	TSW_Start(&This.TmrTempo, 1000 * 30); // On reste au moins 30sec en mode attente

	This.GPIO = PORT_RELAIS_OPT1;
	This.Etat = Etat_INACTIF;

	// Verification toutes les 500ms
	TSW_Start(&This.TmrMngt, 500);
}


/*------------------------------------------------------------------------------------------------*/
void Hygrometrie_Management(void)
{
	// Verification periodique
	if (TSW_IsFinished(&This.TmrMngt) == FALSE)
	{
		return;
	}
	TSW_ReStart(&This.TmrMngt);


	if (This.Etat == Etat_INACTIF)
	{
		if ((TSW_IsRunning(&This.TmrTempo) == FALSE)
		&&	(Hygrometrie < This.Cfg_SeuilStart_Deg))
		{
			This.Etat = Etat_ACTIF;
		}
	}
	else
	{
		if (Hygrometrie >= This.Cfg_SeuilStop_Deg)
		{
			TSW_Start(&This.TmrTempo, 1000 * This.Cfg_TempoApresProd_s);
			This.Etat = Etat_INACTIF;
		}
	}

	// Maj de l'etat de la sortie
	if (GPIO_Get(This.GPIO) != This.Etat)
	{
		_CONSOLE(LogId, "GPIO Hygro = %d\n", This.Etat);
		GPIO_Set(This.GPIO, This.Etat);
		Logs_Data();
	}
}


/*------------------------------------------------------------------------------------------------*/
Hygrometrie_t* Hygrometrie_Get(void)
{
	return &This;
}


/*------------------------------------------------------------------------------------------------*/
