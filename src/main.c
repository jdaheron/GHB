/**
 ***************************************************************************************************
 * @file	main.c
 * @author	j.daheron
 * @version	1.0.0
 * @date	20 juin 2014
 * @brief   Fichier principal.
 ***************************************************************************************************
 */


/* Includes ***************************************************************************************/

#include "ConfIni.h"
#include "fct_TempHygro.h"
#include "api_USB.h"

//#include "equ_Hygrometre.h"
//#include "equ_ThermocoupleK.h"

#include "util_TSW.h"
#include "util_printf.h"
#include "util_Goto.h"
#include "util_LogFile.h"

#include <FILES/fct_MemoireFAT.h>
#include <FILES/FatFs/drivers/drv_SdCard_SPI.h>

#include "Arrosage.h"
#include "Chauffage.h"
#include "Ethernet.h"
#include "Ventilation.h"


#include "Terminal.h"
#include "Terminal_Cmd.h"
#include "if_PC.h"


/* External Variables *****************************************************************************/

const char VERSION_SW[] = {"00001ZAG"};
// Definition de l'offset d'execution en fonction de l'option de compilation
// Modifier aussi le script du linker...
#ifdef DEBUG_AVEC_BL
	const uint32_t OFFSET = 0x00020000;
#else
	const uint32_t OFFSET = 0x00000000;
#endif

 
 /** 
 ***************************************************************************************************
 * @defgroup Private_TypesDefinitions Private TypesDefinitions
 * @{
 */

typedef enum
{
	MODE_DEMARRAGE = 0,
	MODE_SURVEILLANCE,
	MODE_CHAUFFAGE,
	MODE_VENTILLATION,
	MODE_DEFAUT,

} Mode_e;

typedef enum
{
	MODE_FCT_USB = 0,
	MODE_FCT_SERVEUR,

} ModeFct_e;

 
 /** 
 ***************************************************************************************************
 * @defgroup Private_Defines Private Defines
 * @{
 */

#define LogId			"MAIN"

#define REBOOT_ON_DEFAULT_MODE	FALSE
#define MAIN_CONSOLE_ENABLE		1
#define USE_ETHERNET_AND_USB	1

#define TEMPERATURE_PERIODE_ACQUISITION_ms	2500

#define REGLAGE_RTC()		//RTC_ReglerDateHeure(LUNDI, 19, 10, 2015, 22, 15, 0, TRUE)

 
 /** 
 ***************************************************************************************************
 * @defgroup Private_Macros Private Macros
 * @{
 */

#if MAIN_CONSOLE_ENABLE
	#define _MAIN_CONSOLE	_printf
#else
	#define _MAIN_CONSOLE	((void*) 0)
#endif

#define USE_TEMP_HYGRO	1



 /** 
 ***************************************************************************************************
 * @defgroup Private_FunctionPrototypes Private FunctionPrototypes
 * @{
 */

 
 
  /** 
 ***************************************************************************************************
 * @defgroup Private_Variables Private Variables
 * @{
 */

static Mode_e		Mode = MODE_DEMARRAGE;
static Mode_e		LastMode = 0xFF;
static ModeFct_e	ModeFct = MODE_FCT_SERVEUR;
static TSW_s		TmrAffichTempHygro;
static TSW_s		Tmr_LOG;
static TSW_s		Tmr_START;
static TSW_s		Tmr_CH;
static TSW_s		Tmr_EXT;
static TSW_s		Tmr_RTC;
static Etat_e		EtatVentillation = Etat_INACTIF;
static Etat_e		EtatChauffage = Etat_INACTIF;
static float		Temperature = 0;
static float		Hygrometrie = 0;

	
/** 
 ***************************************************************************************************
 * @defgroup Private_Functions Private Functions
 * @{
 */ 


/**************************************************************************************************/
void LifeBit_Main()
{
	TSW Tmr_LifeBit;
	static Etat_e Etat = Etat_INACTIF;

	if (TSW_IsRunning(&Tmr_LifeBit) != TRUE)
	{
		if (Etat == Etat_INACTIF)
		{
			TSW_Start(&Tmr_LifeBit, 10);
			Etat = Etat_ACTIF;
			//_CONSOLE( LogId, "lb\n");
		}
		else
		{
			TSW_Start(&Tmr_LifeBit, 990);
			Etat = Etat_INACTIF;
		}

		GPIO_Set(PORT_LED_LB, Etat);
	}
}


