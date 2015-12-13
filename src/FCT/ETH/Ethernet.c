/**************************************************************************************************/
/*																								  */
/* Gestion de l'ethernet																		  */
/*																								  */
/**************************************************************************************************/


/*--------------------------------------------------------------------------------------------------
	INCLUDES
--------------------------------------------------------------------------------------------------*/

#include "Ethernet.h"
#include "ConfIni.h"
#include "fct_DatabaseEeprom.h"

#include "stm32f2x7_eth.h"
#include "stm32f2x7_eth_bsp.h"
#include "ETH/netconf.h"
#include "ETH/SERVER_HTTP/httpd.h"
#include "ETH/SERVER_TELNET/Telnet.h"
#include "ETH/SERVER_TFTP/tftpserver.h"


/*--------------------------------------------------------------------------------------------------
	PRIVATE DEFINE
--------------------------------------------------------------------------------------------------*/

#define LogId			"ETH"


/*--------------------------------------------------------------------------------------------------
	PRIVATE TYPEDEF
--------------------------------------------------------------------------------------------------*/

typedef struct
{
	uint8_t		Cfg_IP_Adresse[4];
	uint8_t		Cfg_IP_Masque[4];
	uint8_t		Cfg_IP_Passerelle[4];
	uint8_t		Cfg_MAC_Adresse[6];
	Bool_e		Cfg_DHCP_Actif;

} Ethernet_t;


/*--------------------------------------------------------------------------------------------------
	PRIVATE DATA DECLARATION
--------------------------------------------------------------------------------------------------*/

static Ethernet_t This = {
	#ifdef DEBUG_AVEC_BL
		.Cfg_IP_Adresse		= {192, 168,   1, 200},
	#else
		.Cfg_IP_Adresse		= {192, 168,   1, 201},
	#endif
		.Cfg_IP_Masque		= {255, 255, 255,   0},
		.Cfg_IP_Passerelle	= {192, 168,   1, 254},
		.Cfg_MAC_Adresse	= {0, 0, 0, 0, 0, 1},
		.Cfg_DHCP_Actif		= FALSE,
};


/*--------------------------------------------------------------------------------------------------
	FUNCTIONS DEFINITIONS
--------------------------------------------------------------------------------------------------*/


/*------------------------------------------------------------------------------------------------*/
void Ethernet_Init(void)
{
	Ethernet_t TmpThis;


	//------------------------------------------------------
	// Lecture des donnees flash
	//------------------------------------------------------
	DatabaseEeprom_InitData(DatabaseEeprom_Ethernet, NULL, sizeof(Ethernet_t));
	if (DatabaseEeprom_Read(DatabaseEeprom_Ethernet, &TmpThis) == Status_KO)
	{
		DatabaseEeprom_Write(DatabaseEeprom_Ethernet, &This);
		memcpy(&TmpThis, &This, sizeof(Ethernet_t));
	}
	else
	{
		memcpy(&This, &TmpThis, sizeof(Ethernet_t));
	}

	//------------------------------------------------------
	// Comparaison avec fichier ini
	//------------------------------------------------------
	if (ConfIni_Get()->IsValide == TRUE)
	{
		memcpy(This.Cfg_IP_Adresse,		ConfIni_Get()->ETH_IP_Adresse, 4);
		memcpy(This.Cfg_IP_Masque,		ConfIni_Get()->ETH_IP_Masque, 4);
		memcpy(This.Cfg_IP_Passerelle,	ConfIni_Get()->ETH_IP_Passerelle, 4);
		memcpy(This.Cfg_MAC_Adresse,	ConfIni_Get()->ETH_MAC_Adresse, 6);
		This.Cfg_DHCP_Actif = ConfIni_Get()->ETH_DHCP_Actif;

		if (memcmp(&TmpThis, &This, sizeof(Ethernet_t)) != 0)
		{
			DatabaseEeprom_Write(DatabaseEeprom_Ethernet, &This);
		}
	}


	//------------------------------------------------------
	// Affichage de la configuration
	//------------------------------------------------------
	_CONSOLE( LogId, "--- CONF_ETHERNET ---\n");
	_CONSOLE( LogId, "IP_Adresse    = %d.%d.%d.%d\n",		This.Cfg_IP_Adresse[0], This.Cfg_IP_Adresse[1], This.Cfg_IP_Adresse[2], This.Cfg_IP_Adresse[3]);
	_CONSOLE( LogId, "IP_Masque     = %d.%d.%d.%d\n",		This.Cfg_IP_Masque[0], This.Cfg_IP_Masque[1], This.Cfg_IP_Masque[2], This.Cfg_IP_Masque[3]);
	_CONSOLE( LogId, "IP_Passerelle = %d.%d.%d.%d\n", 		This.Cfg_IP_Passerelle[0], This.Cfg_IP_Passerelle[1], This.Cfg_IP_Passerelle[2], This.Cfg_IP_Passerelle[3]);
	_CONSOLE( LogId, "MAC_Adresse   = %d:%d:%d:%d:%d:%d\n",	This.Cfg_MAC_Adresse[0], This.Cfg_MAC_Adresse[1], This.Cfg_MAC_Adresse[2], This.Cfg_MAC_Adresse[3], This.Cfg_MAC_Adresse[4], This.Cfg_MAC_Adresse[5]);
	_CONSOLE( LogId, "DHCP_Actif    = %d\n",					This.Cfg_DHCP_Actif		);


	//------------------------------------------------------
	// Application de la configuration
	//------------------------------------------------------

	ETH_BSP_Config();	// Configure ethernet (GPIOs, clocks, MAC, DMA)
	LwIP_Init(This.Cfg_IP_Adresse, This.Cfg_IP_Masque, This.Cfg_IP_Passerelle, This.Cfg_DHCP_Actif, This.Cfg_MAC_Adresse);		// Initilaize the LwIP stack
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
