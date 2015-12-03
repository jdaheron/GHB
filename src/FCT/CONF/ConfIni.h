/**************************************************************************************************/
/*																								  */
/* Conf Ini																						  */
/*																								  */
/**************************************************************************************************/

#ifndef CONF_INI_H_
#define CONF_INI_H_


/*--------------------------------------------------------------------------------------------------
	PUBLIC INCLUDES
--------------------------------------------------------------------------------------------------*/

#include "BSP.h"
#include <FILES/Parametres.h>


/*--------------------------------------------------------------------------------------------------
	PUBLIC DEFINE
--------------------------------------------------------------------------------------------------*/


/*--------------------------------------------------------------------------------------------------
	PUBLIC TYPEDEF
--------------------------------------------------------------------------------------------------*/

typedef struct
{
	Bool_e		IsValide;

	uint32_t	GEN_StartTempo_s;
    uint32_t	GEN_LogPeriode_s;
	uint32_t	GEN_LogPeriodePendantAction_s;

	uint8_t		ETH_IP_Adresse[4];
	uint8_t		ETH_IP_Masque[4];
	uint8_t		ETH_IP_Passerelle[4];
	uint8_t		ETH_MAC_Adresse[6];
	Bool_e		ETH_DHCP_Actif;

	uint32_t	CH_SeuilStart_DegC;
	uint32_t	CH_SeuilStop_DegC;
	uint32_t	CH_TempoApresCh_s;

    uint32_t	EXT_SeuilStart_DegC;
	uint32_t	EXT_SeuilStop_DegC;
	uint32_t	EXT_TempoApresEXT_s;
	uint32_t	EXT_ActiverPendantCh;

	char		ARR_Heure[8];
	uint32_t	ARR_Intervalle_h;
	uint32_t	ARR_VolumeParPlant_ml;
	uint32_t	ARR_NbPlants;
	uint32_t	ARR_DebitPompe_ml_par_h;
	uint32_t	ARR_VolumeReservoir_ml;

} ConfIni_t;


/*--------------------------------------------------------------------------------------------------
	PUBLIC VARIABLE
--------------------------------------------------------------------------------------------------*/

extern const IniFile_s Conf_IniFile;


/*--------------------------------------------------------------------------------------------------
	PUBLIC FUNCTION PROTOTYPE
--------------------------------------------------------------------------------------------------*/

void 		ConfIni_Init(void);
ConfIni_t*	ConfIni_Get(void);


#endif /* CONF_INI_H_ */

