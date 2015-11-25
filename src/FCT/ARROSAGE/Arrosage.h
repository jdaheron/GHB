/**************************************************************************************************/
/*																								  */
/* Gestion de l'arrosage																		  */
/*																								  */
/**************************************************************************************************/

#ifndef ARROSAGE_H_
#define ARROSAGE_H_


/*--------------------------------------------------------------------------------------------------
	PUBLIC INCLUDES
--------------------------------------------------------------------------------------------------*/

#include "BSP.h"
#include "util_TSW.h"


/*--------------------------------------------------------------------------------------------------
	PUBLIC DEFINE
--------------------------------------------------------------------------------------------------*/


/*--------------------------------------------------------------------------------------------------
	PUBLIC TYPEDEF
--------------------------------------------------------------------------------------------------*/

typedef struct
{
	char			Heure[8];
	uint32_t		Intervalle_h;
	uint32_t		VolumeParPlant_ml;
	uint32_t		NbPlants;
	uint32_t		DebitPompe_ml_par_h;
	uint32_t		VolumeReservoir_ml;
	uint32_t		VolumeRestant_ml;
	uint32_t		TS_Precedent;
	uint32_t		TS_Suivant;

	TSW_s			TmrMngt;
	TSW_s			TmrActif;

	MappingGpio_e	GPIO;
	Etat_e			Etat;

} Arrosage_t;


/*--------------------------------------------------------------------------------------------------
	PUBLIC FUNCTION PROTOTYPE
--------------------------------------------------------------------------------------------------*/

void 		Arrosage_Init(void);
void		Arrosage_Management(void);
void		Arrosage_Start(uint32_t VolumeParPlant_ml);
void		Arrosage_Stop(void);
void		Arrosage_SetVolumeReservoir(uint32_t VolumeReservoirRestant_ml);
Arrosage_t*	Arrosage_Get(void);


#endif /* ARROSAGE_H_ */
