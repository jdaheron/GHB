/**
 ***************************************************************************************************
 * @file	api_USB.c
 * @author	j.daheron
 * @version	1.0.0
 * @date	15 juil. 2014
 * @brief   Gestion de l'interface USB.
 ***************************************************************************************************
 */


/* Includes ***************************************************************************************/

#include <FILES/fct_MemoireFAT.h>
#include <FILES/FatFs/ff_gen_drv.h>
#include "api_USB.h"

#include "usbd_cdc.h"
#include "usbd_cdc_desc.h"
#include "usbd_cdc_interface.h"
#include "usbd_msc.h"
#include "usbd_msc_desc.h"
#include "usbd_msc_storage.h"

#include "util_Goto.h"




/* External Variables *****************************************************************************/


/** @addtogroup api_USB
  * @{
  */ 

 
 /** 
 ***************************************************************************************************
 * @defgroup Private_TypesDefinitions Private TypesDefinitions
 * @{
 */

/**
 * @}
 */ 

 
 /** 
 ***************************************************************************************************
 * @defgroup Private_Defines Private Defines
 * @{
 */
 
/**
 * @}
 */ 

 
 /** 
 ***************************************************************************************************
 * @defgroup Private_Macros Private Macros
 * @{
 */
  
#define USB_CONSOLE_ENABLE	1

#if USB_CONSOLE_ENABLE
	#define _USB_CONSOLE	_printf
#else
	#define _USB_CONSOLE	((void*) 0)
#endif


/**
 * @}
 */ 

  
 /** 
 ***************************************************************************************************
 * @defgroup Private_FunctionPrototypes Private FunctionPrototypes
 * @{
 */


/**
 * @}
 */ 
 
 
  /** 
 ***************************************************************************************************
 * @defgroup Private_Variables Private Variables
 * @{
 */
 
static Usb_s Usb = {
		.CurrentClass	= USB_CLASS_NONE,
		.pMscDiskDriver = NULL,
};
USBD_HandleTypeDef hUsbDevice;


/**
 * @}
 */ 
  	
	
/** 
 ***************************************************************************************************
 * @defgroup Private_Functions Private Functions
 * @{
 */ 
 
 /**
 * @}
 */ 
 

/** 
 ***************************************************************************************************
 * @todo Validation
 */
void
USB_Init(
		Diskio_drvTypeDef* MSC_DiskDriver
)
{
	Usb.pMscDiskDriver = MSC_DiskDriver;
	USB_StartClass(USB_CLASS_MSC);
}


/**
 ***************************************************************************************************
 * @todo Validation
 */
void
USB_StartClass(
		UsbClass_e Class	/**<[in] Class a demarrer.*/
)
{
	// LA classe est deja lancee
	if (Usb.CurrentClass == Class)
	{
			return;
	}

	// Arret de la classe en cours
	if (Usb.CurrentClass != USB_CLASS_NONE)
	{
		USB_StopCurrentClass();
	}

	// Lancement de la classe demandee
	switch (Class)
	{
		case USB_CLASS_CDC:

			_USB_CONSOLE("[USBD] Start CDC Class\n");

			/* Init Device Library,Add Supported Class and Start the library*/
			USBD_Init(&hUsbDevice, &FS_CDC_Desc, DEVICE_FS);
			USBD_RegisterClass(&hUsbDevice, &USBD_CDC);
			USBD_CDC_RegisterInterface(&hUsbDevice, &USBD_CDC_fops);
			USBD_Start(&hUsbDevice);

			// Autorisation de l'acces a la memoire pour l'applicatif
			MemoireFAT_AutoriserAcces();

			Usb.CurrentClass = USB_CLASS_CDC;
			break;

		case USB_CLASS_MSC:

			_USB_CONSOLE("[USBD] Start MSC Class\n");

			// Interdiction de l'acces a la memoire pour l'applicatif
			MemoireFAT_InterdireAcces();

			/* Init Device Library,Add Supported Class and Start the library*/
			USBD_Init(&hUsbDevice, &FS_MSC_Desc, DEVICE_FS);
			USBD_RegisterClass(&hUsbDevice, &USBD_MSC);
			USBD_MSC_RegisterStorage(&hUsbDevice, &USBD_MSC_fops);
			USBD_Start(&hUsbDevice);

			UsbStorage_SetDriver(Usb.pMscDiskDriver);
			UsbStorage_StartTimeout();

			Usb.CurrentClass = USB_CLASS_MSC;

			break;

		default:
			Usb.CurrentClass = USB_CLASS_NONE;
			break;
	}
}

/**
 ***************************************************************************************************
 * @todo Validation
 */
void
USB_StopCurrentClass(
)
{
	USBD_Stop(&hUsbDevice);
}


/**
 ***************************************************************************************************
 * @todo Validation
 */
UsbClass_e
USB_GetCurrentClass(
		void
)
{
	return Usb.CurrentClass;
}


/**
 ***************************************************************************************************
 * @todo Validation
 */
Bool_e
USB_IsPcConnected(
		void
)
{
	if ((hUsbDevice.dev_state == USBD_STATE_CONFIGURED))
		return TRUE;

	return FALSE;
}

/**
 ***************************************************************************************************
 * @todo Validation
 */
Bool_e
USB_IsPcPortOpened(
		void
)
{
	if (USB_IsPcConnected() == FALSE)
		return FALSE;

	if ((Usb.CurrentClass == USB_CLASS_CDC) && CDC_IsPortOpened())
		return TRUE;

	return FALSE;
}

/**
 ***************************************************************************************************
 * @todo Validation
 */
void
USB_Main(
		void
)
{
	switch (Usb.CurrentClass)
	{
		case USB_CLASS_CDC:
			break;

		case USB_CLASS_MSC:

			if (UsbStorage_IsTimedOut() == TRUE)
			{
				_USB_CONSOLE("[USBD] Detection deconnexion\n");

				USB_StopCurrentClass();

				RTC_BkpRegister_Write(0, 0);
				GOTO(0);

			}
			break;
	}
}



 /**
 * @}
 */ 

/* End Of File ************************************************************************************/
