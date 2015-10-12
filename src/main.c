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

#include "fct_MemoireFAT.h"
#include "fct_Temperature.h"
#include "drivers/drv_SdCard_SPI.h"
#include "api_USB.h"

#include "equ_Hygrometre.h"
#include "equ_ThermocoupleK.h"

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

const char VERSION_SW[] = {"00001AAC"};
// Definition de l'offset d'execution en fonction de l'option de compilation
// Modifier aussi le script du linker...
#ifdef DEBUG_AVEC_BL
	const uint32_t OFFSET = 0x00020000;
#else
	const uint32_t OFFSET = 0x00000000;
#endif


/** @addtogroup main
  * @{
  */ 

 
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
 * @}
 */ 

 
 /** 
 ***************************************************************************************************
 * @defgroup Private_Defines Private Defines
 * @{
 */
 
#define MAIN_CONSOLE_ENABLE		1
#define USE_ETHERNET_AND_USB	0

#define TEMPERATURE_PERIODE_ACQUISITION_ms	333

#define REGLAGE_RTC()		//RTC_ReglerDateHeure(DIMANCHE, 9, 11, 2014, 0, 2, 0, TRUE)

/**
 * @}
 */ 

 
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
 * @}
 */ 

  
 /** 
 ***************************************************************************************************
 * @defgroup Private_FunctionPrototypes Private FunctionPrototypes
 * @{
 */


/**
 * @}
 */ 
 
 
  /** 
 ***************************************************************************************************
 * @defgroup Private_Variables Private Variables
 * @{
 */

static Mode_e Mode = MODE_DEMARRAGE;
static Mode_e LastMode = 0xFF;
static ModeFct_e ModeFct = MODE_FCT_SERVEUR;
static TSW_s TmrAffichTemp;
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

static uint32_t	LOG_Periode_s				= 0;
#endif

/**
 * @}
 */ 
  	
	
/** 
 ***************************************************************************************************
 * @defgroup Private_Functions Private Functions
 * @{
 */ 
 

/**
 * @brief	Gestion de la Led de LifeBit
 * @return 	void.
 */
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
		}
		else
		{
			TSW_Start(&Tmr_LifeBit, 990);
			Etat = Etat_INACTIF;
		}

		GPIO_Set(PORT_STAT1, Etat);
	}
}

void LogInit()
{
	LogFile_Write("", 0, "Mode;Ventillation;Chauffage;Temperature");
}

void LogData()
{
	char LogBuffer[255];
	int16_t Temperature_DegC;


	// Mode, Temperature
	if (Temperature_IsValide() == FALSE)
		_sprintf(LogBuffer, "%d;-100\n", Mode);
	else
	{
		Temperature_DegC = Temperature_Get_DegC100();
		_sprintf(LogBuffer, "%d;%d;%d;%d,%02d", Mode, EtatVentillation, EtatChauffage, Temperature_DegC/100, Temperature_DegC%100);
	}

	_printf("LOG:%s\n", LogBuffer);
	LogFile_Write("", 0, LogBuffer);
}

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

	Parametres_CloseFile();

	_printf("START_Tempo_s               =%d\n",	START_Tempo_s);
	_printf("CH_SeuilStart_DegC          =%d\n",	CH_SeuilStart_DegC);
	_printf("CH_SeuilStop_DegC           =%d\n",	CH_SeuilStop_DegC);
	_printf("CH_TempoApresCh_s           =%d\n",	CH_TempoApresCh_s);
	_printf("EXT_SeuilStart_DegC         =%d\n",	EXT_SeuilStart_DegC);
	_printf("EXT_SeuilStop_DegC          =%d\n",	EXT_SeuilStop_DegC);
	_printf("EXT_TempoApresEXT_s         =%d\n",	EXT_TempoApresEXT_s);
	_printf("EXT_ActiverPendantChauffage =%d\n",	EXT_ActiverPendantChauffage);
	_printf("LOG_Periode_s               =%d\n",	LOG_Periode_s);

}


/**
 * @}
 */ 

/**
 ***************************************************************************************************
 *
 */

char BufferIn[512];
char BufferOut[1024];


