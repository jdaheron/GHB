/**************************************************************************************************/
/*																								  */
/* Parametres																							  */
/*																								  */
/**************************************************************************************************/

#ifndef PARAMETRES_H_
#define PARAMETRES_H_


/*--------------------------------------------------------------------------------------------------
	PUBLIC INCLUDES
--------------------------------------------------------------------------------------------------*/

#include <FILES/fct_IniFile.h>
#include "BSP.h"



/*--------------------------------------------------------------------------------------------------
	PUBLIC DEFINE
--------------------------------------------------------------------------------------------------*/


/*--------------------------------------------------------------------------------------------------
	PUBLIC TYPEDEF
--------------------------------------------------------------------------------------------------*/


/*--------------------------------------------------------------------------------------------------
	PUBLIC FUNCTION PROTOTYPE
--------------------------------------------------------------------------------------------------*/

Status_e Parametres_Init(IniFile_s*	IniFile);
Status_e Parametres_OpenReadFile(IniFile_s* IniFile);
Status_e Parametres_CloseFile(IniFile_s* IniFile);
Status_e Parametres_Read(IniFile_s* IniFile, uint32_t NumParam, void* pValue);
Status_e Parametres_Write(IniFile_s* IniFile, uint32_t NumParam, void* Value);
Status_e Parametres_EcrireDonnee(IniFile_s* IniFile, uint8_t NumData, void* value);
Status_e Parametres_EcrireFichier(IniFile_s* IniFile);


#endif /* PARAMETRES_H_ */

