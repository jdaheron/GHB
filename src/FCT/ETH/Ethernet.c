/**************************************************************************************************/
/*																								  */
/* Gestion de l'ethernet																		  */
/*																								  */
/**************************************************************************************************/


/*--------------------------------------------------------------------------------------------------
	INCLUDES
--------------------------------------------------------------------------------------------------*/

#include "Ethernet.h"
#include "Conf.h"

#include "stm32f2x7_eth.h"
#include "stm32f2x7_eth_bsp.h"
#include "ETH/netconf.h"
#include "ETH/SERVER_HTTP/httpd.h"
#include "ETH/SERVER_TELNET/Telnet.h"
#include "ETH/SERVER_TFTP/tftpserver.h"


/*--------------------------------------------------------------------------------------------------
	PRIVATE DEFINE
--------------------------------------------------------------------------------------------------*/


/*--------------------------------------------------------------------------------------------------
	PRIVATE TYPEDEF
--------------------------------------------------------------------------------------------------*/

typedef struct
{
	uint8_t		IP_Adresse[4];
	uint8_t		IP_Masque[4];
	uint8_t		IP_Passerelle[4];
	Bool_e		DHCP_Actif;
	uint8_t		MAC_Adresse[6];

} Ethernet_t;


/*--------------------------------------------------------------------------------------------------
	PRIVATE DATA DECLARATION
--------------------------------------------------------------------------------------------------*/

static Ethernet_t This;


/*--------------------------------------------------------------------------------------------------
	FUNCTIONS DEFINITIONS
--------------------------------------------------------------------------------------------------*/


/*------------------------------------------------------------------------------------------------*/
void Ethernet_Init(void)
{
	char str_IP_Adresse[INIFILE_DATA_STR_MAX_SIZE];
	char str_IP_Masque[INIFILE_DATA_STR_MAX_SIZE];
	char str_IP_Passerelle[INIFILE_DATA_STR_MAX_SIZE];
	char str_MAC_Adresse[INIFILE_DATA_STR_MAX_SIZE];


	//------------------------------------------------------
	// Lecture de la configuration
	//------------------------------------------------------
	if (Parametres_Init(&Conf_IniFile) != Status_OK)
	{
		_printf("Conf forced to default value\n");
	}
	Parametres_OpenReadFile(&Conf_IniFile);

	Parametres_Read(&Conf_IniFile,	Conf_ETH_IP_Adresse			,	str_IP_Adresse		);
	Parametres_Read(&Conf_IniFile,	Conf_ETH_IP_Masque			,	str_IP_Masque		);
	Parametres_Read(&Conf_IniFile,	Conf_ETH_IP_Passerelle		,	str_IP_Passerelle	);
	Parametres_Read(&Conf_IniFile,	Conf_ETH_MAC_Adresse		,	str_MAC_Adresse		);
	Parametres_Read(&Conf_IniFile,	Conf_ETH_DHCP_Actif			,	&This.DHCP_Actif	);

	Parametres_CloseFile(&Conf_IniFile);


	//------------------------------------------------------
	// Affichage de la configuration
	//------------------------------------------------------
	_printf("--- CONF_ETHERNET ---\n");
	_printf("IP_Adresse    = %s\n",	str_IP_Adresse		);
	_printf("IP_Masque     = %s\n",	str_IP_Masque		);
	_printf("IP_Passerelle = %s\n",	str_IP_Passerelle	);
	_printf("MAC_Adresse   = %s\n",	str_MAC_Adresse		);
	_printf("DHCP_Actif    = %d\n",	This.DHCP_Actif		);


	//------------------------------------------------------
	// Décodage/Interprétation
	//------------------------------------------------------
	char* Val[16];

	Conv_ParseString(str_IP_Adresse, '.', Val);
	for (int i = 0; i < 4; i++)
	{
		This.IP_Adresse[i] = strtoul(Val[i], NULL ,10);
	}
	Conv_ParseString(str_IP_Masque, '.', Val);
	for (int i = 0; i < 4; i++)
	{
		This.IP_Masque[i] = strtoul(Val[i], NULL ,10);
	}
	Conv_ParseString(str_IP_Passerelle, '.', Val);
	for (int i = 0; i < 4; i++)
	{
		This.IP_Passerelle[i] = strtoul(Val[i], NULL ,10);
	}
	Conv_ParseString(str_MAC_Adresse, ':', Val);
	for (int i = 0; i < 6; i++)
	{
		This.MAC_Adresse[i] = strtoul(Val[i], NULL ,10);
	}

	//------------------------------------------------------
	// Application de la configuration
	//------------------------------------------------------

	ETH_BSP_Config();	// Configure ethernet (GPIOs, clocks, MAC, DMA)
	LwIP_Init(This.IP_Adresse, This.IP_Masque, This.IP_Passerelle, This.DHCP_Actif, This.MAC_Adresse);		// Initilaize the LwIP stack
	tftpd_init();		// TFTP server Init
	//httpd_init();		// Http webserver Init
	Telnet_Init(TELNET_PROMPT, TELNET_WELCOM);
}


/*------------------------------------------------------------------------------------------------*/
void Ethernet_Management(void)
{
	/* check if any packet received */
	if (ETH_CheckFrameReceived())
	{
		/* process received ethernet packet */
		LwIP_Pkt_Handle();
	}

	/* handle periodic timers for LwIP */
	LwIP_Periodic_Handle(TSW_GetTimestamp_ms());
}


/*------------------------------------------------------------------------------------------------*/
