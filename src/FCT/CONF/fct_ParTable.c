#if 0
/**************************************************************************************************/
/*																								  */
/* Gestion de la table de Parametres															  */
/*																								  */
/**************************************************************************************************/


/*--------------------------------------------------------------------------------------------------
	INCLUDES
--------------------------------------------------------------------------------------------------*/

#include "fct_ParTable.h"
#include "fct_DatabaseEeprom.h"

#include "util_CONSOLE.h"


/*--------------------------------------------------------------------------------------------------
	EXPORTED VARIABLES
--------------------------------------------------------------------------------------------------*/

 
 
/*--------------------------------------------------------------------------------------------------
	PRIVATE DEFINE
--------------------------------------------------------------------------------------------------*/

#define LogId			"DB_EEPROM"


/*--------------------------------------------------------------------------------------------------
	PRIVATE TYPEDEF
--------------------------------------------------------------------------------------------------*/


typedef struct
{
  uint32_t	RamValue;		// Image in RAM
  uint32_t	u32Address;		// Parameter address
  Bool_e    SyncNeeded;		// Parameter needs to by updated in RAMTRON

} PARAMETERS_tParam;



/*--------------------------------------------------------------------------------------------------
	PRIVATE DATA DECLARATION
--------------------------------------------------------------------------------------------------*/



/*------------------------------------------------------------------------------------------------*/
void ParTable_Init(void)
{

}



void
DatabaseEeprom_InitData(
		DatabaseEeprom_Data_e	DatabaseEeprom_Data,	/**<[in] Index de la donnees dans la base.*/
		void*					pDefaultData,			/**<[in] Valeurs par defaut.*/
		uint32_t				DataSize				/**<[in] Taille de la donnees dans la base.*/
);






 /**
 * @}
 */ 

/* End Of File ************************************************************************************/
#endif
