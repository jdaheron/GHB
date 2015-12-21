/**************************************************************************************************/
/*																								  */
/* Gestion de l'arrosage																		  */
/*																								  */
/**************************************************************************************************/


/*--------------------------------------------------------------------------------------------------
	INCLUDES
--------------------------------------------------------------------------------------------------*/

#include "Arrosage.h"
#include "ConfIni.h"
#include "fct_DatabaseEeprom.h"
#include "util_CONSOLE.h"


/*--------------------------------------------------------------------------------------------------
	PRIVATE DEFINE
--------------------------------------------------------------------------------------------------*/

#define LogId			"ARROSAGE"


/*--------------------------------------------------------------------------------------------------
	PRIVATE TYPEDEF
--------------------------------------------------------------------------------------------------*/


/*--------------------------------------------------------------------------------------------------
	PRIVATE DATA DECLARATION
--------------------------------------------------------------------------------------------------*/

static Arrosage_t This =
{
		.Cfg_Restored				= FALSE,
		.Cfg_SaveNeeded				= FALSE,
		.Cfg_Heure					= "20:30",
		.Cfg_Intervalle_h			= 0,
		.Cfg_VolumeParPlant_ml		= 0,
		.Cfg_NbPlants				= 2,
		.Cfg_DebitPompe_ml_par_h	= 12300,
		.Cfg_VolumeReservoir_ml		= 4500,

        .VolumeRestant_ml			= 0,
		.TS_Precedent				= 0,
		.TS_Suivant					= 0,
};


/*--------------------------------------------------------------------------------------------------
	FUNCTIONS DEFINITIONS
--------------------------------------------------------------------------------------------------*/


/*------------------------------------------------------------------------------------------------*/
void Arrosage_Init(void)
{
	Arrosage_t TmpThis;


	_CONSOLE(LogId, "---------- ARROSAGE INIT ----------\n");

	//------------------------------------------------------
	// Lecture des donnees flash
	//------------------------------------------------------
	DatabaseEeprom_InitData(DatabaseEeprom_Arrosage, NULL, sizeof(Arrosage_t));
	if (DatabaseEeprom_Read(DatabaseEeprom_Arrosage, &TmpThis) == Status_KO)
	{
		_CONSOLE(LogId, "Invalid Eeprom Read: Default restored\n");
		DatabaseEeprom_Write(DatabaseEeprom_Arrosage, &This);
		memcpy(&TmpThis, &This, sizeof(Arrosage_t));
		This.Cfg_Restored = TRUE;
	}
	else
	{
		_CONSOLE(LogId, "Valid Eeprom Read \n");
		memcpy(&This, &TmpThis, sizeof(Arrosage_t));
		This.Cfg_Restored = FALSE;
	}


	//------------------------------------------------------
	// Comparaison avec fichier ini
	//------------------------------------------------------
	if (ConfIni_Get()->IsValide == TRUE)
	{
		_CONSOLE(LogId, "Check SD cfg\n");

		memcpy(This.Cfg_Heure,			  ConfIni_Get()->ARR_Heure, 8);
		This.Cfg_Intervalle_h			= ConfIni_Get()->ARR_Intervalle_h;
		This.Cfg_VolumeParPlant_ml		= ConfIni_Get()->ARR_VolumeParPlant_ml;
		This.Cfg_NbPlants				= ConfIni_Get()->ARR_NbPlants;
		This.Cfg_DebitPompe_ml_par_h	= ConfIni_Get()->ARR_DebitPompe_ml_par_h;
		This.Cfg_VolumeReservoir_ml		= ConfIni_Get()->ARR_VolumeReservoir_ml;

		if (memcmp(&TmpThis, &This, sizeof(Arrosage_t)) != 0)
		{
			_CONSOLE(LogId, "Cfg SD copied\n");
			DatabaseEeprom_Write(DatabaseEeprom_Arrosage, &This);
		}
		else
		{
			_CONSOLE(LogId, "Cfg unchanged\n");
		}
	}


	//------------------------------------------------------
	// Affichage
	//------------------------------------------------------
	_CONSOLE(LogId, "Heure               = %s\n",	This.Cfg_Heure					);
	_CONSOLE(LogId, "Intervalle_h        = %d\n",	This.Cfg_Intervalle_h			);
	_CONSOLE(LogId, "VolumeParPlant_ml   = %d\n",	This.Cfg_VolumeParPlant_ml		);
	_CONSOLE(LogId, "NbPlants            = %d\n",	This.Cfg_NbPlants				);
	_CONSOLE(LogId, "DebitPompe_ml_par_h = %d\n",	This.Cfg_DebitPompe_ml_par_h	);
	_CONSOLE(LogId, "VolumeReservoir_ml  = %d\n",	This.Cfg_VolumeReservoir_ml		);

	_CONSOLE(LogId, "STATUS:\n");
	_CONSOLE(LogId, "VolumeRestant_ml    = %d\n",	This.VolumeRestant_ml		);
	_CONSOLE(LogId, "TS_Precedent        = %d\n",	This.TS_Precedent			);
	_CONSOLE(LogId, "TS_Suivant          = %d\n",	This.TS_Suivant				);

	This.GPIO = PORT_RELAIS_OPT1;
	This.Etat = Etat_INACTIF;
	This.Cfg_SaveNeeded = FALSE;

	// Verification de l'arrosage gere toutes les 500ms
	TSW_Start(&This.TmrMngt, 500);
}


