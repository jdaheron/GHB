/**
 *******************************************************************************
 * @file	usbd_cdc_vcp.c
 * @author	j.daheron
 * @version	1.0.0
 * @date	07 mai 2014
 * @brief   Gestion du Port COM virtuel.
 *******************************************************************************
 */


/* Includes ------------------------------------------------------------------*/

#include "BSP.h"
#include "usbd_cdc.h"
#include "usbd_cdc_desc.h"
#include "util_FIFO.h"


/* Private typedef -----------------------------------------------------------*/


/* Private define ------------------------------------------------------------*/

#define APP_RX_DATA_SIZE	128
#define APP_TX_DATA_SIZE	128
#define VCP_FIFO_SIZE		128
#define VCP_TX_TIMEOUT		100000	//10ms env.


/* Private macro -------------------------------------------------------------*/



/* Private macro -------------------------------------------------------------*/

/* Private variables ---------------------------------------------------------*/

static uint8_t VCP_FifoRxBuffer[VCP_FIFO_SIZE];
static Fifo_s VCP_FifoRx;
static Bool_e UsbConnected = FALSE;
static Bool_e UsbPortOpened = FALSE;


/* Create buffer for reception and transmission           */
/* It's up to user to redefine and/or remove those define */

/* Received Data over USB are stored in this buffer       */
uint8_t UserRxBuffer[APP_RX_DATA_SIZE];

/* Send Data over USB CDC are stored in this buffer       */
uint8_t UserTxBuffer[APP_TX_DATA_SIZE];

/* USB handler declaration */
extern USBD_HandleTypeDef hUsbDevice;


/* Private function prototypes -----------------------------------------------*/
static int8_t CDC_Itf_Init     (void);
static int8_t CDC_Itf_DeInit   (void);
static int8_t CDC_Itf_Control  (uint8_t cmd, uint8_t* pbuf, uint16_t length);
static int8_t CDC_Itf_Receive  (uint8_t* pbuf, uint32_t *Len);

USBD_CDC_ItfTypeDef USBD_CDC_fops = 
{
  CDC_Itf_Init,
  CDC_Itf_DeInit,
  CDC_Itf_Control,
  CDC_Itf_Receive
};

/* Private functions ---------------------------------------------------------*/

/**
  * @brief  CDC_Itf_Init
  *         Initializes the CDC media low layer
  * @param  None
  * @retval Result of the opeartion: USBD_OK if all operations are OK else USBD_FAIL
  */
static int8_t CDC_Itf_Init(void)
{
	FIFO_Init(&VCP_FifoRx, VCP_FifoRxBuffer, VCP_FIFO_SIZE, sizeof(uint8_t));
	FIFO_Clear(&VCP_FifoRx);

	USBD_CDC_SetTxBuffer(&hUsbDevice, UserTxBuffer, 0);
	USBD_CDC_SetRxBuffer(&hUsbDevice, UserRxBuffer);

	UsbConnected = TRUE;

	return (USBD_OK);
}

/**
  * @brief  CDC_Itf_DeInit
  *         DeInitializes the CDC media low layer
  * @param  None
  * @retval Result of the opeartion: USBD_OK if all operations are OK else USBD_FAIL
  */
static int8_t CDC_Itf_DeInit(void)
{
	UsbConnected = FALSE;
	UsbPortOpened = FALSE;

	return (USBD_OK);
}

/**
  * @brief  CDC_Itf_Control
  *         Manage the CDC class requests
  * @param  Cmd: Command code            
  * @param  Buf: Buffer containing command data (request parameters)
  * @param  Len: Number of data to be sent (in bytes)
  * @retval Result of the opeartion: USBD_OK if all operations are OK else USBD_FAIL
  */
static int8_t CDC_Itf_Control (uint8_t cmd, uint8_t* pbuf, uint16_t length)
{ 
	switch (cmd)
	{
		case CDC_SEND_ENCAPSULATED_COMMAND:
			/* Add your code here */
			break;

		case CDC_GET_ENCAPSULATED_RESPONSE:
			/* Add your code here */
			break;

		case CDC_SET_COMM_FEATURE:
			/* Add your code here */
			break;

		case CDC_GET_COMM_FEATURE:
			/* Add your code here */
			break;

		case CDC_CLEAR_COMM_FEATURE:
			/* Add your code here */
			break;

		case CDC_SET_LINE_CODING:
			/* Add your code here */
			UsbPortOpened = TRUE;
			break;

		case CDC_GET_LINE_CODING:
			/* Add your code here */
			__NOP();
			break;

		case CDC_SET_CONTROL_LINE_STATE:
			/* Add your code here */
			break;

		case CDC_SEND_BREAK:
			/* Add your code here */
			break;

		default:
			break;
	}

	return (USBD_OK);
}



/**
  * @brief  CDC_Itf_DataRx
  *         Data received over USB OUT endpoint are sent over CDC interface
  *         through this function.
  * @param  Buf: Buffer of data to be transmitted
  * @param  Len: Number of data received (in bytes)
  * @retval Result of the opeartion: USBD_OK if all operations are OK else USBD_FAIL
  */
static int8_t CDC_Itf_Receive (uint8_t* pbuf, uint32_t *Len)
{
	uint32_t NbRead=*Len;

	// Mise en FIFO de la trame recue
	while (NbRead > 0)
	{
		FIFO_Push(&VCP_FifoRx, (uint8_t) *pbuf);

		pbuf++;
		NbRead--;
	}

	// Preparation reception suivante
	USBD_CDC_ReceivePacket(&hUsbDevice);

	return USBD_OK;
}


/**
  * @brief  Rx Transfer completed callback
  * @param  huart: UART handle
  * @retval None
  */
uint32_t CDC_Receive(uint8_t* pBuffer, uint32_t Size)
{
	uint32_t NbRead=0;

	if (UsbPortOpened == FALSE)
		return 0;

	// Depilement de la FIFO
	while ((NbRead != Size) && (FIFO_Pull(&VCP_FifoRx, &(pBuffer[NbRead])) != FIFO_STATE_EMPTY)){

		NbRead++;
	}

	return NbRead;
}


/**
  * @brief  Rx Transfer completed callback
  * @param  huart: UART handle
  * @retval None
  */
void CDC_Transmit(uint8_t *pTxData, uint32_t Size)
{
	uint32_t TO;

	if (UsbPortOpened == FALSE)
		return;

	if (Size > APP_TX_DATA_SIZE)
		Size = APP_TX_DATA_SIZE;

	memcpy(UserTxBuffer, pTxData, Size);

	USBD_CDC_SetTxBuffer(&hUsbDevice, UserTxBuffer, Size);
	USBD_CDC_TransmitPacket(&hUsbDevice);

	USBD_CDC_HandleTypeDef   *hcdc = hUsbDevice.pClassData;

	TO = VCP_TX_TIMEOUT;
	while (hcdc->TxState && TO) TO--;
}

/**
  * @brief  Lecture de l'etat de connexion de l'usb.
  * @retval TRUE/FALSE
  */
Bool_e CDC_IsUsbConnected()
{
	return UsbConnected;
}


/**
  * @brief  Lecture de l'etat d'ouverture du port.
  * @retval TRUE/FALSE
  */
Bool_e CDC_IsPortOpened()
{
	return UsbPortOpened;
}
