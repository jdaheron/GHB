/**************************************************************************************************/
/*																								  */
/* Gestion des commandes du terminal															  */
/*																								  */
/**************************************************************************************************/


/*--------------------------------------------------------------------------------------------------
	INCLUDES
--------------------------------------------------------------------------------------------------*/

#include "Terminal.h"
#include "Terminal_Cmd.h"

#include <FILES/FatFs/ff.h>
#include "util_TSW.h"
#include "util_printf.h"
#include "util_Conversions.h"
#include "Arrosage.h"
#include "Chauffage.h"
#include "Ethernet.h"
#include "Hygrometrie.h"
#include "Ventilation.h"
#include "Modes.h"
#include "fct_MemoireFAT.h"


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

extern Etat_e		EtatVentillation;
extern Etat_e		EtatChauffage;
extern Mode_e		Mode;
extern float		Temperature;
extern float		Hygrometrie;
extern Horodatage_s StartTime;

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
	uint8_t NbParam = Conv_ParseString(bufferIn, TERMINAL_CMD_DELIMITER, Param);
	char tmpBuffer[1024];


	if (NbParam > 2)
	{
		Terminal_Write("ls : WRONG PARAM\n");
		return;
	}

	if (MemoireFAT_IsReady() == FALSE)
	{
		Terminal_Write("Memory Not ready\n");
		return;
	}


	if (NbParam == 2)
	//if (strlen(bufferIn) > 3)
		MemoireFAT_PrintFileList(Param[1], tmpBuffer, 1024);
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

	if (MemoireFAT_IsReady() == FALSE)
	{
		Terminal_Write("Memory Not ready\n");
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
void Cmd_Format(char* bufferIn, pSendResponse_f Terminal_Write)
{
	uint8_t NbParam = Conv_ParseString(bufferIn, TERMINAL_CMD_DELIMITER, Param);


	if (NbParam != 1)
	{
		Terminal_Write("format : WRONG PARAM\n");
		return;
	}

	if (MemoireFAT_IsReady() == FALSE)
	{
		Terminal_Write("Memory Not ready\n");
		return;
	}

	if (MemoireFAT_Erase() == FR_OK)
	{
		Terminal_Write("format : OK\n");
	}
	else
	{
		Terminal_Write("format : ERROR\n");
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

	if (MemoireFAT_IsReady() == FALSE)
	{
		Terminal_Write("Memory Not ready\n");
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

	if (MemoireFAT_IsReady() == FALSE)
	{
		Terminal_Write("Memory Not ready\n");
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
	char day[10];


	uint8_t NbParam = Conv_ParseString(bufferIn, TERMINAL_CMD_DELIMITER, Param);

	if (((strncmp(Param[1], "get", 3) == 0) && (NbParam != 2))
	||	((strncmp(Param[1], "set", 3) == 0) && (NbParam != 9)))
	{
		Terminal_Write("rtc WRONG PARAM\n");
		return;
	}

	RTC_Lire(&Time);

	//--------------------------------------------------------------------------------
	if (strncmp(Param[1], "set", 3) == 0)
	{
		if 		(!strncmp(Param[2], RTC_GetDayString(LUNDI,		3), 3))	Time.JourSemaine = 1;
		else if (!strncmp(Param[2], RTC_GetDayString(MARDI,		3), 3))	Time.JourSemaine = 2;
		else if (!strncmp(Param[2], RTC_GetDayString(MERCREDI,	3), 3))	Time.JourSemaine = 3;
		else if (!strncmp(Param[2], RTC_GetDayString(JEUDI, 	3), 3))	Time.JourSemaine = 4;
		else if (!strncmp(Param[2], RTC_GetDayString(VENDREDI,	3), 3))	Time.JourSemaine = 5;
		else if (!strncmp(Param[2], RTC_GetDayString(SAMEDI,	3), 3))	Time.JourSemaine = 6;
		else if (!strncmp(Param[2], RTC_GetDayString(DIMANCHE,	3), 3))	Time.JourSemaine = 7;
		else															Time.JourSemaine = 1;

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

	//--------------------------------------------------------------------------------
	if (strncmp(Param[1], "get", 3) == 0)
	{
		memset(day, 0, 10);
		strncpy(day, RTC_GetDayString(Time.JourSemaine, 3), 3);
		_sprintf(tmpBuffer, "%s %02d %02d %d %02d %02d %02d %08d",
				day, Time.Jour, Time.Mois, Time.Annee,
				Time.Heure, Time.Minute, Time.Seconde,
				TSW_GetTimestamp_ms());

		Terminal_Write(tmpBuffer);
	}
}


/*------------------------------------------------------------------------------------------------*/
void Cmd_StartupTime(char* bufferIn, pSendResponse_f Terminal_Write)
{
	char tmpBuffer[128];
	char day[10];


	uint8_t NbParam = Conv_ParseString(bufferIn, TERMINAL_CMD_DELIMITER, Param);

	if (NbParam != 1)
	{
		Terminal_Write("startup : WRONG PARAM\n");
		return;
	}

	memset(day, 0, 10);
	strncpy(day, RTC_GetDayString(StartTime.JourSemaine, 3), 10);
	_sprintf(tmpBuffer, "%s %02d %02d %d %02d %02d %02d %08d",
			day, StartTime.Jour, StartTime.Mois, StartTime.Annee,
			StartTime.Heure, StartTime.Minute, StartTime.Seconde,
			TSW_GetTimestamp_ms());

	Terminal_Write(tmpBuffer);
}


/*------------------------------------------------------------------------------------------------*/
void Cmd_Arroser(char* bufferIn, pSendResponse_f Terminal_Write)
{
	uint8_t NbParam = Conv_ParseString(bufferIn, TERMINAL_CMD_DELIMITER, Param);
	uint32_t VolumeParPlant_ml;
	uint32_t Duree_s;
	char tmpBuffer[128];


	if (NbParam != 2)
	{
		Terminal_Write("arroser : WRONG PARAM\n");
		return;
	}

	VolumeParPlant_ml = strtoul(Param[1], NULL ,10);
	Duree_s = Arrosage_Start(VolumeParPlant_ml);

	_sprintf(tmpBuffer, "Duree = %ds\n", Duree_s);
	Terminal_Write(tmpBuffer);
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
void Cmd_Status(char* bufferIn, pSendResponse_f Terminal_Write)
{
	uint8_t NbParam = Conv_ParseString(bufferIn, TERMINAL_CMD_DELIMITER, Param);
	Horodatage_s Time;
	char tmpBuffer[256];
	char day[10];

	if (NbParam != 1)
	{
		Terminal_Write("status : WRONG PARAM\n");
		return;
	}

	RTC_Lire(&Time);

	memset(day, 0, 10);
	strncpy(day, RTC_GetDayString(Time.JourSemaine, 3), 3);
	_sprintf(tmpBuffer, "%s %02d %02d %d %02d %02d %02d %08d %d %d %d %d %d %d.%d %d.%d",
			day, Time.Jour, Time.Mois, Time.Annee,
			Time.Heure, Time.Minute, Time.Seconde,
			TSW_GetTimestamp_ms(),
			Mode,
			EtatVentillation,
			EtatChauffage,
			Arrosage_Get()->Etat,
			Hygrometrie_Get()->Etat,
			(uint16_t) Temperature,
			(uint16_t) (Temperature * 10) % 10,
			(uint16_t) Hygrometrie,
			(uint16_t) (Hygrometrie * 10) % 10
	);

	Terminal_Write(tmpBuffer);
}


/*------------------------------------------------------------------------------------------------*/
void Cmd_Cfg(char* bufferIn, pSendResponse_f Terminal_Write)
{
	uint8_t NbParam = Conv_ParseString(bufferIn, TERMINAL_CMD_DELIMITER, Param);
	char tmpBuffer[1024];
	Bool_e IsValide = FALSE;
	uint32_t Value = 0;
	char* NextParam = Param[1];


	//--------------------------------------------------------------------------------
	if ((strncmp(NextParam, "get", 3) == 0) && (NbParam == 3))
	{
		NextParam = Param[2];

		//------------------------------------------------------------
		if (strncmp(NextParam, "def", 3) == 0)
		{
			_sprintf(tmpBuffer, "CfgDefault:\nArrosage    = %d\nChauffage   = %d\nEthernet    = %d\nVentilation = %d",
				Arrosage_Get()->Cfg_Restored,
				Chauffage_Get()->Cfg_Restored,
				Ethernet_Get()->Cfg_Restored,
				Ventilation_Get()->Cfg_Restored
			);

			IsValide = TRUE;
		}

		//------------------------------------------------------------
		else if (strncmp(NextParam, "arr", 3) == 0)
		{
			_sprintf(tmpBuffer, "Heure               = %s\n",	Arrosage_Get()->Cfg_Heure					);	Terminal_Write(tmpBuffer);
			_sprintf(tmpBuffer, "Intervalle_h        = %d\n",	Arrosage_Get()->Cfg_Intervalle_h			);	Terminal_Write(tmpBuffer);
			_sprintf(tmpBuffer, "VolumeParPlant_ml   = %d\n",	Arrosage_Get()->Cfg_VolumeParPlant_ml		);	Terminal_Write(tmpBuffer);
			_sprintf(tmpBuffer, "NbPlants            = %d\n",	Arrosage_Get()->Cfg_NbPlants				);	Terminal_Write(tmpBuffer);
			_sprintf(tmpBuffer, "DebitPompe_ml_par_h = %d\n",	Arrosage_Get()->Cfg_DebitPompe_ml_par_h		);	Terminal_Write(tmpBuffer);
			_sprintf(tmpBuffer, "VolumeReservoir_ml  = %d\n",	Arrosage_Get()->Cfg_VolumeReservoir_ml		);	Terminal_Write(tmpBuffer);

			_sprintf(tmpBuffer, "STATUS:\n");
			_sprintf(tmpBuffer, "VolumeRestant_ml    = %d\n",	Arrosage_Get()->VolumeRestant_ml			);	Terminal_Write(tmpBuffer);
			_sprintf(tmpBuffer, "TS_Precedent        = %d\n",	Arrosage_Get()->TS_Precedent				);	Terminal_Write(tmpBuffer);
			_sprintf(tmpBuffer, "TS_Suivant          = %d\n",	Arrosage_Get()->TS_Suivant					);	Terminal_Write(tmpBuffer);

			_sprintf(tmpBuffer, "\n");
			IsValide = TRUE;
		}

		//------------------------------------------------------------
		else if (strncmp(NextParam, "ch", 2) == 0)
		{
			_sprintf(tmpBuffer, "SeuilStart_DegC = %d\n"	,	Chauffage_Get()->Cfg_SeuilStart_DegC	);	Terminal_Write(tmpBuffer);
			_sprintf(tmpBuffer, "SeuilStop_DegC  = %d\n"	,	Chauffage_Get()->Cfg_SeuilStop_DegC		);	Terminal_Write(tmpBuffer);
			_sprintf(tmpBuffer, "TempoApresCh_s  = %d\n"	,	Chauffage_Get()->Cfg_TempoApresCh_s		);	Terminal_Write(tmpBuffer);

			_sprintf(tmpBuffer, "\n");
			IsValide = TRUE;
		}

		//------------------------------------------------------------
		else if (strncmp(NextParam, "eth", 3) == 0)
		{
			_sprintf(tmpBuffer, "IP_Adresse    = %d.%d.%d.%d\n",		Ethernet_Get()->Cfg_IP_Adresse[0],		Ethernet_Get()->Cfg_IP_Adresse[1],		Ethernet_Get()->Cfg_IP_Adresse[2],		Ethernet_Get()->Cfg_IP_Adresse[3]);                                                                         	Terminal_Write(tmpBuffer);
			_sprintf(tmpBuffer, "IP_Masque     = %d.%d.%d.%d\n",		Ethernet_Get()->Cfg_IP_Masque[0],		Ethernet_Get()->Cfg_IP_Masque[1],		Ethernet_Get()->Cfg_IP_Masque[2],		Ethernet_Get()->Cfg_IP_Masque[3]);                                                                          	Terminal_Write(tmpBuffer);
			_sprintf(tmpBuffer, "IP_Passerelle = %d.%d.%d.%d\n", 		Ethernet_Get()->Cfg_IP_Passerelle[0],	Ethernet_Get()->Cfg_IP_Passerelle[1],	Ethernet_Get()->Cfg_IP_Passerelle[2],	Ethernet_Get()->Cfg_IP_Passerelle[3]);                                                                      	Terminal_Write(tmpBuffer);
			_sprintf(tmpBuffer, "MAC_Adresse   = %d:%d:%d:%d:%d:%d\n",	Ethernet_Get()->Cfg_MAC_Adresse[0],		Ethernet_Get()->Cfg_MAC_Adresse[1],		Ethernet_Get()->Cfg_MAC_Adresse[2],		Ethernet_Get()->Cfg_MAC_Adresse[3], Ethernet_Get()->Cfg_MAC_Adresse[4], Ethernet_Get()->Cfg_MAC_Adresse[5]);	Terminal_Write(tmpBuffer);
			_sprintf(tmpBuffer, "DHCP_Actif    = %d\n",					Ethernet_Get()->Cfg_DHCP_Actif		); Terminal_Write(tmpBuffer);

			_sprintf(tmpBuffer, "\n");
			IsValide = TRUE;
		}

		//------------------------------------------------------------
		else if (strncmp(NextParam, "ext", 3) == 0)
		{
			_sprintf(tmpBuffer, "SeuilStop_DegC    = %d\n",	Ventilation_Get()->Cfg_SeuilStop_DegC			);	Terminal_Write(tmpBuffer);
			_sprintf(tmpBuffer, "SeuilStart_DegC   = %d\n",	Ventilation_Get()->Cfg_SeuilStart_DegC			);	Terminal_Write(tmpBuffer);
			_sprintf(tmpBuffer, "TempoApresEXT_s   = %d\n",	Ventilation_Get()->Cfg_TempoApresEXT_s			);	Terminal_Write(tmpBuffer);
			_sprintf(tmpBuffer, "ActiverPendantCh. = %d\n",	Ventilation_Get()->Cfg_ActiverPendantChauffage	);	Terminal_Write(tmpBuffer);

			_sprintf(tmpBuffer, "\n");
			IsValide = TRUE;
		}
	}
	//--------------------------------------------------------------------------------
	else if ((strncmp(NextParam, "set", 3) == 0) && (NbParam == 5))
	{
		NextParam = Param[2];

		//------------------------------------------------------------
		if (strncmp(NextParam, "arr", 3) == 0)
		{
			NextParam = Param[3];

			if (strncmp(NextParam, "Intervalle", 10) == 0)
			{
				Value = strtoul(Param[4], NULL ,10);
				Arrosage_Get()->Cfg_Intervalle_h = Value;
				Arrosage_Get()->Cfg_SaveNeeded	= TRUE;
				_sprintf(tmpBuffer, "\n");
				IsValide = TRUE;
			}
			else if (strncmp(NextParam, "Volume", 6) == 0)
			{
				Value = strtoul(Param[4], NULL ,10);
				Arrosage_Get()->Cfg_VolumeParPlant_ml = Value;
				Arrosage_Get()->Cfg_SaveNeeded	= TRUE;
				_sprintf(tmpBuffer, "\n");
				IsValide = TRUE;
			}
			else if (strncmp(NextParam, "NbPlants", 8) == 0)
			{
				Value = strtoul(Param[4], NULL ,10);
				Arrosage_Get()->Cfg_NbPlants	= Value;
				Arrosage_Get()->Cfg_SaveNeeded	= TRUE;
				_sprintf(tmpBuffer, "\n");
				IsValide = TRUE;
			}
		}
	}


	if (IsValide == FALSE)
	{
		Terminal_Write("rtc WRONG PARAM\n");
		return;
	}

	Terminal_Write(tmpBuffer);

}


/*------------------------------------------------------------------------------------------------*/
void Terminal_Cmd_Init(void)
{
	//TODO : DEBUG
	Terminal_RegisterCommand("quit",		Cmd_Quit,			"Fermeture de la communication");
	Terminal_RegisterCommand("ls",			Cmd_ListFiles,		"Affichage de la liste des fichiers d'un repertoire");
	Terminal_RegisterCommand("reboot",		Cmd_Reboot,			"Redémarrage SW");
	Terminal_RegisterCommand("test",		Cmd_Test,			"Commande de test");
	Terminal_RegisterCommand("delete",		Cmd_Delete,			"Effacement d'un fichier");
	Terminal_RegisterCommand("format",		Cmd_Format,			"Formatage de la carte SD");
	Terminal_RegisterCommand("rename",		Cmd_Rename,			"Renommage d'un fichier");
	Terminal_RegisterCommand("read",		Cmd_Read,			"Lecture dun contenu d'un fichier");
	Terminal_RegisterCommand("rtc",			Cmd_Rtc,			"Gestion de la RTC");
	Terminal_RegisterCommand("startup",		Cmd_StartupTime,	"Lecture de l'heure de demarrage");
	Terminal_RegisterCommand("arroser",		Cmd_Arroser,		"Lancement d'un arrosage immédiat");
	Terminal_RegisterCommand("reservoir",	Cmd_Reservoir,		"Ecriture de l'etat du remplissage du reservoir");
	Terminal_RegisterCommand("status",		Cmd_Status,			"Lecture de l'etat de la carte");
	Terminal_RegisterCommand("cfg",			Cmd_Cfg,			"Gestion de la conf");
}


/*------------------------------------------------------------------------------------------------*/
