/**************************************************************************************************/
/*																								  */
/* Gestion des commandes du terminal															  */
/*																								  */
/**************************************************************************************************/


/*--------------------------------------------------------------------------------------------------
	INCLUDES
--------------------------------------------------------------------------------------------------*/

#include "Terminal.h"

#include <FILES/FatFs/ff.h>
#include "util_TSW.h"
#include "util_printf.h"
#include "Arrosage.h"


/*--------------------------------------------------------------------------------------------------
	PRIVATE DEFINE
--------------------------------------------------------------------------------------------------*/


/*--------------------------------------------------------------------------------------------------
	PRIVATE TYPEDEF
--------------------------------------------------------------------------------------------------*/

/** Structure de gestion d'une commande. */
typedef struct
{
	const char*		CmdString;			/**< Chaine de commande.*/
	pCommand_f		pCmdFunction;		/**< Fonction a executer.*/
	const char*		DescriptorString;	/**< Chaine de description.*/

} Cmd_s;



/*--------------------------------------------------------------------------------------------------
	PRIVATE DATA DECLARATION
--------------------------------------------------------------------------------------------------*/

static char* Param[16];


/*--------------------------------------------------------------------------------------------------
	FUNCTIONS DEFINITIONS
--------------------------------------------------------------------------------------------------*/


/*------------------------------------------------------------------------------------------------*/
void Cmd_Quit(char* bufferIn, pSendResponse_f Terminal_Write)
{
	Terminal_Write("Cmd_Quit\n");
}


/*------------------------------------------------------------------------------------------------*/
void Cmd_Reboot(char* bufferIn, pSendResponse_f Terminal_Write)
{
	Terminal_Write("Reboot bms...\n");
	GOTO(0);
}


/*------------------------------------------------------------------------------------------------*/
void Cmd_ListFiles(char* bufferIn, pSendResponse_f Terminal_Write)
{
	char tmpBuffer[1024];

	if (strlen(bufferIn) > 3)
		MemoireFAT_PrintFileList(bufferIn + 3, tmpBuffer, 1024);
	else
		MemoireFAT_PrintFileList("", tmpBuffer, 1024);

	Terminal_Write(tmpBuffer);
}


/*------------------------------------------------------------------------------------------------*/
void Cmd_Test(char* bufferIn, pSendResponse_f Terminal_Write)
{
	uint8_t NbParam = Conv_ParseString(bufferIn, TERMINAL_CMD_DELIMITER, Param);
	uint32_t TS;

	TS = RTC_GetTimestamp();
}


/*------------------------------------------------------------------------------------------------*/
void Cmd_Delete(char* bufferIn, pSendResponse_f Terminal_Write)
{
	uint8_t NbParam = Conv_ParseString(bufferIn, TERMINAL_CMD_DELIMITER, Param);

	if (NbParam != 2)
	{
		Terminal_Write("delete : WRONG PARAM\n");
		return;
	}

	if (f_unlink(Param[1]) == FR_OK)
	{
		Terminal_Write("delete : OK\n");
	}
	else
	{
		Terminal_Write("delete : ERROR\n");
	}
}


/*------------------------------------------------------------------------------------------------*/
void Cmd_Rename(char* bufferIn, pSendResponse_f Terminal_Write)
{
	uint8_t NbParam = Conv_ParseString(bufferIn, TERMINAL_CMD_DELIMITER, Param);

	if (NbParam != 3)
	{
		Terminal_Write("rename : WRONG PARAM\n");
		return;
	}

	if (f_rename(Param[1], Param[2]) == FR_OK)
	{
		Terminal_Write("rename : OK\n");
	}
	else
	{
		Terminal_Write("rename : ERROR\n");
	}
}


/*------------------------------------------------------------------------------------------------*/
void Cmd_Read(char* bufferIn, pSendResponse_f Terminal_Write)
{
	uint8_t NbParam = Conv_ParseString(bufferIn, TERMINAL_CMD_DELIMITER, Param);
	FIL File;
	Horodatage_s Time;
	char Buffer[1024];
	int NbRead;


	if (NbParam != 2)
	{
		Terminal_Write("read : WRONG PARAM\n");
		return;
	}

	if (f_open(&File, Param[1], FA_READ) != FR_OK)
	{
		Terminal_Write("read : Open ERROR\n");
		return;
	}
	f_read(&File, Buffer, 1024, &NbRead);
	f_close(&File);

	Terminal_Write(Buffer);
}


