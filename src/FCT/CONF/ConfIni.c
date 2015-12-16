/**************************************************************************************************/
/*																								  */
/* Conf Ini																						  */
/*																								  */
/**************************************************************************************************/


/*--------------------------------------------------------------------------------------------------
	INCLUDES
--------------------------------------------------------------------------------------------------*/

#include "ConfIni.h"
#include "util_printf.h"
#include "drv_CAT24AA16.h"


/*--------------------------------------------------------------------------------------------------
	PRIVATE DEFINE
--------------------------------------------------------------------------------------------------*/

#define LogId			"CONF_INI"


/*--------------------------------------------------------------------------------------------------
	PRIVATE TYPEDEF
--------------------------------------------------------------------------------------------------*/


typedef enum
{
	SECTION_GENERAL					 = 0,
	ConfIni_GEN_StartTempo_s			,

	SECTION_ETHERNET					,
	ConfIni_ETH_IP_Adresse				,
	ConfIni_ETH_IP_Masque				,
	ConfIni_ETH_IP_Passerelle			,
	ConfIni_ETH_MAC_Adresse				,
	ConfIni_ETH_DHCP_Actif				,

	SECTION_CHAUFFAGE					,
	ConfIni_CH_SeuilStart_DegC			,
	ConfIni_CH_SeuilStop_DegC			,
	ConfIni_CH_TempoApresCh_s			,

	SECTION_EXTRACT						,
	ConfIni_EXT_SeuilStart_DegC			,
	ConfIni_EXT_SeuilStop_DegC			,
	ConfIni_EXT_TempoApresEXT_s			,
	ConfIni_EXT_ActiverPendantCh		,

	SECTION_LOG							,
	ConfIni_LOG_Periode_s				,
	ConfIni_LOG_PeriodePendantAction_s	,

	SECTION_ARROSAGE					,
	ConfIni_ARR_Heure					,
	ConfIni_ARR_Intervalle_h			,
	ConfIni_ARR_VolumeParPlant_ml		,
	ConfIni_ARR_NbPlants				,
	ConfIni_ARR_DebitPompe_ml_par_h		,
	ConfIni_ARR_VolumeReservoir_ml		,
	ConfIni_ARR_VolumeRestant_ml		,
	ConfIni_ARR_TS_Precedent			,
	ConfIni_ARR_TS_Suivant				,

	NB_PARAM,

} ConfIni_e;


/*--------------------------------------------------------------------------------------------------
	PRIVATE DATA DECLARATION
--------------------------------------------------------------------------------------------------*/

