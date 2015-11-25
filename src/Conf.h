/**************************************************************************************************/
/*																								  */
/* Conf																							  */
/*																								  */
/**************************************************************************************************/

#ifndef CONF_H_
#define CONF_H_


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

typedef enum
{
	SECTION_GENERAL				 = 0,
	Conf_GEN_StartTempo_s			,

	SECTION_ETHERNET				,
	Conf_ETH_IP_Adresse				,
	Conf_ETH_IP_Masque				,
	Conf_ETH_IP_Passerelle			,
	Conf_ETH_MAC_Adresse			,
	Conf_ETH_DHCP_Actif				,

	SECTION_CHAUFFAGE				,
	Conf_CH_SeuilStart_DegC			,
	Conf_CH_SeuilStop_DegC			,
	Conf_CH_TempoApresCh_s			,

	SECTION_EXTRACT					,
	Conf_EXT_SeuilStart_DegC		,
	Conf_EXT_SeuilStop_DegC			,
	Conf_EXT_TempoApresEXT_s		,
	Conf_EXT_ActiverPendantCh		,

	SECTION_LOG						,
	Conf_LOG_Periode_s				,
	Conf_LOG_PeriodePendantAction_s	,

	SECTION_ARROSAGE				,
	Conf_ARR_Heure					,
	Conf_ARR_Intervalle_h			,
	Conf_ARR_VolumeParPlant_ml		,
	Conf_ARR_NbPlants				,
	Conf_ARR_DebitPompe_ml_par_h	,
	Conf_ARR_VolumeReservoir_ml		,
	Conf_ARR_VolumeRestant_ml		,
	Conf_ARR_TS_Precedent			,
	Conf_ARR_TS_Suivant				,

	NB_PARAM,

} Param_Liste_e;


typedef struct
{
	uint32_t	StartTempo_s;
	uint32_t	LOG_Periode_s;
	uint32_t	LOG_PeriodePendantAction_s;

} Conf_t;


/*--------------------------------------------------------------------------------------------------
	PUBLIC VARIABLE
--------------------------------------------------------------------------------------------------*/

extern const IniFile_s Conf_IniFile;


/*--------------------------------------------------------------------------------------------------
	PUBLIC FUNCTION PROTOTYPE
--------------------------------------------------------------------------------------------------*/

void 	Conf_Init(void);
void 	Conf_Write(uint32_t NumParam, void* Value);
Conf_t*	Conf_Get(void);


#endif /* CONF_H_ */

