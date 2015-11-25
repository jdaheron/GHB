/**************************************************************************************************/
/*																								  */
/* Gestion de l'arrosage																		  */
/*																								  */
/**************************************************************************************************/


/*--------------------------------------------------------------------------------------------------
	INCLUDES
--------------------------------------------------------------------------------------------------*/

#include "Ventilation.h"
#include "Conf.h"


/*--------------------------------------------------------------------------------------------------
	PRIVATE DEFINE
--------------------------------------------------------------------------------------------------*/


/*--------------------------------------------------------------------------------------------------
	PRIVATE TYPEDEF
--------------------------------------------------------------------------------------------------*/


/*--------------------------------------------------------------------------------------------------
	PRIVATE DATA DECLARATION
--------------------------------------------------------------------------------------------------*/

static Ventilation_t This;


/*--------------------------------------------------------------------------------------------------
	FUNCTIONS DEFINITIONS
--------------------------------------------------------------------------------------------------*/


/*------------------------------------------------------------------------------------------------*/
void Ventilation_Init(void)
{
	//------------------------------------------------------
	// Lecture de la configuration
	//------------------------------------------------------
	if (Parametres_Init(&Conf_IniFile) != Status_OK)
	{
		_printf("Conf forced to default value\n");
	}
	Parametres_OpenReadFile(&Conf_IniFile);

	Parametres_Read(&Conf_IniFile,	Conf_EXT_SeuilStart_DegC	,	&This.SeuilStop_DegC			);
	Parametres_Read(&Conf_IniFile,	Conf_EXT_SeuilStop_DegC		,	&This.SeuilStart_DegC			);
	Parametres_Read(&Conf_IniFile,	Conf_EXT_TempoApresEXT_s	,	&This.TempoApresEXT_s			);
	Parametres_Read(&Conf_IniFile,	Conf_EXT_ActiverPendantCh	,	&This.ActiverPendantChauffage	);

	Parametres_CloseFile(&Conf_IniFile);


	//------------------------------------------------------
	// Affichage de la configuration
	//------------------------------------------------------
	_printf("--- CONF VENTILATION ---\n");
	_printf("SeuilStop_DegC    = %d\n",	This.SeuilStop_DegC				);
	_printf("SeuilStart_DegC   = %d\n",	This.SeuilStart_DegC			);
	_printf("TempoApresEXT_s   = %d\n",	This.TempoApresEXT_s			);
	_printf("ActiverPendantCh. = %d\n",	This.ActiverPendantChauffage	);
}


/*------------------------------------------------------------------------------------------------*/
void Ventilation_Management(void)
{

}


/*------------------------------------------------------------------------------------------------*/
Ventilation_t* Ventilation_Get(void)
{
	return &This;
}


/*------------------------------------------------------------------------------------------------*/
