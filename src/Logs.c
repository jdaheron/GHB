/**************************************************************************************************/
/*																								  */
/* Gestion des Logs																			  */
/*																								  */
/**************************************************************************************************/


/*--------------------------------------------------------------------------------------------------
	INCLUDES
--------------------------------------------------------------------------------------------------*/

#include "Logs.h"
#include "util_LogFile.h"
#include "util_printf.h"
#include "util_CONSOLE.h"
#include "util_TSW.h"

#include "Modes.h"
#include "Arrosage.h"
#include "fct_TempHygro.h"
#include "ConfIni.h"


/*--------------------------------------------------------------------------------------------------
	PRIVATE DEFINE
--------------------------------------------------------------------------------------------------*/

#define LogId	"LOGS"
#define LOG_DELAI_s						300		// ConfIni_Get()->GEN_LogPeriode_s
#define LOG_DELAI_PENDANT_ACTION_s		300		// ConfIni_Get()->GEN_LogPeriodePendantAction_s



/*--------------------------------------------------------------------------------------------------
	PRIVATE TYPEDEF
--------------------------------------------------------------------------------------------------*/


/*--------------------------------------------------------------------------------------------------
	PRIVATE DATA DECLARATION
--------------------------------------------------------------------------------------------------*/

extern Etat_e	EtatVentillation;
extern Etat_e	EtatChauffage;
extern Mode_e	Mode;
extern float	Temperature;
extern float	Hygrometrie;

static TSW_s	Tmr_LOG;


/*------------------------------------------------------------------------------------------------*/
void Logs_Init(void)
{
	LogFile_Write("", 0, "Mode;Ventillation;Chauffage;Arrosage;Temperature;Hygrometrie");
}


/*------------------------------------------------------------------------------------------------*/
void Logs_Data(void)
{
	char LogBuffer[255];
	int16_t Temperature_DegC;


	// Mode, Temperature
	if (TempHygro_IsValide() == FALSE)
	{
		_sprintf(LogBuffer, "%d;%d;%d;%d;%.01f;%.01f;",
				Mode, EtatVentillation, EtatChauffage, Arrosage_Get()->Etat,
				-100, -100
		);
	}
	else
	{
		_sprintf(LogBuffer, "%d;%d;%d;%d;%.01f;%.01f;",
				Mode, EtatVentillation, EtatChauffage, Arrosage_Get()->Etat,
				Temperature, Hygrometrie
		);
	}

	_CONSOLE(LogId, "LOG:%s\n", LogBuffer);
	LogFile_Write("", 0, LogBuffer);

	if ((Mode == MODE_CHAUFFAGE)
	||	(Mode == MODE_VENTILLATION)
	||	(Arrosage_Get()->Etat == Etat_ACTIF))
	{
		TSW_Start(&Tmr_LOG, 1000 * LOG_DELAI_PENDANT_ACTION_s);
	}
	else
	{
		TSW_Start(&Tmr_LOG, 1000 * LOG_DELAI_s);
	}
}


/*------------------------------------------------------------------------------------------------*/
void Logs_Management(void)
{
	if (TSW_IsRunning(&Tmr_LOG) != TRUE)
	{
		Logs_Data();
	}
}


/*------------------------------------------------------------------------------------------------*/
