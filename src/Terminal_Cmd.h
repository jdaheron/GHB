/**************************************************************************************************/
/*																								  */
/* Gestion des commandes du terminal															  */
/*																								  */
/**************************************************************************************************/

#ifndef TERMINAL_CMD_H_
#define TERMINAL_CMD_H_


/*--------------------------------------------------------------------------------------------------
	COMMAND LIST
----------------------------------------------------------------------------------------------------

COMMANDE	DESCRIPTION								PARAM
quit		Fermeture de la communication			-
ls			Affichage des fichiers d'un rep.		Repertoire
reboot		Redémarrage SW							-
test		Commande de test
delete		Effacement d'un fichier					Fichier
format		Formatage de la carte SD				-
rename		Renommage d'un fichier					AncienNom, NouveauNom
read		Lecture dun contenu d'un fichier		Fichier
rtc			Gestion de la RTC						get
													set, NumJour, Jour, Mois, Annee, Heure, Minute, Seconde
startup		Lecture de l'heure de demarrage			-
arroser		Lancement d'un arrosage immédiat		VolumeParPlant_ml
reservoir	Ecriture remplissage reservoir			get
													set, VolumeRestant_ml
status		Lecture de l'etat de la carte			day, Time.Jour, Time.Mois, Time.Annee,
cfg			Gestion de la conf						get, def
													get, arr
													get, ch
													get, eth
													get, ext
													set, arr, Intervalle/Volume/NbPlants


--------------------------------------------------------------------------------------------------*/


/*--------------------------------------------------------------------------------------------------
	PUBLIC INCLUDES
--------------------------------------------------------------------------------------------------*/

#include "BSP.h"


/*--------------------------------------------------------------------------------------------------
	PUBLIC DEFINE
--------------------------------------------------------------------------------------------------*/


/*--------------------------------------------------------------------------------------------------
	PUBLIC TYPEDEF
--------------------------------------------------------------------------------------------------*/


/*--------------------------------------------------------------------------------------------------
	PUBLIC FUNCTION PROTOTYPE
--------------------------------------------------------------------------------------------------*/
void	 Terminal_Cmd_Init(void);


#endif /* TERMINAL_CMD_H_ */