static const Key_s ParamDefaultValue[] =
{
		//KeyType				SectionName		KeyName						Value						MinValue			MaxValue

		{KeyType_SectionName,	"GENERAL",		"",							(void*) (0),				(void*) (0),		(void*) (0)					},
		{KeyType_Int,			"GENERAL",		"StartTempo_s",				(void*) (30),				(void*) (0),		(void*) (3600)				},

		{KeyType_SectionName,	"ETHERNET",		"",							(void*) (0),				(void*) (0),		(void*) (0)					},
		{KeyType_Str,			"ETHERNET",		"IP_Adresse",				(void*) "192.168.1.200",	(void*) (0),		(void*) (0)					},
		{KeyType_Str,			"ETHERNET",		"IP_Masque",				(void*) "255.255.255.0",	(void*) (0),		(void*) (0)					},
		{KeyType_Str,			"ETHERNET",		"IP_Passerelle",			(void*) "192.168.1.254",	(void*) (0),		(void*) (0)					},
		{KeyType_Str,			"ETHERNET",		"MAC_Adresse",				(void*) "00:00:00:00:00:01",(void*) (0),		(void*) (0)					},
		{KeyType_Int,			"ETHERNET",		"DHCP_Actif",				(void*) (0),				(void*) (0),		(void*) (1)					},

		{KeyType_SectionName,	"CHAUFFAGE",	"",							(void*) (0),				(void*) (0),		(void*) (0)					},
		{KeyType_Int,			"CHAUFFAGE",	"SeuilStart_DegC",			(void*) (22),				(void*) (10),		(void*) (50)				},
		{KeyType_Int,			"CHAUFFAGE",	"SeuilStop_DegC",			(void*) (25),				(void*) (10),		(void*) (50)				},
		{KeyType_Int,			"CHAUFFAGE",	"TempoApresCh_s",			(void*) (30),				(void*) (0),		(void*) (3600)				},

		{KeyType_SectionName,	"EXTRACT",		"",							(void*) (0),				(void*) (0),		(void*) (0)					},
		{KeyType_Int,			"EXTRACT",		"SeuilStart_DegC",			(void*) (26),				(void*) (10),		(void*) (50)				},
		{KeyType_Int,			"EXTRACT",		"SeuilStop_DegC",			(void*) (24),				(void*) (10),		(void*) (50)				},
		{KeyType_Int,			"EXTRACT",		"TempoApresEXT_s",			(void*) (15),				(void*) (0),		(void*) (3600)				},
		{KeyType_Int,			"EXTRACT",		"ActiverPendantCh",			(void*) (0),				(void*) (0),		(void*) (1)					},

		{KeyType_SectionName,	"LOG",			"",							(void*) (0),				(void*) (0),		(void*) (0)					},
		{KeyType_Int,			"LOG",			"Periode_s",				(void*) (60),				(void*) (5),		(void*) (3600)				},
		{KeyType_Int,			"LOG",			"PeriodePendantAction_s",	(void*) (60),				(void*) (5),		(void*) (3600)				},

		{KeyType_SectionName,	"ARROSAGE",		"",							(void*) (0),				(void*) (0),		(void*) (0)					},
		{KeyType_Str,			"ARROSAGE",		"Heure",					(void*) "20:00",			(void*) (0),		(void*) (1000000)			},
		{KeyType_Int,			"ARROSAGE",		"Intervalle_h",				(void*) (0),				(void*) (0),		(void*) (240)				},
		{KeyType_Int,			"ARROSAGE",		"VolumeParPlant_ml",		(void*) (0),				(void*) (0),		(void*) (1000000)			},
		{KeyType_Int,			"ARROSAGE",		"NbPlants",					(void*) (0),				(void*) (0),		(void*) (100)				},
		{KeyType_Int,			"ARROSAGE",		"DebitPompe_ml_par_h",		(void*) (12300),			(void*) (100),		(void*) (1000000)			},
		{KeyType_Int,			"ARROSAGE",		"VolumeReservoir_ml",		(void*) (4000),				(void*) (100),		(void*) (1000000)			},
		{KeyType_Int,			"ARROSAGE",		"VolumeRestant_ml",			(void*) (0),				(void*) (0),		(void*) (1000000)			},
		{KeyType_Int,			"ARROSAGE",		"TS_Precedente",			(void*) (0),				(void*) (0),		(void*) (4200000000)		}, //TODO : "TS_Precedent"
		{KeyType_Int,			"ARROSAGE",		"TS_Suivant",				(void*) (0),				(void*) (0),		(void*) (4200000000)		},


};

static char ParamCurrentValue[NB_PARAM][INIFILE_DATA_STR_MAX_SIZE] = {{0},{0}};


const IniFile_s Conf_IniFile = {
		.FileName		= "PARAM.ini",
		.KeyTable		= ParamDefaultValue,
		.NbKey			= NB_PARAM,
		.CurrentValue	= ParamCurrentValue,
};


static ConfIni_t This;


/*--------------------------------------------------------------------------------------------------
	FUNCTIONS DEFINITIONS
--------------------------------------------------------------------------------------------------*/


