/**************************************************************************************************/
/*																								  */
/* Gestion du chauffage																			  */
/*																								  */
/**************************************************************************************************/

#ifndef CHAUFFAGE_H_
#define CHAUFFAGE_H_


/*--------------------------------------------------------------------------------------------------
	PUBLIC INCLUDES
--------------------------------------------------------------------------------------------------*/

#include "BSP.h"


/*--------------------------------------------------------------------------------------------------
	PUBLIC DEFINE
--------------------------------------------------------------------------------------------------*/


/*--------------------------------------------------------------------------------------------------
	PUBLIC TYPEDEF
--------------------------------------------------------------------------------------------------*/

typedef struct
{
	Bool_e		Cfg_Restored;
	uint32_t	Cfg_SeuilStart_DegC;
	uint32_t	Cfg_SeuilStop_DegC;
	uint32_t	Cfg_TempoApresCh_s;

} Chauffage_t;


/*--------------------------------------------------------------------------------------------------
	PUBLIC FUNCTION PROTOTYPE
--------------------------------------------------------------------------------------------------*/

void 			Chauffage_Init(Bool_e UseConfIni);
void			Chauffage_Management(void);
Chauffage_t*	Chauffage_Get(void);


#endif /* CHAUFFAGE_H_ */
