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

#include "fct_TempHygro.h"
#include "fct_MemoireFAT.h"
#include "drivers/drv_SdCard_SPI.h"
#include "api_USB.h"

//#include "equ_Hygrometre.h"
//#include "equ_ThermocoupleK.h"

#include "util_TSW.h"
#include "util_printf.h"
#include "util_Goto.h"
#include "util_LogFile.h"

#include "main.h"
#include "Conf.h"
#include "stm32f2x7_eth.h"
#include "netconf.h"
#include "httpd.h"
#include "telnet.h"
#include "tftpserver.h"
#include "Terminal.h"
#include "if_PC.h"


/* External Variables *****************************************************************************/

const char VERSION_SW[] = {"00001AAY"};
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

static Mode_e Mode = MODE_DEMARRAGE;
static Mode_e LastMode = 0xFF;
static ModeFct_e ModeFct = MODE_FCT_SERVEUR;
static TSW_s TmrAffichTempHygro;
static TSW_s Tmr_LOG;
static TSW_s Tmr_START;
static TSW_s Tmr_CH;
static TSW_s Tmr_EXT;
static Etat_e EtatVentillation = Etat_INACTIF;
static Etat_e EtatChauffage = Etat_INACTIF;

#if 0
static uint8_t START_Tempo_s				= 30;

static uint8_t CH_SeuilStart_DegC			= 22;
static uint8_t CH_SeuilStop_DegC			= 25;
static uint8_t CH_TempoApresCh_s			= 30;

static uint8_t EXT_SeuilStop_DegC			= 24;
static uint8_t EXT_SeuilStart_DegC			= 26;
static uint8_t EXT_TempoApresEXT_s			= 15;
static uint16_t	EXT_ActiverPendantChauffage	= FALSE;

static uint16_t	LOG_Periode_s				= 60;
#else
static uint32_t START_Tempo_s				= 0;

static uint32_t CH_SeuilStart_DegC			= 0;
static uint32_t CH_SeuilStop_DegC			= 0;
static uint32_t CH_TempoApresCh_s			= 0;

static uint32_t EXT_SeuilStop_DegC			= 0;
static uint32_t EXT_SeuilStart_DegC			= 0;
static uint32_t EXT_TempoApresEXT_s			= 0;
static uint32_t	EXT_ActiverPendantChauffage	= 1;

static float Temperature = 0;
static float Hygrometrie = 0;

static uint32_t	LOG_Periode_s				= 0;
static uint32_t	LOG_PeriodePendantAction_s	= 0;


#endif
  	
	
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
			_printf("lb\n");
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
	LogFile_Write("", 0, "Mode;Ventillation;Chauffage;Temperature;Hygrometrie");
}


/**************************************************************************************************/
void LogData()
{
	char LogBuffer[255];
	int16_t Temperature_DegC;


	// Mode, Temperature
	if (TempHygro_IsValide() == FALSE)
	{
		_sprintf(LogBuffer, "%d;%d;%d;%.01f;%.01f;",
				Mode, EtatVentillation, EtatChauffage,
				-100, -100
		);
	}
	else
	{
		_sprintf(LogBuffer, "%d;%d;%d;%.01f;%.01f;",
				Mode, EtatVentillation, EtatChauffage,
				Temperature, Hygrometrie
		);
	}

	_printf("LOG:%s\n", LogBuffer);
	LogFile_Write("", 0, LogBuffer);

	if ((Mode == MODE_CHAUFFAGE)
	||	(Mode == MODE_VENTILLATION))
	{
		TSW_Start(&Tmr_LOG, 1000 * LOG_PeriodePendantAction_s);
	}
	else
	{
		TSW_Start(&Tmr_LOG, 1000 * LOG_Periode_s);
	}
}