/*------------------------------------------------------------------------------------------------*/
void ConfIni_Init(void)
{
	char str_IP_Adresse[INIFILE_DATA_STR_MAX_SIZE];
	char str_IP_Masque[INIFILE_DATA_STR_MAX_SIZE];
	char str_IP_Passerelle[INIFILE_DATA_STR_MAX_SIZE];
	char str_MAC_Adresse[INIFILE_DATA_STR_MAX_SIZE];
	This.IsValide = TRUE;


	if (MemoireFAT_IsReady() == FALSE)
	{
		This.IsValide = FALSE;
	}

	//------------------------------------------------------
	// Lecture de la configuration
	//------------------------------------------------------
	if (Parametres_Init(&Conf_IniFile) != Status_OK)
	{
		_CONSOLE(LogId, "Conf forced to default value\n");
	}
	Parametres_OpenReadFile(&Conf_IniFile);

	Parametres_Read(&Conf_IniFile,	ConfIni_GEN_StartTempo_s			,	&This.GEN_StartTempo_s				);
	Parametres_Read(&Conf_IniFile,	ConfIni_LOG_Periode_s				,	&This.GEN_LogPeriode_s				);
	Parametres_Read(&Conf_IniFile,	ConfIni_LOG_PeriodePendantAction_s	,	&This.GEN_LogPeriodePendantAction_s	);

	Parametres_Read(&Conf_IniFile,	ConfIni_ETH_IP_Adresse				,	str_IP_Adresse						);
	Parametres_Read(&Conf_IniFile,	ConfIni_ETH_IP_Masque				,	str_IP_Masque						);
	Parametres_Read(&Conf_IniFile,	ConfIni_ETH_IP_Passerelle			,	str_IP_Passerelle					);
	Parametres_Read(&Conf_IniFile,	ConfIni_ETH_MAC_Adresse				,	str_MAC_Adresse						);
	Parametres_Read(&Conf_IniFile,	ConfIni_ETH_DHCP_Actif				,	&This.ETH_DHCP_Actif				);

	Parametres_Read(&Conf_IniFile,	ConfIni_CH_SeuilStart_DegC			,	&This.CH_SeuilStart_DegC			);
	Parametres_Read(&Conf_IniFile,	ConfIni_CH_SeuilStop_DegC			,	&This.CH_SeuilStop_DegC				);
	Parametres_Read(&Conf_IniFile,	ConfIni_CH_TempoApresCh_s			,	&This.CH_TempoApresCh_s				);

	Parametres_Read(&Conf_IniFile,	ConfIni_EXT_SeuilStart_DegC			,	&This.EXT_SeuilStart_DegC			);
	Parametres_Read(&Conf_IniFile,	ConfIni_EXT_SeuilStop_DegC			,	&This.EXT_SeuilStop_DegC			);
	Parametres_Read(&Conf_IniFile,	ConfIni_EXT_TempoApresEXT_s			,	&This.EXT_TempoApresEXT_s			);
	Parametres_Read(&Conf_IniFile,	ConfIni_EXT_ActiverPendantCh		,	&This.EXT_ActiverPendantCh			);

	Parametres_Read(&Conf_IniFile,	ConfIni_ARR_Heure					,	This.ARR_Heure						);
	Parametres_Read(&Conf_IniFile,	ConfIni_ARR_Intervalle_h			,	&This.ARR_Intervalle_h				);
	Parametres_Read(&Conf_IniFile,	ConfIni_ARR_VolumeParPlant_ml		,	&This.ARR_VolumeParPlant_ml			);
	Parametres_Read(&Conf_IniFile,	ConfIni_ARR_NbPlants				,	&This.ARR_NbPlants					);
	Parametres_Read(&Conf_IniFile,	ConfIni_ARR_DebitPompe_ml_par_h		,	&This.ARR_DebitPompe_ml_par_h		);
	Parametres_Read(&Conf_IniFile,	ConfIni_ARR_VolumeReservoir_ml		,	&This.ARR_VolumeReservoir_ml		);

	Parametres_CloseFile(&Conf_IniFile);

	//------------------------------------------------------
	// Décodage/Interprétation
	//------------------------------------------------------
	char* Val[16];

	Conv_ParseString(str_IP_Adresse, '.', Val);
	for (int i = 0; i < 4; i++)
	{
		This.ETH_IP_Adresse[i] = strtoul(Val[i], NULL ,10);
	}
	Conv_ParseString(str_IP_Masque, '.', Val);
	for (int i = 0; i < 4; i++)
	{
		This.ETH_IP_Masque[i] = strtoul(Val[i], NULL ,10);
	}
	Conv_ParseString(str_IP_Passerelle, '.', Val);
	for (int i = 0; i < 4; i++)
	{
		This.ETH_IP_Passerelle[i] = strtoul(Val[i], NULL ,10);
	}
	Conv_ParseString(str_MAC_Adresse, ':', Val);
	for (int i = 0; i < 6; i++)
	{
		This.ETH_MAC_Adresse[i] = strtoul(Val[i], NULL ,10);
	}
}


/*------------------------------------------------------------------------------------------------*/
ConfIni_t* ConfIni_Get(void)
{
	return &This;
}


/*------------------------------------------------------------------------------------------------*/
