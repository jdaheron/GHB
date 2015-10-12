/**
 ***************************************************************************************************
 * @file	Terminal.c
 * @author	Julien D.
 * @version	1.0.0
 * @date	18 nov. 2014
 * @brief   Gestion du terminal.
 ***************************************************************************************************
 */


/* Includes ***************************************************************************************/

#include "Terminal.h"



/* External Variables *****************************************************************************/


/** @addtogroup Terminal
  * @{
  */ 

 
 /** 
 ***************************************************************************************************
 * @defgroup Private_TypesDefinitions Private TypesDefinitions
 * @{
 */



/** Structure de gestion d'une commande. */
typedef struct
{
	const char*		CmdString;			/**< Chaine de commande.*/
	pCommand_f		pCmdFunction;		/**< Fonction a executer.*/
	const char*		DescriptorString;	/**< Chaine de description.*/

} Cmd_s;

/**
 * @}
 */ 

 
 /** 
 ***************************************************************************************************
 * @defgroup Private_Defines Private Defines
 * @{
 */
 
/**
 * @}
 */ 

 
 /** 
 ***************************************************************************************************
 * @defgroup Private_Macros Private Macros
 * @{
 */
  
/**
 * @}
 */ 

  
 /** 
 ***************************************************************************************************
 * @defgroup Private_FunctionPrototypes Private FunctionPrototypes
 * @{
 */

/**
 * @}
 */ 
 
 
  /** 
 ***************************************************************************************************
 * @defgroup Private_Variables Private Variables
 * @{
 */

static Cmd_s	Cmd[TERMINAL_NB_CMD_MAX];
static uint16_t	NbCmd = 0;
static uint16_t CurrentOut_MaxSize = 0;
static char*	CurrentOut_Buffer = NULL;
static uint16_t	CurrentOut_Size = 0;
static Bool_e	CurrentOut_Error = FALSE;




//static char TermBuffer[2000];

void Cmd_Quit(
		char*			bufferIn,				/**< Trame d'entreee, non utilisee.*/
		pSendResponse_f	Terminal_Write			/**< Fonction d'emission de la reponse.*/
)
{
	Terminal_Write("Cmd_Quit\n");
}

void Cmd_ListFiles(
		char*			bufferIn,				/**< Trame d'entreee, non utilisee.*/
		pSendResponse_f	Terminal_Write			/**< Fonction d'emission de la reponse.*/
)
{
	char tmpBuffer[1024];

	if (strlen(bufferIn) > 3)
		MemoireFAT_PrintFileList(bufferIn + 3, tmpBuffer, 1024);
	else
		MemoireFAT_PrintFileList("", tmpBuffer, 1024);

	Terminal_Write(tmpBuffer);
}





/**
 * @}
 */ 
  	
	
/** 
 ***************************************************************************************************
 * @defgroup Private_Functions Private Functions
 * @{
 */ 
 

/**
 ***************************************************************************************************
 * @brief	Traitement des chaines de sortie terminal.
 * @return 	void
 */
static void
Terminal_Output(
		char* Buffer
)
{
	if (Buffer == NULL)
		return;

	if (CurrentOut_Buffer == NULL)
		return;

	if (CurrentOut_Error == TRUE)
		return;

	// REcuperation de la taille a ajouter
	uint16_t AddLength = strlen(Buffer);

	// Verification depassement
	if (CurrentOut_Size + AddLength >= CurrentOut_MaxSize)
	{
		strcpy(CurrentOut_Buffer, "Terminal buffer overflow\n");
		CurrentOut_Error = TRUE;
	}
	// Ajout si pas de depassement
	else
	{
		strcat(CurrentOut_Buffer, Buffer);
		CurrentOut_Size += AddLength;
	}
}


/**
 ***************************************************************************************************
 * @brief	Impression de la liste de commandes disponibles.
 * @return 	void
 */
static void
Terminal_PrintCmdList(
		char*			bufferIn,				/**< Trame d'entreee, non utilisee.*/
		pSendResponse_f	Terminal_Write			/**< Fonction d'emission de la reponse.*/
)
{
	if (Terminal_Write == NULL)
		return;

	char buff[64];
	uint16_t MaxLenght = 11;
	uint16_t CmdLenght;

	for (uint16_t i=0; i<NbCmd; i++)
	{
		CmdLenght = strlen(Cmd[i].CmdString);
		if (CmdLenght > MaxLenght)
			MaxLenght = CmdLenght;
	}
	MaxLenght += 4;	// 2 espaces avant et apres le nom de la commande.

	memset(buff, ' ', MaxLenght);
	buff[MaxLenght] = 0;
	memcpy(buff, " Command", 8);
	strcat(buff, "Description\n");
	Terminal_Write(buff);

	for (uint16_t i=0; i<NbCmd; i++)
	{
		memset(buff, ' ', MaxLenght);
		buff[MaxLenght] = 0;
		memcpy(&buff[2], Cmd[i].CmdString, strlen(Cmd[i].CmdString));
		Terminal_Write(buff);
		Terminal_Write(Cmd[i].DescriptorString);
		Terminal_Write("\n");
	}

}


 /**
 * @}
 */ 