/**************************************************************************************************/
void LogInit()
{
	LogFile_Write("", 0, "Mode;Ventillation;Chauffage;Arrosage;Temperature;Hygrometrie");
}


/**************************************************************************************************/
void LogData()
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

	_CONSOLE( LogId, "LOG:%s\n", LogBuffer);
	LogFile_Write("", 0, LogBuffer);

	if ((Mode == MODE_CHAUFFAGE)
	||	(Mode == MODE_VENTILLATION)
	||	(Arrosage_Get()->Etat == Etat_INACTIF))
	{
		TSW_Start(&Tmr_LOG, 1000 * ConfIni_Get()->GEN_LogPeriodePendantAction_s);
	}
	else
	{
		TSW_Start(&Tmr_LOG, 1000 * ConfIni_Get()->GEN_LogPeriode_s);
	}
}



/**************************************************************************************************/
Status_e Mode_Demarrage(void)
{
	static uint8_t Etape = 0;
	Status_e Status = Status_EnCours;


	switch (Etape)
	{
		// Start timer
		case 0 :

			//TSW_Start(&Tmr_START, 1000 * START_Tempo_s);

			EtatVentillation	= Etat_INACTIF;
			EtatChauffage		= Etat_INACTIF;
			GPIO_Set(Arrosage_Get()->GPIO, Etat_INACTIF);

			Etape++;
			break;


		// Test Ventillation
		case 1 :

			EtatVentillation	= Etat_ACTIF;
			EtatChauffage		= Etat_INACTIF;
			GPIO_Set(Arrosage_Get()->GPIO, Etat_INACTIF);

			TSW_Start(&Tmr_START, 5000);

			Etape++;
			break;


		// Wait end + Test Chauffage
		case 2 :
			if (TSW_IsRunning(&Tmr_START) == TRUE)
			{
				break;
			}

			EtatVentillation	= Etat_INACTIF;
			EtatChauffage		= Etat_ACTIF;
			GPIO_Set(Arrosage_Get()->GPIO, Etat_INACTIF);

			TSW_Start(&Tmr_START, 5000);

			Etape++;
			break;

		// Wait end + Test Pompe
		case 3 :
			if (TSW_IsRunning(&Tmr_START) == TRUE)
			{
				break;
			}

			EtatVentillation	= Etat_INACTIF;
			EtatChauffage		= Etat_INACTIF;
			GPIO_Set(Arrosage_Get()->GPIO, Etat_ACTIF);

			TSW_Start(&Tmr_START, 5000);

			Etape++;
			break;

		// Wait end
		case 4 :
			if (TSW_IsRunning(&Tmr_START) == TRUE)
			{
				break;
			}

			EtatVentillation	= Etat_INACTIF;
			EtatChauffage		= Etat_INACTIF;
			GPIO_Set(Arrosage_Get()->GPIO, Etat_INACTIF);

			TSW_Start(&Tmr_START, 10000);

			Etape++;
			break;

		case 5 :
		default :
			if (TSW_IsRunning(&Tmr_START) == TRUE)
			{
				break;
			}

			Status = Status_Fini;
			Etape = 0;
			break;
	}

	return Status;
}


