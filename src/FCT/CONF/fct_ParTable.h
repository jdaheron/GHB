#if 0
/**************************************************************************************************/
/*																								  */
/* Gestion de la table de Parametres															  */
/*																								  */
/**************************************************************************************************/

#ifndef FCT_PARTABLE_H_
#define FCT_PARTABLE_H_


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

/** Liste des donnees de la base de donnees. */
typedef enum
{
	ParTable_0 = 0,
	ParTable_1,
	ParTable_2,

	ParTable_QTT

} ParTable_e;


/*--------------------------------------------------------------------------------------------------
	PUBLIC FUNCTION PROTOTYPE
--------------------------------------------------------------------------------------------------*/

void ParTable_Init();


#endif /* FCT_PARTABLE_H_ */
#endif

