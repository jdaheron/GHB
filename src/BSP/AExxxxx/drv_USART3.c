/**
 *******************************************************************************
 * @file	drv_USART3.c
 * @author	j.daheron
 * @version	1.0.0
 * @date	04 nov. 2013
 * @brief   Gestion de l'USART3.
 *******************************************************************************
 */


/* Includes ------------------------------------------------------------------*/

#include "drv_USART3.h"
#include "util_Fifo.h"


/* External variables --------------------------------------------------------*/


/* Private typedef -----------------------------------------------------------*/


/* Private defines -----------------------------------------------------------*/

#define FIFO_RX_SIZE		64
#define FIFO_TX_SIZE		1
#define TIMEOUT				100000	//10ms env.
#define UARTx				USART3


/* Private macros ------------------------------------------------------------*/


/* Private variables ---------------------------------------------------------*/

static uint8_t FifoRxBuffer[FIFO_RX_SIZE];
static Fifo_s FifoRx;
#if (FIFO_TX_SIZE > 1)
	static uint8_t FifoTxBuffer[FIFO_TX_SIZE];
	static Fifo_s FifoTx;
	static uint8_t TxEnCours = FALSE;
#endif


/* Private function prototypes -----------------------------------------------*/


/* Private functions ---------------------------------------------------------*/



/**-----------------------------------------------------------------------------
 * @brief	Initialisation.
 *
 * @param[in]	Baudrate	Debit en bauds.
 */
void USART3_Init(uint32_t Baudrate)
{
	USART_InitTypeDef xUSART_Init = {

			.USART_BaudRate				= Baudrate,
			.USART_WordLength			= USART_WordLength_8b,
			.USART_StopBits				= USART_StopBits_1,
			.USART_Parity				= USART_Parity_No,
			.USART_Mode					= USART_Mode_Rx | USART_Mode_Tx,
			.USART_HardwareFlowControl	= USART_HardwareFlowControl_None,
	};

	// Initialisation des Fifo
	Fifo_InitStruct_s FifoInit = {
			.NbElements		= FIFO_RX_SIZE,
			.pDataBuffer	= FifoRxBuffer,
			.ElementSize	= sizeof(uint8_t),
			.FifoType		= FIFO_TYPE_DATA,
	};
	Fifo_Init(&FifoRx, &FifoInit);
	#if (FIFO_TX_SIZE > 1)
		FifoInit.NbElements		= FIFO_TX_SIZE,
		FifoInit.pDataBuffer	= FifoTxBuffer;
		Fifo_Init(&FifoTx, &FifoInit);
	#endif

	// Activation horloges
	RCC_APB1PeriphResetCmd(RCC_APB1Periph_USART3, DISABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE);

	// Config peripherique
	USART_Init(UARTx, &xUSART_Init);

	// Configuration interruption
	USART_ITConfig(UARTx, USART_IT_RXNE, ENABLE);
	USART_ITConfig(UARTx, USART_IT_TXE, DISABLE);

	#ifdef configLIBRARY_KERNEL_INTERRUPT_PRIORITY
		NVIC_InitStructure.NVIC_IRQChannel = USART3_IRQn;
		NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = configLIBRARY_KERNEL_INTERRUPT_PRIORITY;
		NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
		NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
		NVIC_Init( &NVIC_InitStructure );
	#else
		NVIC_EnableIRQ(USART3_IRQn);
	#endif

	// Activation peripherique
	USART_Cmd(UARTx, ENABLE);
}


/**-----------------------------------------------------------------------------
 * @brief	Desinitialisation.
 *
 */
void USART3_DeInit()
{
	NVIC_DisableIRQ(USART3_IRQn);
	USART_Cmd(UARTx, DISABLE);								// Desactivation peripherique
	RCC_APB1PeriphResetCmd(RCC_APB1Periph_USART3, ENABLE);	// Desactivation horloge
}


/**-----------------------------------------------------------------------------
 * @brief	Lecture du buffer de reception.
 *
 * @param[out]	pBuffer	Donnees lues (NULL si vide).
 * @param[in]	Taille	Nombre de donnees a lire.
 *
 * return		NbRead	Nombre de donnees lues
 */
uint16_t USART3_Read(uint8_t* pBuffer, uint16_t Taille)
{
	uint16_t NbRead=0;

	// Remplissage du buffer
	while ((NbRead != Taille) && (Fifo_Pull(&FifoRx, &(pBuffer[NbRead])) != FIFO_STATE_EMPTY))
	{
		NbRead++;
	}
	return NbRead;
}


/**-----------------------------------------------------------------------------
 * @brief	Ecriture d'un buffer.
 *
 * @param[in]	pBuffer	Donnees a ecrire.
 * @param[in]	Taille	Nombre de donnees a ecrire.
 */
void USART3_Write(uint8_t* pBuffer, uint16_t Taille)
{
	uint16_t NbWrite = 0;

#if (FIFO_TX_SIZE > 1)

	// Remplissage du buffer
	while ((NbWrite != Taille) && (Fifo_Push(&FifoTx, pBuffer[NbWrite]) != FIFO_STATE_FULL))
	{
		NbWrite++;
	}

	// Si pas d'emission en cours, on la lance
	if (TxEnCours == FALSE)
	{
		TxEnCours = TRUE;
		USART_ITConfig(UARTx, USART_IT_TXE, ENABLE);
	}

#else

	uint32_t TO;

	while (Taille != 0){

		// Attente donnee envoyee
		TO = TIMEOUT;
		while ((USART_GetFlagStatus(UARTx, USART_FLAG_TXE) != SET) && TO) TO--;
		if (!TO) return;	// On quitte en cas d'erreur

		// Ecriture donnee
		USART_SendData(UARTx, pBuffer[NbWrite++]);

		Taille--;
	}

#endif

}


/**-----------------------------------------------------------------------------
  * @brief  USART3_IRQHandler
  */
void USART3_IRQHandler()
{
	uint8_t data;

	// Verification donnee recue
	while (USART_GetITStatus(UARTx, USART_IT_RXNE) == SET)
	{
		data = (uint8_t) USART_ReceiveData(UARTx);
		Fifo_Push(&FifoRx, &data);
	}

	#if (FIFO_TX_SIZE > 1)
		// Verification donnee a emettre
		if (USART_GetITStatus(UARTx, USART_IT_TXE) == SET)
		{
			if (Fifo_Pull(&FifoTx, &data) != FIFO_STATE_EMPTY)
			{
				USART_SendData(UARTx, data);
			}
			else
			{
				USART_ITConfig(UARTx, USART_IT_TXE, DISABLE);
				TxEnCours = FALSE;
			}
		}
	#endif

}