int main(void)
{
	int16_t Temperature_DegC;
	Bool_e NouveauMode;

	// ------------------------------------------------------------------------
	// --- INIT DU SYSTEME
	// --- Initialisations uC, Peripheriques, Fonctions et Interfaces
	BSP_Init();									// Init carte
	SysTick_Config(SystemCoreClock / 1000);		// Init Tick 1ms
	HAL_Console_Init(115200);
	RTC_StartInit(TRUE);						// Start Init RTC
	//I2C1_Init(100 * 1000);						// 100kHz
	ADC1_Init();

	_printf("\n--- START - ALJ");
	_printf(VERSION_SW);
	_printf(" ---\n");


	REGLAGE_RTC();

	//----------------------------------
	// FONCTIONNALITES
	MemoireFAT_Init((Diskio_drvTypeDef*) &SdCard_SPI_Driver);
	Temperature_Init(TEMPERATURE_PERIODE_ACQUISITION_ms);
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

	TSW_Start(&TmrAffichTemp, 2000);

	PC_Init();
	Terminal_Init();

	TSW_Start(&Tmr_START, 1000 * START_Tempo_s);


	_printf("--- StartupTime=%dms ---\n\n", TSW_GetTimestamp_ms());
	while(1)
	{

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
			if (GPIO_Get(PORT_WKUP) == Etat_ACTIF)
			{
				RTC_BkpRegister_Write(0, 1);
				while (GPIO_Get(PORT_WKUP) == Etat_ACTIF)
					TSW_Delay(100);
				GOTO(0);
			}
		#endif

		//----------------------------------
		// PROCESSUS
		LifeBit_Main();
		RTC_Main();
		MemoireFAT_Main();
		Temperature_Thread();

		//----------------------------------
		// LECTURE TEMPERATURE
		if (Temperature_IsValide() == FALSE)
			Mode = MODE_DEFAUT;
		else
			Temperature_DegC = Temperature_Get_DegC100() / 100;

		//----------------------------------
		// AFFICHAGE TEMPERATURE
		if (TSW_IsRunning(&TmrAffichTemp) == FALSE)
		{
			_printf("Temperature = ");
			if (Temperature_IsValide() == FALSE)
				_printf("Non valide\n");
			else
			{
				Temperature_DegC = Temperature_Get_DegC100();
				_printf("%d.%02d %c\n",
						Temperature_DegC/100,
						Temperature_DegC%100,
						'°');
				Temperature_DegC /= 100;
			}

			TSW_Start(&TmrAffichTemp, 1000);
		}

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
			TSW_Start(&Tmr_LOG, 1000 * LOG_Periode_s);
		}

		//----------------------------------
		// GESTION DES MODES
		switch (Mode)
		{
			//--------------------------------------------------------------
			case MODE_DEMARRAGE :

				EtatVentillation = Etat_INACTIF;
				EtatChauffage = Etat_INACTIF;

				if (TSW_IsRunning(&Tmr_START) == TRUE)
					break;

				Mode = MODE_SURVEILLANCE;
				break;


			//--------------------------------------------------------------
			case MODE_SURVEILLANCE :

				if (NouveauMode)
				{
					_printf("----- MODE_SURVEILLANCE -----\n");

					EtatVentillation = Etat_INACTIF;
					EtatChauffage = Etat_INACTIF;
				}

				// Attente fin timer CH
				if (TSW_IsRunning(&Tmr_CH) == TRUE)
					break;

				// Attente fin timer EXT
				if (TSW_IsRunning(&Tmr_EXT) == TRUE)
					break;

				if (Temperature_DegC < CH_SeuilStart_DegC)
					Mode = MODE_CHAUFFAGE;

				if (Temperature_DegC >= EXT_SeuilStart_DegC)
					Mode = MODE_VENTILLATION;

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
				if (Temperature_DegC >= CH_SeuilStop_DegC)
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

				if (Temperature_DegC < EXT_SeuilStop_DegC)
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
		if (GPIO_Get(PORT_REL1_VENTILLATION) != EtatVentillation)
		{
			_printf("Ventillation = %d\n", EtatVentillation);
			GPIO_Set(PORT_REL1_VENTILLATION, EtatVentillation);
			LogData();
		}
		if (GPIO_Get(PORT_REL2_CHAUFFAGE) != EtatChauffage)
		{
			_printf("Chauffage = %d\n", EtatChauffage);
			GPIO_Set(PORT_REL2_CHAUFFAGE, EtatChauffage);
			LogData();
		}
	}

	return 0;

}


/**
 ***************************************************************************************************
 * Validation OK - JD le 08/09/2014
 */

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


/**
  * @brief  Inserts a delay time.
  * @param  nCount: number of 10ms periods to wait for.
  * @retval None
  */
void Delay(uint32_t nCount)
{
  /* Capture the current local time */
  uint32_t timingdelay = TSW_GetTimestamp_ms() + nCount;

  /* wait until the desired delay finish */
  while(timingdelay > TSW_GetTimestamp_ms())
  {
  }
}


 /**
 * @}
 */ 

/* End Of File ************************************************************************************/
