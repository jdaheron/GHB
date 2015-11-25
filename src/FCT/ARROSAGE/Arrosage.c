/**************************************************************************************************/
/*																								  */
/* Gestion de l'arrosage																		  */
/*																								  */
/**************************************************************************************************/


/*--------------------------------------------------------------------------------------------------
	INCLUDES
--------------------------------------------------------------------------------------------------*/

#include "Arrosage.h"
#include "Conf.h"
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

static Arrosage_t This;


/*--------------------------------------------------------------------------------------------------
	FUNCTIONS DEFINITIONS
--------------------------------------------------------------------------------------------------*/


/*------------------------------------------------------------------------------------------------*/
void Arrosage_Init(void)
{
	//------------------------------------------------------
	// Lecture de la configuration
	//------------------------------------------------------
	if (Parametres_Init(&Conf_IniFile) != Status_OK)
	{
		_printf("Conf forced to default value\n");
	}
	Parametres_OpenReadFile(&Conf_IniFile);

	Parametres_Read(&Conf_IniFile,	Conf_ARR_Heure					,	This.Heure					);
	Parametres_Read(&Conf_IniFile,	Conf_ARR_Intervalle_h			,	&This.Intervalle_h			);
	Parametres_Read(&Conf_IniFile,	Conf_ARR_VolumeParPlant_ml		,	&This.VolumeParPlant_ml		);
	Parametres_Read(&Conf_IniFile,	Conf_ARR_NbPlants				,	&This.NbPlants				);
	Parametres_Read(&Conf_IniFile,	Conf_ARR_DebitPompe_ml_par_h	,	&This.DebitPompe_ml_par_h	);
	Parametres_Read(&Conf_IniFile,	Conf_ARR_VolumeReservoir_ml		,	&This.VolumeReservoir_ml	);
	Parametres_Read(&Conf_IniFile,	Conf_ARR_VolumeRestant_ml		,	&This.VolumeRestant_ml		);
	Parametres_Read(&Conf_IniFile,	Conf_ARR_TS_Precedent			,	&This.TS_Precedent			);
	Parametres_Read(&Conf_IniFile,	Conf_ARR_TS_Suivant				,	&This.TS_Suivant			);

	Parametres_CloseFile(&Conf_IniFile);


	//------------------------------------------------------
	// Affichage de la configuration
	//------------------------------------------------------
	_printf("--- ARROSAGE CONF ---\n");
	_printf("Heure               = %s\n",	This.Heure					);
	_printf("Intervalle_h        = %d\n",	This.Intervalle_h			);
	_printf("VolumeParPlant_ml   = %d\n",	This.VolumeParPlant_ml		);
	_printf("NbPlants            = %d\n",	This.NbPlants				);
	_printf("DebitPompe_ml_par_h = %d\n",	This.DebitPompe_ml_par_h	);
	_printf("VolumeReservoir_ml  = %d\n",	This.VolumeReservoir_ml		);
	_printf("VolumeRestant_ml    = %d\n",	This.VolumeRestant_ml		);
	_printf("TS_Precedent        = %d\n",	This.TS_Precedent			);
	_printf("TS_Suivant          = %d\n",	This.TS_Suivant				);


	This.GPIO = PORT_RELAIS_OPT1;
	This.Etat = Etat_INACTIF;

	// Verification de l'arrosage gere toutes les secondes
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

		if ((This.Intervalle_h > 0)
		&&	(RTC_GetTimestamp() >= This.TS_Suivant))
		{
			Arrosage_Start(This.VolumeParPlant_ml);
		}
	}

	// Maj de l'etat de la sortie
	if (GPIO_Get(This.GPIO) != This.Etat)
	{
		_printf("GPIO Arrosage = %d\n", This.Etat);
		GPIO_Set(This.GPIO, This.Etat);
	}
}


/*------------------------------------------------------------------------------------------------*/
void Arrosage_Start(uint32_t VolumeParPlant_ml)
{
	uint32_t Duree_s;


	if ((VolumeParPlant_ml == 0)
	||	(This.DebitPompe_ml_par_h == 0))
	{
		TSW_Stop(&This.TmrActif);
		return;
	}

	// Calcul de la duree
	Duree_s  = VolumeParPlant_ml * This.NbPlants;
	Duree_s *= 3600;
	Duree_s /= This.DebitPompe_ml_par_h;

	// Mise a jour des donnees
	This.TS_Precedent	= RTC_GetTimestamp();
	This.TS_Suivant		= This.TS_Precedent + This.Intervalle_h * 3600;
	if (This.VolumeRestant_ml > VolumeParPlant_ml * This.NbPlants)
	{
		This.VolumeRestant_ml -= VolumeParPlant_ml * This.NbPlants;
	}
	else
	{
		This.VolumeRestant_ml = 0;
	}

	// Sauvegarde des donnees
	Parametres_Write(&Conf_IniFile,	Conf_ARR_VolumeRestant_ml		,	&This.VolumeRestant_ml		);
	Parametres_Write(&Conf_IniFile,	Conf_ARR_TS_Precedent			,	&This.TS_Precedent			);
	Parametres_Write(&Conf_IniFile,	Conf_ARR_TS_Suivant				,	&This.TS_Suivant			);

	// Envoi des infos
	_printf("--- ARROSAGE START ---\n");
	_printf("VolumeParPlant_ml   = %d\n",	VolumeParPlant_ml			);
	_printf("Duree_s             = %d\n",	Duree_s						);
	_printf("VolumeRestant_ml    = %d\n",	This.VolumeRestant_ml		);
	_printf("TS_Precedent        = %d\n",	This.TS_Precedent			);
	_printf("TS_Suivant          = %d\n",	This.TS_Suivant				);

	// Lancement arrosage
	TSW_Start(&This.TmrActif, Duree_s * 1000);

	_printf("--- ARROSAGE CONF ---\n");
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
	Parametres_Write(&Conf_IniFile,	Conf_ARR_VolumeRestant_ml		,	&This.VolumeRestant_ml		);
}


/*------------------------------------------------------------------------------------------------*/
Arrosage_t*	Arrosage_Get(void)
{
	return &This;
}


/*------------------------------------------------------------------------------------------------*/
