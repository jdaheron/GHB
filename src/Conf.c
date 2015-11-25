/**************************************************************************************************/
/*																								  */
/* Conf																							  */
/*																								  */
/**************************************************************************************************/


/*--------------------------------------------------------------------------------------------------
	INCLUDES
--------------------------------------------------------------------------------------------------*/

#include "Conf.h"
#include "util_printf.h"
#include "drv_CAT24AA16.h"


/*--------------------------------------------------------------------------------------------------
	PRIVATE DEFINE
--------------------------------------------------------------------------------------------------*/


/*--------------------------------------------------------------------------------------------------
	PRIVATE TYPEDEF
--------------------------------------------------------------------------------------------------*/


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


static Conf_t This;


/*--------------------------------------------------------------------------------------------------
	FUNCTIONS DEFINITIONS
--------------------------------------------------------------------------------------------------*/


/*------------------------------------------------------------------------------------------------*/
void Conf_Init(void)
{
	if (Parametres_Init(&Conf_IniFile) != Status_OK)
	{
		_printf("Conf forced to default value\n");
	}
	Parametres_OpenReadFile(&Conf_IniFile);

	Parametres_Read(&Conf_IniFile,	Conf_GEN_StartTempo_s			,	&This.StartTempo_s				);
	Parametres_Read(&Conf_IniFile,	Conf_LOG_Periode_s				,	&This.LOG_Periode_s				);
	Parametres_Read(&Conf_IniFile,	Conf_LOG_PeriodePendantAction_s	,	&This.LOG_PeriodePendantAction_s	);

	Parametres_CloseFile(&Conf_IniFile);


	//------------------------------------------------------
	// Affichage de la configuration
	//------------------------------------------------------
	_printf("--- CONF  ---\n");
	_printf("StartTempo_s               = %d\n",	This.StartTempo_s				);
	_printf("LOG_Periode_s              = %d\n",	This.LOG_Periode_s				);
	_printf("LOG_PeriodePendantAction_s = %d\n",	This.LOG_PeriodePendantAction_s	);
}


/*------------------------------------------------------------------------------------------------*/
void Conf_Write(uint32_t NumParam, void* Value)
{
	Parametres_Write(&Conf_IniFile, NumParam, Value);
}


/*------------------------------------------------------------------------------------------------*/
Conf_t* Conf_Get(void)
{
	return &This;
}


/*------------------------------------------------------------------------------------------------*/
