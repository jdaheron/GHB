/**************************************************************************************************/
/*																								  */
/* Gestion de l'ethernet																		  */
/*																								  */
/**************************************************************************************************/

#ifndef ETHERNET_H_
#define ETHERNET_H_


/*--------------------------------------------------------------------------------------------------
	PUBLIC INCLUDES
--------------------------------------------------------------------------------------------------*/

#include "BSP.h"


/*--------------------------------------------------------------------------------------------------
	PUBLIC DEFINE
--------------------------------------------------------------------------------------------------*/


//#define USE_DHCP       /* enable DHCP, if disabled static address is used */

/* MAC ADDRESS: MAC_ADDR0:MAC_ADDR1:MAC_ADDR2:MAC_ADDR3:MAC_ADDR4:MAC_ADDR5 */
#define MAC_ADDR0	2
#define MAC_ADDR1	0
#define MAC_ADDR2	0
#define MAC_ADDR3	0
#define MAC_ADDR4	0
#define MAC_ADDR5	0

/*Static IP ADDRESS: IP_ADDR0.IP_ADDR1.IP_ADDR2.IP_ADDR3 */
//#define IP_ADDR0	192
//#define IP_ADDR1	168
//#define IP_ADDR2	1
//#define IP_ADDR3	200

/*NETMASK*/
//#define NETMASK_ADDR0	255
//#define NETMASK_ADDR1	255
//#define NETMASK_ADDR2	255
//#define NETMASK_ADDR3	0

/*Gateway Address*/
//#define GW_ADDR0	192
//#define GW_ADDR1	168
//#define GW_ADDR2	1
//#define GW_ADDR3	100


/*--------------------------------------------------------------------------------------------------
	PUBLIC TYPEDEF
--------------------------------------------------------------------------------------------------*/

typedef struct
{
	Bool_e 		Cfg_Restored;
	uint8_t		Cfg_IP_Adresse[4];
	uint8_t		Cfg_IP_Masque[4];
	uint8_t		Cfg_IP_Passerelle[4];
	uint8_t		Cfg_MAC_Adresse[6];
	Bool_e		Cfg_DHCP_Actif;

} Ethernet_t;


/*--------------------------------------------------------------------------------------------------
	PUBLIC FUNCTION PROTOTYPE
--------------------------------------------------------------------------------------------------*/

void 		Ethernet_Init(Bool_e UseConfIni);
void		Ethernet_Management(void);
Ethernet_t*	Ethernet_Get(void);


#endif /* ETHERNET_H_ */
