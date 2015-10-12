/**
 ***************************************************************************************************
 * @file	util_Terminal.h
 * @author	Julien
 * @version	1.0.0
 * @date	18 nov. 2014
 * @brief   Gestion du terminal.
 ***************************************************************************************************
 */


/* Define to prevent recursive inclusion **********************************************************/

#ifndef UTIL_TERMINAL_H_
#define UTIL_TERMINAL_H_


/* Includes ***************************************************************************************/

#include "BSP.h"


/** @defgroup Terminal Terminal
  * @brief Gestion du terminal.
  * @{
  */ 

/** 
 ***************************************************************************************************
 * @defgroup Exported_Defines Exported Defines
 * @{
 */

#define PROMPT					"JdTerm>"
#define TERMINAL_NB_CMD_MAX		20


 /**
 * @}
 */ 
 
 
/** 
 ***************************************************************************************************
 * @defgroup Exported_TypesDefinitions Exported TypesDefinitions
 * @{
 */

typedef void (*pSendResponse_f)(char*);
typedef void (*pCommand_f)(char* bufferIn, pSendResponse_f Terminal_Write);


/**
 * @}
 */ 
 
 
/** 
 ***************************************************************************************************
 * @defgroup Exported_Macros Exported Macros 
 * @{
 */
 
 /**
 * @}
 */ 
 
 
/** 
 ***************************************************************************************************
 * @defgroup Exported_Variables Exported Variables
 * @{
 */
 
 /**
 * @}
 */ 

 
/** 
 ***************************************************************************************************
 * @defgroup Exported_Functions Exported Functions  
 * @{
 */


/**
 * @brief	Initialisation du terminal.
 * @return 	void
 */
void
Terminal_Init(
		void
);

/**
* @brief	Enregistrement d'une commande.
* @return 	Status(OK/KO)
*/
Status_e
Terminal_RegisterCommand(
		const char*		CmdString,			/**< Chaine de commande.*/
		pCommand_f		pCmdFunction,		/**< Fonction a executer.*/
		const char*		DescriptorString	/**< Chaine de description.*/
);


/**
 * @brief	Traitement d'une trame recue.
 * @return 	Status(OK/KO)
 */
Status_e
Terminal_Parser(
		char*		bufferIn,			/**< Trame d'entreee, a parser.*/
		char*		bufferOut,			/**< Trame de reponse.*/
		uint16_t	MaxOutSize			/**< Taille maxi de la trame de reponse.*/
);

	 
 /**
 * @}
 */ 
 
 
 /**
 * @}
 */ 


#endif /* UTIL_TERMINAL_H_ */
