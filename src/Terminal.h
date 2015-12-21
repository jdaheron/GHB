/**************************************************************************************************/
/*																								  */
/* Gestion du terminal																			  */
/*																								  */
/**************************************************************************************************/

#ifndef UTIL_TERMINAL_H_
#define UTIL_TERMINAL_H_


/*--------------------------------------------------------------------------------------------------
	PUBLIC INCLUDES
--------------------------------------------------------------------------------------------------*/

#include "BSP.h"


/*--------------------------------------------------------------------------------------------------
	PUBLIC DEFINE
--------------------------------------------------------------------------------------------------*/

#define TERMINAL_WELCOM			"Welcome to JD TERMINAL"
#define TERMINAL_PROMPT			"JdTerm>"
#define TERMINAL_CMD_DELIMITER	' '
#define TERMINAL_NB_CMD_MAX		32


/*--------------------------------------------------------------------------------------------------
	PUBLIC TYPEDEF
--------------------------------------------------------------------------------------------------*/

typedef void (*pSendResponse_f)(char*);
typedef void (*pCommand_f)(char* bufferIn, pSendResponse_f Terminal_Write);


/*--------------------------------------------------------------------------------------------------
	PUBLIC FUNCTION PROTOTYPE
--------------------------------------------------------------------------------------------------*/
void	 Terminal_Init(void);
Status_e Terminal_RegisterCommand(const char* CmdString, pCommand_f pCmdFunction, const char* DescriptorString);
Status_e Terminal_Parser(char* bufferIn, char* bufferOut, uint16_t MaxOutSize);


#endif /* UTIL_TERMINAL_H_ */
