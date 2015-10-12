/**
 *******************************************************************************
 * @file	util_Fifo.h
 * @author	j.daheron
 * @version	2.0.0
 * @date	27 sept. 2014
 * @brief   Gestion de Fifo.
 *******************************************************************************
 */


/* Define to prevent recursive inclusion -------------------------------------*/

#ifndef UTIL_FIFO_H_
#define UTIL_FIFO_H_


/* Includes ------------------------------------------------------------------*/

#include "BSP.h"



/* Exported types ------------------------------------------------------------*/

/** Definition des etats de la FIFO. */
typedef enum{

	FIFO_STATE_ERROR = 0,		/// Fifo erreur rencontree
	FIFO_STATE_EMPTY,			/// Fifo vide
	FIFO_STATE_LAST_DATA,		/// On vient de lire la derniere donnee disponible
	FIFO_STATE_DATA_DISPO,		/// Des donnees sont disponibles
	FIFO_STATE_FULL,			/// Fifo pleine
	FIFO_STATE_STRING_LOST,		/// Une chaine de caractere a ete ecrasee

} FifoState_e;

/** Definition des types de FIFO. */
typedef enum{

	FIFO_TYPE_DATA = 0,		/// Fifo de donnees
	FIFO_TYPE_STRING,		/// Fifo de chaines de caracteres

	FIFO_TYPE_NB,

} FifoType_e;

/** Definition de l'objet FIFO. */
typedef struct
{
	volatile uint32_t	IndexIn;
	volatile uint32_t	IndexOut;
	uint32_t			NbElements;
	uint32_t			ElementSize;
	volatile uint8_t*	pData;
	FifoType_e			FifoType;
	FifoState_e			State;

} Fifo_s;

/** Definition de la structure de configuration d'une FIFO. */
typedef struct
{
	uint32_t			NbElements;
	uint32_t			ElementSize;
	volatile uint8_t*	pDataBuffer;
	FifoType_e			FifoType;

} Fifo_InitStruct_s;



/* Exported constants --------------------------------------------------------*/


/* Exported macro ------------------------------------------------------------*/


/* Exported functions ------------------------------------------------------- */

void 		Fifo_Init(Fifo_s* pFifo, Fifo_InitStruct_s* pInitStruct);
void 		Fifo_Clear(Fifo_s* pFifo);

FifoState_e Fifo_Push(Fifo_s* pFifo, void* pData);
FifoState_e Fifo_Pull(Fifo_s* pFifo, void* pData);
FifoState_e Fifo_Read(Fifo_s* pFifo, void* pData);

FifoState_e Fifo_PushString(Fifo_s* pFifo, const char* str);
const char* Fifo_PullString(Fifo_s* pFifo);

Bool_e		Fifo_IsEmpty	(Fifo_s* pFifo);
Bool_e		Fifo_IsFull		(Fifo_s* pFifo);

#endif /* UTIL_FIFO_H_ */




