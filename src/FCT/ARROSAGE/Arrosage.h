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
	Bool_e 			Cfg_Restored;
	Bool_e 			Cfg_SaveNeeded;
	char			Cfg_Heure[8];
	uint32_t		Cfg_Intervalle_h;
	uint32_t		Cfg_VolumeParPlant_ml;
	uint32_t		Cfg_NbPlants;
	uint32_t		Cfg_DebitPompe_ml_par_h;
	uint32_t		Cfg_VolumeReservoir_ml;

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
uint32_t	Arrosage_Start(uint32_t VolumeParPlant_ml);
void		Arrosage_Stop(void);
void		Arrosage_SetVolumeReservoir(uint32_t VolumeReservoirRestant_ml);
Bool_e		Arrosage_IsActive(void);
Arrosage_t*	Arrosage_Get(void);


#endif /* ARROSAGE_H_ */
