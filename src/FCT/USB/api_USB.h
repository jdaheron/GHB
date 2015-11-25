/**
 ***************************************************************************************************
 * @file	api_USB.h
 * @author	j.daheron
 * @version	1.0.0
 * @date	15 juil. 2014
 * @brief   Gestion de l'interface USB.
 ***************************************************************************************************
 */


/* Define to prevent recursive inclusion **********************************************************/

#ifndef API_USB_H_
#define API_USB_H_


/* Includes ***************************************************************************************/

#include <FILES/FatFs/ff_gen_drv.h>
#include "BSP.h"


/** @defgroup api_USB api_USB
  * @brief Gestion de l'interface USB.
  * @{
  */ 

/** 
 ***************************************************************************************************
 * @defgroup Exported_Defines Exported Defines
 * @{
 */
 
 /**
 * @}
 */ 
 
 
/** 
 ***************************************************************************************************
 * @defgroup Exported_TypesDefinitions Exported TypesDefinitions
 * @{
 */

/** Definition breve de l'enumeration. */
typedef enum
{
	USB_CLASS_NONE = 0, /**< Aucune Classe.*/
	USB_CLASS_CDC,		/**< Classe de communication.*/
	USB_CLASS_MSC,		/**< Classe de stockage de masse.*/
	
} UsbClass_e;


/** Definition breve de la structure. */
typedef struct {

	UsbClass_e			CurrentClass;	/**< Classe en cours d'utilisation.*/
	Diskio_drvTypeDef*	pMscDiskDriver;	/**< Driver de disque si utilisation MSC.*/

} Usb_s;



/**
 * @}
 */ 
 
 
/** 
 ***************************************************************************************************
 * @defgroup Exported_Macros Exported Macros 
 * @{
 */
 
 /**
 * @}
 */ 
 
 
/** 
 ***************************************************************************************************
 * @defgroup Exported_Variables Exported Variables
 * @{
 */
 
 /**
 * @}
 */ 

 
/** 
 ***************************************************************************************************
 * @defgroup Exported_Functions Exported Functions  
 * @{
 */


/**
 * @brief	Initialisation de l'api USB.
 * @return 	void
 */
void
USB_Init(
		Diskio_drvTypeDef* MSC_DiskDriver
);

/**
 * @brief	Demarrage d'une classe USB.
 * @return 	void
 */
void
USB_StartClass(
		UsbClass_e Class	/**<[in] Class a demarrer.*/
);

/**
 * @brief	Arret de la classe en cours.
 * @return 	void
 */
void
USB_StopCurrentClass(
);

/**
 * @brief	Lecture de la classe en cours d'utilisation.
 * @return 	CurrentClass
 */
UsbClass_e
USB_GetCurrentClass(
);


/**
 * @brief	Lecture de l'etat de connexion du PC.
 * @return 	Etat de connexion du PC
 */
Bool_e
USB_IsPcConnected(
		void
);

/**
 * @brief	Lecture de l'etat d'ouverture du port COM.
 * @return 	Etat d'ouverture du port.
 */
Bool_e
USB_IsPcPortOpened(
		void
);


/**
 * @brief	Fonction principale de l'api USB.
 * @return 	void
 */
void
USB_Main(
);


 /**
 * @}
 */ 
 
 
 /**
 * @}
 */ 


#endif /* API_USB_H_ */