#if 0
void Terminal_Parser(char* buffer)
{
	char* pDeb;
	char* pParam0;
/*
	if (strstr(buffer, "quit"))
	{
		hs->left = 0;
		hs->flags |= TELNET_FLAG_CLOSE_CONNECTION;
	}
	else if (pDeb = strstr(p, "ls"))
	{

	}
	else if (pDeb = strstr(p, "get_v"))
	{
		usprintf(tstr,"VersionSW = AL%s\r\n", VERSION_SW);
		hs->data_out = tstr;
		hs->left = strlen(hs->data_out);
		addPrompt(hs);
	}
	else if (pDeb = strstr(p, "reboot"))
	{
		usprintf(tstr,"Rebooting device...\r\n");
		hs->data_out = tstr;
		hs->left = strlen(hs->data_out);
		addPrompt(hs);

		GOTO(0);
	}
	else if (pDeb = strstr(p, "delete "))
	{
		pParam0 = pDeb + strlen("delete ");
		f_unlink(pParam0);
		usprintf(tstr,"Delete file: %s\r\n", pParam0);
		hs->data_out = tstr;
		hs->left = strlen(hs->data_out);
		addPrompt(hs);
	}
	else
	{
		usprintf(tstr,"\r\nThis is TELNET echoing your command : \"%s\"%s",p,prompt);
		hs->data_out = tstr;
		hs->left = strlen(hs->data_out);
	}

	hs->cmd_buffer[0] = 0;
	// copy any remaining part of command line to position 0
	//strcpy(hs->cmd_buffer,q+1);
*/
}
#endif



/** 
 ***************************************************************************************************
 * @brief	Initialisation du terminal.
 * @return 	void
 */
void
Terminal_Init(
		void
)
{
	// Initialisation de la table de commandes
	memset((void*) Cmd, 0, TERMINAL_NB_CMD_MAX * sizeof(Cmd_s));
	NbCmd = 0;

	// Declaration (systematique) de la commande "list"
	Terminal_RegisterCommand("list",	Terminal_PrintCmdList,	"Impression de la liste de commandes disponibles");



	//TODO : DEBUG
	Terminal_RegisterCommand("quit",	Cmd_Quit,				"Fonction d'arret");
	Terminal_RegisterCommand("ls",		Cmd_ListFiles,			"Affichage de la liste des fichiers d'un repertoire");
}

/**
 ***************************************************************************************************
 * @brief	Enregistrement d'une commande.
 * @return 	Status(OK/KO)
 */
Status_e
Terminal_RegisterCommand(
		const char*		CmdString,				/**< Chaine de commande.*/
		pCommand_f		pCmdFunction,			/**< Fonction a executer.*/
		const char*		DescriptorString		/**< Chaine de description.*/
)
{
	if (CmdString == NULL)
		return Status_KO;

	if (CmdString[0] == '0')
		return Status_KO;

	if (pCmdFunction == NULL)
		return Status_KO;

	if (NbCmd >= TERMINAL_NB_CMD_MAX)
		return Status_KO;

	Cmd[NbCmd].CmdString		= CmdString;
	Cmd[NbCmd].pCmdFunction		= pCmdFunction;
	Cmd[NbCmd].DescriptorString	= DescriptorString;

	NbCmd++;

	return Status_OK;
}


/**
 ***************************************************************************************************
 * @brief	Parser de trame recue.
 * @return 	Status(OK/KO)
 */
Status_e
Terminal_Parser(
		char*		bufferIn,			/**< Trame d'entreee, a parser.*/
		char*		bufferOut,			/**< Trame de reponse.*/
		uint16_t	MaxOutSize			/**< Taille maxi de la trame de reponse.*/
)
{
	Bool_e TrameTraitee = FALSE;
	uint16_t iCmd = 0;
	uint16_t CmdLength;
	char* pDeb;
	char* pFin;

	// Velification des parametres d'entree
	if (bufferIn == NULL)
		return Status_KO;

	if (bufferOut == NULL)
		return Status_KO;

	if (MaxOutSize < (1 + strlen(PROMPT)))
		return Status_KO;

	// Initialisation du buffer de sortie
	CurrentOut_MaxSize		= MaxOutSize - (1 + strlen(PROMPT));
	CurrentOut_Buffer		= bufferOut;
	CurrentOut_Size			= 0;
	CurrentOut_Error		= FALSE;
	CurrentOut_Buffer[0]	= 0;

	// Suppression '\n' en fin de chaine
	pFin = strchr(bufferIn, '\n');
	if (pFin != NULL)
		*pFin = 0;

	// Recherche de la commande a executer
	do
	{
		pDeb = strstr(bufferIn, Cmd[iCmd].CmdString);
		if (pDeb != NULL)
		{
			// Verification du caractere suivant la cmd
			CmdLength = strlen(Cmd[iCmd].CmdString);
			if ((pDeb[CmdLength] == ' ') || (pDeb[CmdLength] == 0))
			{
				// Echo commande
				CmdLength = strlen(pDeb);
				if (CmdLength > MaxOutSize)
					return Status_KO;
				Terminal_Output(pDeb);
				Terminal_Output("\n");

				// Appel de la fonction de traitement de la commande
				if (Cmd[iCmd].pCmdFunction != NULL)
					Cmd[iCmd].pCmdFunction(pDeb, Terminal_Output);

				TrameTraitee = TRUE;
			}
		}

		iCmd++;
	}
	while ((Cmd[iCmd].CmdString != NULL) && (TrameTraitee == FALSE));

	if (TrameTraitee == FALSE)
	{
		Terminal_Output("Commande non reconnue\n");
	}

	// Ajout prompt
	CmdLength = strlen(bufferOut);
	if (bufferOut[CmdLength-1] != '\n')	// Ajout \n si necessaire
		Terminal_Output("\n");
	Terminal_Output(PROMPT);

	if (TrameTraitee)
		return Status_OK;

	return Status_KO;
}



 /**
 * @}
 */ 

/* End Of File ************************************************************************************/