/**************************************************************************************************/
void Conf_Init()
{
	Parametres_Init();

	Parametres_OpenReadFile();

	Parametres_Read(START_TEMPO_s, &START_Tempo_s);

	Parametres_Read(CH_SEUIL_START_DegC, &CH_SeuilStart_DegC);
	Parametres_Read(CH_SEUIL_STOP_DegC, &CH_SeuilStop_DegC);
	Parametres_Read(CH_TEMPO_APRES_CH_s, &CH_TempoApresCh_s);

	Parametres_Read(EXT_SEUIL_START_DegC, &EXT_SeuilStart_DegC);
	Parametres_Read(EXT_SEUIL_STOP_DegC, &EXT_SeuilStop_DegC);
	Parametres_Read(EXT_TEMPO_APRES_EXT_s, &EXT_TempoApresEXT_s);
	Parametres_Read(EXT_ACTIVER_PENDANT_CH, &EXT_ActiverPendantChauffage);

	Parametres_Read(LOG_PERIODE_s, &LOG_Periode_s);
	Parametres_Read(LOG_PERIODE_PENDANT_ACTION_s, &LOG_PeriodePendantAction_s);

	Parametres_CloseFile();

	if (LOG_Periode_s == 0)
	{
		START_Tempo_s				= 120;
		CH_SeuilStart_DegC			= 20;
		CH_SeuilStop_DegC			= 28;
		CH_TempoApresCh_s			= 120;
		EXT_SeuilStart_DegC			= 32;
		EXT_SeuilStop_DegC			= 25;
		EXT_TempoApresEXT_s			= 120;
		EXT_ActiverPendantChauffage	= 0;
		LOG_Periode_s				= 120;
		LOG_PeriodePendantAction_s	= 110;

	}

	_printf("START_Tempo_s               =%d\n",	START_Tempo_s);
	_printf("CH_SeuilStart_DegC          =%d\n",	CH_SeuilStart_DegC);
	_printf("CH_SeuilStop_DegC           =%d\n",	CH_SeuilStop_DegC);
	_printf("CH_TempoApresCh_s           =%d\n",	CH_TempoApresCh_s);
	_printf("EXT_SeuilStart_DegC         =%d\n",	EXT_SeuilStart_DegC);
	_printf("EXT_SeuilStop_DegC          =%d\n",	EXT_SeuilStop_DegC);
	_printf("EXT_TempoApresEXT_s         =%d\n",	EXT_TempoApresEXT_s);
	_printf("EXT_ActiverPendantChauffage =%d\n",	EXT_ActiverPendantChauffage);
	_printf("LOG_Periode_s               =%d\n",	LOG_Periode_s);
	_printf("LOG_PeriodePendantAction_s  =%d\n",	LOG_PeriodePendantAction_s);

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

			Etape++;
			break;


		// Test Ventillation
		case 1 :

			EtatVentillation	= Etat_ACTIF;
			EtatChauffage		= Etat_INACTIF;

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

			TSW_Start(&Tmr_START, 5000);

			Etape++;
			break;

		// Wait end
		case 3 :
			if (TSW_IsRunning(&Tmr_START) == TRUE)
			{
				break;
			}

			EtatVentillation	= Etat_INACTIF;
			EtatChauffage		= Etat_INACTIF;

			TSW_Start(&Tmr_START, 10000);

			Etape++;
			break;

		case 4 :
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


	// ------------------------------------------------------------------------
	// --- INIT DU SYSTEME
	// --- Initialisations uC, Peripheriques, Fonctions et Interfaces
	BSP_Init();									// Init carte
	SysTick_Config(SystemCoreClock / 1000);		// Init Tick 1ms
	HAL_Console_Init(115200);
	RTC_StartInit(TRUE);						// Start Init RTC
	I2C1_Init(100 * 1000);						// 100kHz
	ADC1_Init();

	_printf("\n--- START - ALJ");
	_printf(VERSION_SW);
	_printf(" ---\n");

	REGLAGE_RTC();

	//----------------------------------
	// FONCTIONNALITES
	MemoireFAT_Init((Diskio_drvTypeDef*) &SdCard_SPI_Driver);
	TempHygro_Init(TEMPERATURE_PERIODE_ACQUISITION_ms);
	Conf_Init();
//	Hygrometre_Init();
	LogInit();

	#if USE_ETHERNET_AND_USB
		if (RTC_BkpRegister_Read(0) == 0)
		{
			_printf("MODE_FCT_SERVEUR\n");
			ModeFct = MODE_FCT_SERVEUR;

			//MemoireFAT_PrintFileList("httpserver");

			/* configure ethernet (GPIOs, clocks, MAC, DMA) */
			ETH_BSP_Config();

			/* Initilaize the LwIP stack */
			LwIP_Init();

			/* TFTP server Init */
			tftpd_init();

			/* Http webserver Init */
		//	httpd_init();

			telnet_init();
		}
		else
		{
			_printf("MODE_FCT_USB\n");
			ModeFct = MODE_FCT_USB;
//			USB_Init((Diskio_drvTypeDef*) &SdCard_SPI_Driver);
			RTC_BkpRegister_Write(0, 0);
		}
	#endif

	TSW_Start(&TmrAffichTempHygro, 3000);

	PC_Init();
	Terminal_Init();


	//Mode_Test();

	WDG_InitWWDG(2000);

	_printf("--- StartupTime=%dms ---\n\n", TSW_GetTimestamp_ms());
	while(1)
	{
		WDG_Refresh();

		if (PC_Read((uint8_t*) BufferIn, NULL) == TRUE)
		{
			Terminal_Parser(BufferIn, BufferOut, 1024);
			if (strlen(BufferOut) > 0)
				PC_Write(BufferOut, strlen(BufferOut));
		}

		#if USE_ETHERNET_AND_USB
			if (ModeFct == MODE_FCT_SERVEUR)
			{
				/* check if any packet received */
				if (ETH_CheckFrameReceived())
				{
					/* process received ethernet packet */
					LwIP_Pkt_Handle();
				}

				/* handle periodic timers for LwIP */
				LwIP_Periodic_Handle(TSW_GetTimestamp_ms());
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
		#endif

		//----------------------------------
		// PROCESSUS
		LifeBit_Main();
		RTC_Main();
		MemoireFAT_Main();
		TempHygro_Thread();

		//----------------------------------
		// LECTURE TEMPERATURE
		if (TempHygro_IsValide() == FALSE)
		{
			Mode = MODE_DEFAUT;
		}
		else
		{
			Temperature = TempHygro_GetTemperature();
			Hygrometrie = TempHygro_GetHygrometrie();
		}

		//----------------------------------
		// AFFICHAGE TEMPERATURE
/*		if (TSW_IsRunning(&TmrAffichTempHygro) == FALSE)
		{
			_printf("TempHygro = ");
			if (TempHygro_IsValide() == FALSE)
				_printf("Non valide\n");
			else
			{
				_printf("%.01f %c\t%.01f %c\n",
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
			NouveauMode = FALSE;

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

				if (Mode_Demarrage() == Status_Fini)
				{
					Mode = MODE_SURVEILLANCE;
				}

				break;


			//--------------------------------------------------------------
			case MODE_SURVEILLANCE :

				if (NouveauMode)
				{
					_printf("----- MODE_SURVEILLANCE -----\n");

					EtatVentillation = Etat_INACTIF;
					EtatChauffage = Etat_INACTIF;
				}

				// Pas de chauffage dessuite après l'extraction
				if ((Temperature < CH_SeuilStart_DegC)
				&&	(TSW_IsRunning(&Tmr_EXT) == FALSE))
				{
					Mode = MODE_CHAUFFAGE;
				}

				// Pas d'extraction dessuite après le chauffage
				if ((Temperature >= EXT_SeuilStart_DegC)
				&&	(TSW_IsRunning(&Tmr_CH) == FALSE))
				{
					Mode = MODE_VENTILLATION;
				}

				break;


			//--------------------------------------------------------------
			case MODE_CHAUFFAGE :

				if (NouveauMode)
				{
					_printf("----- MODE_CHAUFFAGE -----\n");

					if (EXT_ActiverPendantChauffage)
						EtatVentillation = Etat_ACTIF;
					else
						EtatVentillation = Etat_INACTIF;
					EtatChauffage = Etat_ACTIF;
				}

				// Attente franchissement seuil
				if (Temperature >= CH_SeuilStop_DegC)
				{
					EtatChauffage = Etat_INACTIF;

					TSW_Start(&Tmr_CH, 1000 * CH_TempoApresCh_s);
					Mode = MODE_SURVEILLANCE;
				}

				break;

			//--------------------------------------------------------------
			case MODE_VENTILLATION :

				if (NouveauMode)
				{
					_printf("----- MODE_VENTILLATION -----\n");

					EtatChauffage = Etat_INACTIF;
					EtatVentillation = Etat_ACTIF;
				}

				if (Temperature < EXT_SeuilStop_DegC)
				{
					EtatVentillation = Etat_INACTIF;

					TSW_Start(&Tmr_EXT, 1000 * EXT_TempoApresEXT_s);
					Mode = MODE_SURVEILLANCE;
				}

				break;

			//--------------------------------------------------------------
			case MODE_DEFAUT :

				if (NouveauMode)
				{
					_printf("----- MODE_DEFAUT -----\n");

					EtatVentillation = Etat_INACTIF;
					EtatChauffage = Etat_INACTIF;
				}
				else
				{
					_printf("REBOOT...\n");
					TSW_Delay(5000);
					GOTO(0);
					Mode = MODE_DEMARRAGE;
				}
				break;

		}

		//----------------------------------
		// MAJ DES SORTIES
		if (GPIO_Get(PORT_RELAIS_V_EXT) != EtatVentillation)
		{
			_printf("Ventillation = %d\n", EtatVentillation);
			GPIO_Set(PORT_RELAIS_V_EXT, EtatVentillation);
			//LogData();
		}
		if (GPIO_Get(PORT_RELAIS_CH) != EtatChauffage)
		{
			_printf("Chauffage = %d\n", EtatChauffage);
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