/*------------------------------------------------------------------------------------------------*/
void Arrosage_Management(void)
{
	// Verification periodique
	if (TSW_IsFinished(&This.TmrMngt) == FALSE)
	{
		return;
	}
	TSW_ReStart(&This.TmrMngt);

	// Arrosage en cours
	if (TSW_IsRunning(&This.TmrActif) == TRUE)
	{
		This.Etat = Etat_ACTIF;
	}
	else
	{
		This.Etat = Etat_INACTIF;

		if ((This.Cfg_Intervalle_h > 0)
		&&	(RTC_GetTimestamp() >= This.TS_Suivant))
		{
			Arrosage_Start(This.Cfg_VolumeParPlant_ml);
			This.Cfg_SaveNeeded = TRUE;
		}
	}

	// Maj de l'etat de la sortie
	if (GPIO_Get(This.GPIO) != This.Etat)
	{
		_CONSOLE(LogId, "GPIO Arrosage = %d\n", This.Etat);
		GPIO_Set(This.GPIO, This.Etat);
	}

	// Sauvegarde si necessaire
	if (This.Cfg_SaveNeeded == TRUE)
	{
		DatabaseEeprom_Write(DatabaseEeprom_Arrosage, &This);
		This.Cfg_SaveNeeded = FALSE;
	}
}


/*------------------------------------------------------------------------------------------------*/
uint32_t Arrosage_Start(uint32_t VolumeParPlant_ml)
{
	uint32_t Duree_s;


	if ((VolumeParPlant_ml == 0)
	||	(This.Cfg_DebitPompe_ml_par_h == 0))
	{
		TSW_Stop(&This.TmrActif);
		return 0;
	}

	// Calcul de la duree
	Duree_s  = VolumeParPlant_ml * This.Cfg_NbPlants;
	Duree_s *= 3600;
	Duree_s /= This.Cfg_DebitPompe_ml_par_h;

	// Mise a jour des donnees
	This.TS_Precedent	= RTC_GetTimestamp();
	This.TS_Suivant		= This.TS_Precedent + This.Cfg_Intervalle_h * 3600;
	if (This.VolumeRestant_ml > VolumeParPlant_ml * This.Cfg_NbPlants)
	{
		This.VolumeRestant_ml -= VolumeParPlant_ml * This.Cfg_NbPlants;
	}
	else
	{
		This.VolumeRestant_ml = 0;
	}

	// Sauvegarde des donnees
	//Parametres_Write(&Conf_IniFile,	Conf_ARR_VolumeRestant_ml		,	&This.VolumeRestant_ml		);
	//Parametres_Write(&Conf_IniFile,	Conf_ARR_TS_Precedent			,	&This.TS_Precedent			);
	//Parametres_Write(&Conf_IniFile,	Conf_ARR_TS_Suivant				,	&This.TS_Suivant			);

	// Envoi des infos
	_CONSOLE(LogId, "--- ARROSAGE START ---\n");
	_CONSOLE(LogId, "VolumeParPlant_ml   = %d\n",	VolumeParPlant_ml			);
	_CONSOLE(LogId, "Duree_s             = %d\n",	Duree_s						);
	_CONSOLE(LogId, "VolumeRestant_ml    = %d\n",	This.VolumeRestant_ml		);
	_CONSOLE(LogId, "TS_Precedent        = %d\n",	This.TS_Precedent			);
	_CONSOLE(LogId, "TS_Suivant          = %d\n",	This.TS_Suivant				);

	// Lancement arrosage
	TSW_Start(&This.TmrActif, Duree_s * 1000);

	This.Cfg_SaveNeeded = TRUE;

	return Duree_s;
}


/*------------------------------------------------------------------------------------------------*/
void Arrosage_Stop(void)
{
	TSW_Stop(&This.TmrActif);
	This.Etat = Etat_INACTIF;
	GPIO_Set(This.GPIO, This.Etat);
}


/*------------------------------------------------------------------------------------------------*/
void Arrosage_SetVolumeReservoir(uint32_t VolumeReservoirRestant_ml)
{
	This.VolumeRestant_ml = VolumeReservoirRestant_ml;
	//Parametres_Write(&Conf_IniFile,	Conf_ARR_VolumeRestant_ml		,	&This.VolumeRestant_ml		);
}


/*------------------------------------------------------------------------------------------------*/
Bool_e Arrosage_IsActive(void)
{
	if (GPIO_Get(This.GPIO) == Etat_ACTIF)
	{
		return TRUE;
	}

	return FALSE;
}


/*------------------------------------------------------------------------------------------------*/
Arrosage_t*	Arrosage_Get(void)
{
	return &This;
}


/*------------------------------------------------------------------------------------------------*/
