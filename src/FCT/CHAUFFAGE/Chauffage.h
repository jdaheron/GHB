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
	uint32_t	Cfg_SeuilStart_DegC;
	uint32_t	Cfg_SeuilStop_DegC;
	uint32_t	Cfg_TempoApresCh_s;

} Chauffage_t;


/*--------------------------------------------------------------------------------------------------
	PUBLIC FUNCTION PROTOTYPE
--------------------------------------------------------------------------------------------------*/

void 			Chauffage_Init(void);
void			Chauffage_Management(void);
Chauffage_t*	Chauffage_Get(void);


#endif /* CHAUFFAGE_H_ */
