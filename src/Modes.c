/**************************************************************************************************/
/*																								  */
/* Gestion des Modes																			  */
/*																								  */
/**************************************************************************************************/


/*--------------------------------------------------------------------------------------------------
	INCLUDES
--------------------------------------------------------------------------------------------------*/

#include "Modes.h"
#include "util_CONSOLE.h"
#include "util_TSW.h"

#include "Arrosage.h"
#include "fct_TempHygro.h"
#include "ConfIni.h"


/*--------------------------------------------------------------------------------------------------
	PRIVATE DEFINE
--------------------------------------------------------------------------------------------------*/


/*--------------------------------------------------------------------------------------------------
	PRIVATE TYPEDEF
--------------------------------------------------------------------------------------------------*/


/*--------------------------------------------------------------------------------------------------
	PRIVATE DATA DECLARATION
--------------------------------------------------------------------------------------------------*/

static TSW_s	Tmr_START;
extern Etat_e	EtatVentillation;
extern Etat_e	EtatChauffage;
extern Mode_e	Mode;
extern float	Temperature;
extern float	Hygrometrie;


/*------------------------------------------------------------------------------------------------*/
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


/*------------------------------------------------------------------------------------------------*/
void Mode_Test(void)
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




/*------------------------------------------------------------------------------------------------*/