/*------------------------------------------------------------------------------------------------*/
void Cmd_Rtc(char* bufferIn, pSendResponse_f Terminal_Write)
{
	Horodatage_s Time;
	char tmpBuffer[128];
	char* LUNDI		= {"LUN"};
	char* MARDI		= {"MAR"};
	char* MERCREDI	= {"MER"};
	char* JEUDI		= {"JEU"};
	char* VENDREDI	= {"VEN"};
	char* SAMEDI	= {"SAM"};
	char* DIMANCHE	= {"DIM"};
	char* Jour;


	uint8_t NbParam = Conv_ParseString(bufferIn, TERMINAL_CMD_DELIMITER, Param);

	if (((strncmp(Param[1], "get", 3) == 0) && (NbParam != 2))
	||	((strncmp(Param[1], "set", 3) == 0) && (NbParam != 9)))
	{
		Terminal_Write("rtc WRONG PARAM\n");
		return;
	}
	RTC_Lire(&Time);


	if (strncmp(Param[1], "set", 3) == 0)
	{
		_sprintf(tmpBuffer, "RTC = %d %02d %02d %02d %02d %02d %08d",
			Time.Annee, Time.Mois, Time.Jour,
			Time.Heure, Time.Minute, Time.Seconde,
			TSW_GetTimestamp_ms());

		if 		(strncmp(Param[2], LUNDI, 3)	== 0)	Time.JourSemaine = 1;
		else if (strncmp(Param[2], MARDI, 3)	== 0)	Time.JourSemaine = 2;
		else if (strncmp(Param[2], MERCREDI, 3)	== 0)	Time.JourSemaine = 3;
		else if (strncmp(Param[2], JEUDI, 3)	== 0)	Time.JourSemaine = 4;
		else if (strncmp(Param[2], VENDREDI, 3)	== 0)	Time.JourSemaine = 5;
		else if (strncmp(Param[2], SAMEDI, 3)	== 0)	Time.JourSemaine = 6;
		else if (strncmp(Param[2], DIMANCHE, 3)	== 0)	Time.JourSemaine = 7;
		else											Time.JourSemaine = 1;

		Time.Jour			= strtoul(Param[3], NULL ,10);
		Time.Mois			= strtoul(Param[4], NULL ,10);
		Time.Annee			= strtoul(Param[5], NULL ,10);
		Time.Heure			= strtoul(Param[6], NULL ,10);
		Time.Minute			= strtoul(Param[7], NULL ,10);
		Time.Seconde		= strtoul(Param[8], NULL ,10);

		RTC_Regler(&Time);
		Delay_ms(10);
		RTC_Lire(&Time);
		strncpy(Param[1], "get", 3);
	}

	if (strncmp(Param[1], "get", 3) == 0)
	{
		switch (Time.JourSemaine)
		{
			case 1 : Jour = LUNDI	;	break;
			case 2 : Jour = MARDI	;	break;
			case 3 : Jour = MERCREDI;	break;
			case 4 : Jour = JEUDI	;	break;
			case 5 : Jour = VENDREDI;	break;
			case 6 : Jour = SAMEDI	;	break;
			case 7 : Jour = DIMANCHE;	break;
		}

		_sprintf(tmpBuffer, "%s %02d %02d %d %02d %02d %02d %08d",
					Jour, Time.Jour, Time.Mois, Time.Annee,
					Time.Heure, Time.Minute, Time.Seconde,
					TSW_GetTimestamp_ms());

		Terminal_Write(tmpBuffer);
	}
}


/*------------------------------------------------------------------------------------------------*/
void Cmd_Arroser(char* bufferIn, pSendResponse_f Terminal_Write)
{
	uint8_t NbParam = Conv_ParseString(bufferIn, TERMINAL_CMD_DELIMITER, Param);

	if (NbParam != 2)
	{
		Terminal_Write("arroser : WRONG PARAM\n");
		return;
	}

	uint32_t VolumeParPlant_ml = strtoul(Param[1], NULL ,10);

	Arrosage_Start(VolumeParPlant_ml);
}


/*------------------------------------------------------------------------------------------------*/
void Cmd_Reservoir(char* bufferIn, pSendResponse_f Terminal_Write)
{
	uint8_t NbParam = Conv_ParseString(bufferIn, TERMINAL_CMD_DELIMITER, Param);
	char tmpBuffer[128];
	uint32_t VolumeReservoirRestant_ml;


	if (((strncmp(Param[1], "get", 3) == 0) && (NbParam != 2))
	||	((strncmp(Param[1], "set", 3) == 0) && (NbParam != 3)))
	{
		Terminal_Write("reservoir WRONG PARAM\n");
		return;
	}


	if (strncmp(Param[1], "set", 3) == 0)
	{
		VolumeReservoirRestant_ml = strtoul(Param[2], NULL ,10);
		Arrosage_SetVolumeReservoir(VolumeReservoirRestant_ml);
	}

	_sprintf(tmpBuffer, "%d", Arrosage_Get()->VolumeRestant_ml);
	Terminal_Write(tmpBuffer);
}


/*------------------------------------------------------------------------------------------------*/
void Terminal_Cmd_Init(void)
{
	//TODO : DEBUG
	Terminal_RegisterCommand("quit",		Cmd_Quit,		"Fermeture de la communication");
	Terminal_RegisterCommand("ls",			Cmd_ListFiles,	"Affichage de la liste des fichiers d'un repertoire");
	Terminal_RegisterCommand("reboot",		Cmd_Reboot,		"Redémarrage SW");
	Terminal_RegisterCommand("test",		Cmd_Test,		"Commande de test");
	Terminal_RegisterCommand("delete",		Cmd_Delete,		"Effacement d'un fichier");
	Terminal_RegisterCommand("rename",		Cmd_Rename,		"Renommage d'un fichier");
	Terminal_RegisterCommand("read",		Cmd_Read,		"Lecture dun contenu d'un fichier");
	Terminal_RegisterCommand("rtc",			Cmd_Rtc,		"Gestion de la RTC");
	Terminal_RegisterCommand("arroser",		Cmd_Arroser,	"Lancement d'un arrosage immédiat");
	Terminal_RegisterCommand("reservoir",	Cmd_Reservoir,	"Ecriture de l'etat du remplissage du reservoir");
}


/*------------------------------------------------------------------------------------------------*/
