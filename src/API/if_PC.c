/**
 *******************************************************************************
 * @file	if_EServe.h
 * @author	j.daheron
 * @version	1.0.0
 * @date	18 nov. 2013
 * @brief   Gestion de l'interface avec e-serve.
 *******************************************************************************
 */


/* Includes ------------------------------------------------------------------*/

#include "if_PC.h"
#include "util_UART.h"
#include "api_USB.h"
#include "usbd_cdc_interface.h"


/* External variables --------------------------------------------------------*/


/* Private typedef -----------------------------------------------------------*/

#define PC_TIMEOUT_RX_ms		20
#define PC_BUFFER_SIZE			255


/* Private defines -----------------------------------------------------------*/



/* Private macros ------------------------------------------------------------*/


/* Private variables ---------------------------------------------------------*/

static UartHandle_s hUart_PC;
static uint8_t buff_PC[PC_BUFFER_SIZE];


/* Private function prototypes -----------------------------------------------*/



/* Private functions ---------------------------------------------------------*/


/**-----------------------------------------------------------------------------
 * @brief	Validation des fonctions.
 *
 */

/**
 ***************************************************************************************************
 * Validation OK - JD 28/07/14
 */
void PC_Init(
)
{
	UartConfig_s UartConfig = {
		.pInitFct		= NULL,
		.Baudrate		= 0,

		.pRxFct			= (pFunction) HAL_Console_Read,
		.pRxBuffer		=  (uint8_t *) buff_PC,
		.RxBufferSize	= PC_BUFFER_SIZE,
		.RxSOFcar		= 0,
		.RxEOFstr		= "\n",
		.RxEOFsize		= 1,
		.RxTimeout_ms	= PC_TIMEOUT_RX_ms,

		.pTxFct			= (pFunction) HAL_Console_Write,
	};

	Uart_Init(&hUart_PC, &UartConfig);
}

/**
 ***************************************************************************************************
 * Validation OK - JD 28/07/14
 */
Bool_e
PC_Read(
		uint8_t* pBuffer,		/**<[in] Pointeur vers buffer dans lequel copier la trame recue. */
		uint16_t* pTailleLue	/**<[in] Pointeur vers taille du buffer lu. */
)
{
	uint8_t* pTrameRecue;
	uint16_t TailleRetournee;

	// Gestion des receptions
	Uart_GererReception(&hUart_PC);

	// Verification trame recue
	if (Uart_IsTrameRecue(&hUart_PC))
	{
		// Lecture pointeur et taille de la trame recue
		Uart_GetTrameRecue(&hUart_PC, &pTrameRecue, &TailleRetournee);

		// Verification taille
		if (TailleRetournee > PC_BUFFER_SIZE)
			TailleRetournee = PC_BUFFER_SIZE;

		// Copie vers le pointeur de sortie
		if (pBuffer != NULL)
			memcpy(pBuffer, pTrameRecue, TailleRetournee);

		if (pTailleLue != NULL)
			*pTailleLue = TailleRetournee;

		// Preparation nouvelle reception
		Uart_PreparerReception(&hUart_PC);

		return TRUE;
	}

	return FALSE;
}

/**
 ***************************************************************************************************
 * Validation OK - JD 28/07/14
 */
void PC_Write(
		uint8_t* pBuffer,		/**<[in] Pointeur vers le buffer a emettre. */
		uint16_t Taille			/**<[in] Taille du buffer a emettre. */
)
{
	Uart_EnvoyerTrame(&hUart_PC, pBuffer, Taille);
}

/**
 ***************************************************************************************************
 * Validation OK - JD 28/07/14
 */
void PC_WriteString(
		char* pBuffer		/**<[in] Chaine a emettre. */
)
{
	Uart_EnvoyerTrame(&hUart_PC, pBuffer, strlen(pBuffer));
}