/**************************************************************************************************/
void Mode_Test()
{
	int Etape = 0;

	while (1)
	{
		Etape++;

		switch (Etape)
		{
			case 1 :
				GPIO_Set(PORT_IHM_LED1, Etat_ACTIF);
				GPIO_Set(PORT_IHM_LED2, Etat_INACTIF);
				GPIO_Set(PORT_IHM_LED3, Etat_INACTIF);
				break;

			case 2 :
				GPIO_Set(PORT_IHM_LED1, Etat_INACTIF);
				GPIO_Set(PORT_IHM_LED2, Etat_ACTIF);
				GPIO_Set(PORT_IHM_LED3, Etat_INACTIF);
				break;

			case 3 :
				GPIO_Set(PORT_IHM_LED1, Etat_INACTIF);
				GPIO_Set(PORT_IHM_LED2, Etat_INACTIF);
				GPIO_Set(PORT_IHM_LED3, Etat_ACTIF);
				break;

			case 4 :
				GPIO_Set(PORT_IHM_LED1, Etat_ACTIF);
				GPIO_Set(PORT_IHM_LED2, Etat_ACTIF);
				GPIO_Set(PORT_IHM_LED3, Etat_ACTIF);
				break;

			case 5 :
				GPIO_Set(PORT_IHM_LED1, Etat_INACTIF);
				GPIO_Set(PORT_IHM_LED2, Etat_INACTIF);
				GPIO_Set(PORT_IHM_LED3, Etat_INACTIF);

				GPIO_Set(PORT_RELAIS_L    , Etat_INACTIF);
				GPIO_Set(PORT_RELAIS_CH   , Etat_INACTIF);
				GPIO_Set(PORT_RELAIS_V_INT, Etat_INACTIF);
				GPIO_Set(PORT_RELAIS_V_EXT, Etat_INACTIF);
				GPIO_Set(PORT_RELAIS_OPT1 , Etat_INACTIF);
				GPIO_Set(PORT_RELAIS_OPT2 , Etat_INACTIF);
				GPIO_Set(PORT_RELAIS_OPT3 , Etat_INACTIF);
				break;

			case 6 :
				GPIO_Set(PORT_RELAIS_L    , Etat_ACTIF);
				GPIO_Set(PORT_RELAIS_CH   , Etat_INACTIF);
				GPIO_Set(PORT_RELAIS_V_INT, Etat_INACTIF);
				GPIO_Set(PORT_RELAIS_V_EXT, Etat_INACTIF);
				GPIO_Set(PORT_RELAIS_OPT1 , Etat_INACTIF);
				GPIO_Set(PORT_RELAIS_OPT2 , Etat_INACTIF);
				GPIO_Set(PORT_RELAIS_OPT3 , Etat_INACTIF);
				break;

			case 7 :
				GPIO_Set(PORT_RELAIS_L    , Etat_INACTIF);
				GPIO_Set(PORT_RELAIS_CH   , Etat_ACTIF);
				GPIO_Set(PORT_RELAIS_V_INT, Etat_INACTIF);
				GPIO_Set(PORT_RELAIS_V_EXT, Etat_INACTIF);
				GPIO_Set(PORT_RELAIS_OPT1 , Etat_INACTIF);
				GPIO_Set(PORT_RELAIS_OPT2 , Etat_INACTIF);
				GPIO_Set(PORT_RELAIS_OPT3 , Etat_INACTIF);
				break;


			case 8 :
				GPIO_Set(PORT_RELAIS_L    , Etat_INACTIF);
				GPIO_Set(PORT_RELAIS_CH   , Etat_INACTIF);
				GPIO_Set(PORT_RELAIS_V_INT, Etat_ACTIF);
				GPIO_Set(PORT_RELAIS_V_EXT, Etat_INACTIF);
				GPIO_Set(PORT_RELAIS_OPT1 , Etat_INACTIF);
				GPIO_Set(PORT_RELAIS_OPT2 , Etat_INACTIF);
				GPIO_Set(PORT_RELAIS_OPT3 , Etat_INACTIF);
				break;

			case 9 :
				GPIO_Set(PORT_RELAIS_L    , Etat_INACTIF);
				GPIO_Set(PORT_RELAIS_CH   , Etat_INACTIF);
				GPIO_Set(PORT_RELAIS_V_INT, Etat_INACTIF);
				GPIO_Set(PORT_RELAIS_V_EXT, Etat_ACTIF);
				GPIO_Set(PORT_RELAIS_OPT1 , Etat_INACTIF);
				GPIO_Set(PORT_RELAIS_OPT2 , Etat_INACTIF);
				GPIO_Set(PORT_RELAIS_OPT3 , Etat_INACTIF);
				break;

			case 10 :
				GPIO_Set(PORT_RELAIS_L    , Etat_INACTIF);
				GPIO_Set(PORT_RELAIS_CH   , Etat_INACTIF);
				GPIO_Set(PORT_RELAIS_V_INT, Etat_INACTIF);
				GPIO_Set(PORT_RELAIS_V_EXT, Etat_INACTIF);
				GPIO_Set(PORT_RELAIS_OPT1 , Etat_ACTIF);
				GPIO_Set(PORT_RELAIS_OPT2 , Etat_INACTIF);
				GPIO_Set(PORT_RELAIS_OPT3 , Etat_INACTIF);
				break;

			case 11 :
				GPIO_Set(PORT_RELAIS_L    , Etat_INACTIF);
				GPIO_Set(PORT_RELAIS_CH   , Etat_INACTIF);
				GPIO_Set(PORT_RELAIS_V_INT, Etat_INACTIF);
				GPIO_Set(PORT_RELAIS_V_EXT, Etat_INACTIF);
				GPIO_Set(PORT_RELAIS_OPT1 , Etat_INACTIF);
				GPIO_Set(PORT_RELAIS_OPT2 , Etat_ACTIF);
				GPIO_Set(PORT_RELAIS_OPT3 , Etat_INACTIF);
				break;

			case 12 :
				GPIO_Set(PORT_RELAIS_L    , Etat_INACTIF);
				GPIO_Set(PORT_RELAIS_CH   , Etat_INACTIF);
				GPIO_Set(PORT_RELAIS_V_INT, Etat_INACTIF);
				GPIO_Set(PORT_RELAIS_V_EXT, Etat_INACTIF);
				GPIO_Set(PORT_RELAIS_OPT1 , Etat_INACTIF);
				GPIO_Set(PORT_RELAIS_OPT2 , Etat_INACTIF);
				GPIO_Set(PORT_RELAIS_OPT3 , Etat_ACTIF);
				break;

			case 13 :
				GPIO_Set(PORT_RELAIS_L    , Etat_INACTIF);
				GPIO_Set(PORT_RELAIS_CH   , Etat_INACTIF);
				GPIO_Set(PORT_RELAIS_V_INT, Etat_INACTIF);
				GPIO_Set(PORT_RELAIS_V_EXT, Etat_INACTIF);
				GPIO_Set(PORT_RELAIS_OPT1 , Etat_INACTIF);
				GPIO_Set(PORT_RELAIS_OPT2 , Etat_INACTIF);
				GPIO_Set(PORT_RELAIS_OPT3 , Etat_INACTIF);
				break;

			default :
				Etape = 0;
				break;
		}

		TSW_Delay(1000);
	}
}


