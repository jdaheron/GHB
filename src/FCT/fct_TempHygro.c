/**
 ***************************************************************************************************
 * @file	fct_Temperature.h
 * @author	Julien
 * @version	1.0.0
 * @date	12 oct. 2014
 * @brief   Gestion de la temperature.
 ***************************************************************************************************
 */


/* Includes ***************************************************************************************/

#include "fct_TempHygro.h"
#include "equ_AM23xx.h"



#define TEMPHYGRO_TEMP_DEFAULT_VALUE	22
#define TEMPHYGRO_HYGRO_DEFAULT_VALUE	50


/* External Variables *****************************************************************************/

static TempHygro_s TempHygro;

 

/**************************************************************************************************/
void TempHygro_Init(uint32_t PeriodeAcquisition_ms)
{
	// Init des variables
	TempHygro.IsValide				= FALSE;
	TempHygro.NbReadError			= 0;
	TempHygro.CurrentTemp_DegC		= TEMPHYGRO_TEMP_DEFAULT_VALUE;
	TempHygro.CurrentHygro_pr100	= TEMPHYGRO_HYGRO_DEFAULT_VALUE;
	TempHygro.iAcquisition			= 0;
	TempHygro.PeriodeAcquisition_ms	=PeriodeAcquisition_ms;

	// Premiere acquisition
	TempHygro_Read();
	for (uint16_t i=1; i<TEMPHYGRO_NB_ECH; i++)
	{
		TempHygro.HygroAcquisition[i] 	= TempHygro.HygroAcquisition[0];
		TempHygro.TempAcquisition[i] 	= TempHygro.TempAcquisition[0];
	}

	// Lancement du Timer d'acquisition
	TSW_Start(&TempHygro.TmrAcquisition, TempHygro.PeriodeAcquisition_ms);
}


/**************************************************************************************************/
void TempHygro_Read(void)
{
	int16_t Temp;
	uint16_t Humid;


	// Read Error
	if (AM23xx_Read(&Temp, &Humid) == FALSE)
	{
		TempHygro.NbReadError++;
		if (TempHygro.NbReadError >= TEMPHYGRO_NB_READ_ERROR_MAX)
		{
			TempHygro.IsValide = FALSE;
		}
	}
	// Read OK
	else
	{
		TempHygro.NbReadError = 0;
		TempHygro.IsValide = TRUE;

		TempHygro.TempAcquisition[TempHygro.iAcquisition] = Temp;
		TempHygro.HygroAcquisition[TempHygro.iAcquisition] = Humid;

		TempHygro.iAcquisition++;
		if (TempHygro.iAcquisition >= TEMPHYGRO_NB_ECH)
		{
			TempHygro.iAcquisition = 0;
		}
	}
}


/**************************************************************************************************/
void TempHygro_Thread(void)
{
	if (TSW_IsRunning(&TempHygro.TmrAcquisition) == FALSE)
	{
		TempHygro_Read();
		TSW_ReStart(&TempHygro.TmrAcquisition);
	}
}


/**************************************************************************************************/
Bool_e TempHygro_IsValide(void)
{
	return TempHygro.IsValide;
}


/**************************************************************************************************/
float TempHygro_GetTemperature(void)
{
	if (TempHygro.IsValide == FALSE)
	{
		return TEMPHYGRO_TEMP_DEFAULT_VALUE;
	}

	int32_t Somme = 0;
	for (uint16_t i=0; i<TEMPHYGRO_NB_ECH; i++)
	{
		Somme += TempHygro.TempAcquisition[i];
	}
	TempHygro.CurrentTemp_DegC = (float) Somme / TEMPHYGRO_NB_ECH / 10;

	return TempHygro.CurrentTemp_DegC;
}


/**************************************************************************************************/
float TempHygro_GetHygrometrie(void)
{
	int32_t Moyenne;
	if (TempHygro.IsValide == FALSE)
	{
		return TEMPHYGRO_HYGRO_DEFAULT_VALUE;
	}

	int32_t Somme = 0;
	for (uint16_t i=0; i<TEMPHYGRO_NB_ECH; i++)
	{
		Somme += TempHygro.HygroAcquisition[i];
	}
	TempHygro.CurrentHygro_pr100 = (float) Somme / TEMPHYGRO_NB_ECH / 10;

	return TempHygro.CurrentHygro_pr100;
}


/* End Of File ************************************************************************************/
