/**************************************************************************************************/
/*																								  */
/* Gestion de l'hygrometrie																	  */
/*																								  */
/**************************************************************************************************/

#ifndef HYGROMETRIE_H_
#define HYGROMETRIE_H_


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
	uint32_t		Cfg_SeuilStop_Deg;
	uint32_t		Cfg_SeuilStart_Deg;
	uint32_t		Cfg_TempoApresProd_s;

	TSW_s			TmrMngt;
	TSW_s			TmrTempo;

	MappingGpio_e	GPIO;
	Etat_e			Etat;

} Hygrometrie_t;


/*--------------------------------------------------------------------------------------------------
	PUBLIC FUNCTION PROTOTYPE
--------------------------------------------------------------------------------------------------*/

void 			Hygrometrie_Init(Bool_e UseConfIni);
void			Hygrometrie_Management(void);
Hygrometrie_t*	Hygrometrie_Get(void);


#endif /* HYGROMETRIE_H_ */