char BufferIn[512];
char BufferOut[1024];



/**************************************************************************************************/
int main(void)
{
	Bool_e NouveauMode;
	Horodatage_s Time = {
			.Heure = 0,
			.Minute = 0,
			.Seconde = 0,
	};


	// ------------------------------------------------------------------------
	// --- INIT DU SYSTEME
	// --- Initialisations uC, Peripheriques, Fonctions et Interfaces
	BSP_Init();									// Init carte
	SysTick_Config(SystemCoreClock / 1000);		// Init Tick 1ms
	HAL_Console_Init(115200);
	RTC_StartInit(TRUE);						// Start Init RTC
	I2C1_Init(100 * 1000);						// 100kHz
	ADC1_Init();

	_CONSOLE( LogId, "\n--- START - ALJ");
	_CONSOLE( LogId, VERSION_SW);
	_CONSOLE( LogId, " ---\n");

	REGLAGE_RTC();

	//----------------------------------
	// FONCTIONNALITES
	MemoireFAT_Init((Diskio_drvTypeDef*) &SdCard_SPI_Driver);
	#if USE_TEMP_HYGRO
		TempHygro_Init(TEMPERATURE_PERIODE_ACQUISITION_ms);
	#endif
	Delay_ms(50);

//	Hygrometre_Init();
	ConfIni_Init();
	Arrosage_Init();
	Chauffage_Init();
	Ventilation_Init();
	LogInit();

	PC_Init();
	Terminal_Init();
	Terminal_Cmd_Init();

	//_CONSOLE( LogId, "MODE_FCT_SERVEUR\n");
	ModeFct = MODE_FCT_SERVEUR;
	//MemoireFAT_PrintFileList("httpserver");
	Ethernet_Init();

	if (RTC_BkpRegister_Read(0) != 0)
	{
		_CONSOLE( LogId, "MODE_FCT_USB\n");
		ModeFct = MODE_FCT_USB;
		//USB_Init((Diskio_drvTypeDef*) &SdCard_SPI_Driver);
		RTC_BkpRegister_Write(0, 0);
	}

	// Lancement des timers
	TSW_Start(&TmrAffichTempHygro, 3000);
	TSW_Start(&Tmr_RTC, 10000);

	//Mode_Test();

	WDG_InitWWDG(2000);

	_CONSOLE( LogId, "--- StartupTime=%dms ---\n\n", TSW_GetTimestamp_ms());
	while(1)
	{
		WDG_Refresh();

		if (PC_Read((uint8_t*) BufferIn, NULL) == TRUE)
		{
			Terminal_Parser(BufferIn, BufferOut, 1024);
			if (strlen(BufferOut) > 0)
				PC_Write(BufferOut, strlen(BufferOut));
		}



		// Choix du mode de fonctionnement
		// WKUP = ACTIF -> USB  -  WKUP = INACTIF -> WebServer
		//if (GPIO_Get(PORT_WKUP) == Etat_ACTIF)
		//{
		//	RTC_BkpRegister_Write(0, 1);
		//	while (GPIO_Get(PORT_WKUP) == Etat_ACTIF)
		//		TSW_Delay(100);
		//	GOTO(0);
		//}

		//----------------------------------
		// PROCESSUS
		LifeBit_Main();
		RTC_Main();
		MemoireFAT_Main();
		#if USE_TEMP_HYGRO
			TempHygro_Thread();
		#endif

		if (Mode != MODE_DEMARRAGE)
		{
			Arrosage_Management();
			Chauffage_Management();
			Ventilation_Management();
		}

		if (ModeFct == MODE_FCT_SERVEUR)
		{
			Ethernet_Management();
		}


		//----------------------------------
		// LECTURE TEMPERATURE
		if ((TempHygro_IsValide() == FALSE)
		&&	(Mode != MODE_DEMARRAGE))
		{
			Mode = MODE_DEFAUT;
		}
		else
		{
			Temperature = TempHygro_GetTemperature();
			Hygrometrie = TempHygro_GetHygrometrie();
		}

		//----------------------------------
		// RTC
		if (TSW_IsFinished(&Tmr_RTC))
		{
			RTC_Lire(&Time);
			TSW_ReStart(&Tmr_RTC);

			//_CONSOLE( LogId, "RTC = %d-%02d-%02d %02d:%02d:%02d;%08d;",
			//				Time.Annee, Time.Mois, Time.Jour,
			//				Time.Heure, Time.Minute, Time.Seconde,
			//				TSW_GetTimestamp_ms());
		}


		//----------------------------------
		// AFFICHAGE TEMPERATURE
/*		if (TSW_IsRunning(&TmrAffichTempHygro) == FALSE)
		{
			_CONSOLE( LogId, "TempHygro = ");
			if (TempHygro_IsValide() == FALSE)
				_CONSOLE( LogId, "Non valide\n");
			else
			{
				_CONSOLE( LogId, "%.01f %c\t%.01f %c\n",
						Temperature, '°',
						Hygrometrie, '%');
			}

			TSW_Start(&TmrAffichTempHygro, 2500);
		}
*/
		//----------------------------------
		// DETECTION CHG MODE
		if (LastMode != Mode)
		{
			LogData();
			LastMode = Mode;
			NouveauMode = TRUE;
		}
		else
		{
			NouveauMode = FALSE;
		}

		//----------------------------------
		// LOG
		if (TSW_IsRunning(&Tmr_LOG) != TRUE)
		{
			LogData();
		}

		//----------------------------------
		// GESTION DES MODES
		switch (Mode)
		{
			//--------------------------------------------------------------
			case MODE_DEMARRAGE :

				if (NouveauMode)
				{
					_CONSOLE( LogId, "----- MODE_DEMARRAGE -----\n");
				}

				if (Mode_Demarrage() == Status_Fini)
				{
					Mode = MODE_SURVEILLANCE;
				}

				break;


			//--------------------------------------------------------------
			case MODE_SURVEILLANCE :

				if (NouveauMode)
				{
					_CONSOLE( LogId, "----- MODE_SURVEILLANCE -----\n");

					EtatVentillation = Etat_INACTIF;
					EtatChauffage = Etat_INACTIF;
				}

				if ((TSW_IsRunning(&Tmr_EXT) == FALSE)
				&&	(TSW_IsRunning(&Tmr_CH) == FALSE))
				{
					// Pas de chauffage dessuite après l'extraction
					if (Temperature < Chauffage_Get()->Cfg_SeuilStart_DegC)
					{
						Mode = MODE_CHAUFFAGE;
					}

					// Pas d'extraction dessuite après le chauffage
					if (Temperature >= Ventilation_Get()->Cfg_SeuilStart_DegC)
					{
						Mode = MODE_VENTILLATION;
					}
				}

				break;


			//--------------------------------------------------------------
			case MODE_CHAUFFAGE :

				if (NouveauMode)
				{
					_CONSOLE( LogId, "----- MODE_CHAUFFAGE -----\n");

					if (Ventilation_Get()->Cfg_ActiverPendantChauffage)
						EtatVentillation = Etat_ACTIF;
					else
						EtatVentillation = Etat_INACTIF;
					EtatChauffage = Etat_ACTIF;
				}

				// Attente franchissement seuil
				if (Temperature >= Chauffage_Get()->Cfg_SeuilStop_DegC)
				{
					EtatChauffage = Etat_INACTIF;

					TSW_Start(&Tmr_CH, 1000 * Chauffage_Get()->Cfg_TempoApresCh_s);
					Mode = MODE_SURVEILLANCE;
				}

				break;

			//--------------------------------------------------------------
			case MODE_VENTILLATION :

				if (NouveauMode)
				{
					_CONSOLE( LogId, "----- MODE_VENTILLATION -----\n");

					EtatChauffage = Etat_INACTIF;
					EtatVentillation = Etat_ACTIF;
				}

				if (Temperature < Ventilation_Get()->Cfg_SeuilStop_DegC)
				{
					EtatVentillation = Etat_INACTIF;

					TSW_Start(&Tmr_EXT, 1000 * Ventilation_Get()->Cfg_TempoApresEXT_s);
					Mode = MODE_SURVEILLANCE;
				}

				break;

			//--------------------------------------------------------------
			case MODE_DEFAUT :

				if (NouveauMode)
				{
					_CONSOLE( LogId, "----- MODE_DEFAUT -----\n");

					EtatVentillation 	= Etat_INACTIF;
					EtatChauffage		= Etat_INACTIF;
					Arrosage_Stop();
				}
				else
				{
					if (REBOOT_ON_DEFAULT_MODE == TRUE)
					{
						_CONSOLE( LogId, "REBOOT...\n");
						TSW_Delay(5000);
						GOTO(0);
						Mode = MODE_DEMARRAGE;
					}
				}
				break;

		}

		//----------------------------------
		// MAJ DES SORTIES
		if (GPIO_Get(PORT_RELAIS_V_EXT) != EtatVentillation)
		{
			_CONSOLE( LogId, "Ventillation = %d\n", EtatVentillation);
			GPIO_Set(PORT_RELAIS_V_EXT, EtatVentillation);
			//LogData();
		}
		if (GPIO_Get(PORT_RELAIS_CH) != EtatChauffage)
		{
			_CONSOLE( LogId, "Chauffage = %d\n", EtatChauffage);
			GPIO_Set(PORT_RELAIS_CH, EtatChauffage);
			//LogData();
		}
	}

	return 0;

}


/**************************************************************************************************/
extern void SdCard_SPI_timerproc (void);

void ApplicationTickHook (void) {

	static uint8_t ucTick_10ms=0;
	static uint8_t ucTick_100ms=0;


	/* Gestion du Tick Timer Software */
	TSW_Refresh();

	/* Tick 10ms */
	ucTick_10ms++;
	if (ucTick_10ms >= 10){
		ucTick_10ms = 0;
		ADC1_Tick();
		SdCard_SPI_timerproc();
	}

	/* Tick 100ms */
	ucTick_100ms++;
	if (ucTick_100ms >= 10){
		ucTick_100ms = 0;

	}
}


/**************************************************************************************************/
void Delay(uint32_t nCount)
{
  /* Capture the current local time */
  uint32_t timingdelay = TSW_GetTimestamp_ms() + nCount;

  /* wait until the desired delay finish */
  while(timingdelay > TSW_GetTimestamp_ms())
  {
  }
}


/* End Of File ************************************************************************************/
