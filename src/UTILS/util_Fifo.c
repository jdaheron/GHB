/**
 *******************************************************************************
 * @file	util_Fifo.h
 * @author	j.daheron
 * @version	2.0.0
 * @date	27 sept. 2014
 * @brief   Gestion de Fifo.
 *******************************************************************************
 */


/* Includes ------------------------------------------------------------------*/

#include "util_Fifo.h"


/* External variables --------------------------------------------------------*/


/* Private typedef -----------------------------------------------------------*/


/* Private defines -----------------------------------------------------------*/


/* Private macros ------------------------------------------------------------*/


/* Private variables ---------------------------------------------------------*/


/* Private function prototypes -----------------------------------------------*/


/* Private functions ---------------------------------------------------------*/


/**-----------------------------------------------------------------------------
 * @brief	Initialisation d'une Fifo.
 *
 * @param[in]	pFifo		Pointeur vers la fifo
 * @param[in]	pBuffer		Pointeur vers le buffer
 * @param[in]	FifoSize	Nombre d'elements de la fifo
 * @param[in]	DataSize	Taille d'un element, en octet
 *
 */
void Fifo_Init(Fifo_s* pFifo, Fifo_InitStruct_s* pInitStruct)
{
	// Verification des parametres
	if (pFifo == NULL) return;
	if (pInitStruct == NULL) return;
	if (pInitStruct->pDataBuffer == NULL) return;
	if (pInitStruct->FifoType >= FIFO_TYPE_NB) return;

	// Copie de la configuration
	pFifo->NbElements	= pInitStruct->NbElements;
	pFifo->ElementSize	= pInitStruct->ElementSize;
	pFifo->pData		= pInitStruct->pDataBuffer;
	pFifo->FifoType		= pInitStruct->FifoType;

	if (pFifo->FifoType == FIFO_TYPE_STRING)
		pFifo->ElementSize = 1;

	Fifo_Clear(pFifo);
}

/*
//TODO JD : Creation dynamique ?
void FIFO_Create(Fifo_t* pFifo, uint32_t FifoSize, uint32_t DataSize)
{
}
void FIFO_Delete(Fifo_t* pFifo)
{
}
*/


/**-----------------------------------------------------------------------------
 * @brief	Effacement d'une fifo.
 *
 * @param[in]	pFifo	Pointeur vers la fifo
 */
void Fifo_Clear(Fifo_s* pFifo)
{
	// Verification des parametres
	if (pFifo == NULL)
		return;

	memset(pFifo->pData, 0, pFifo->ElementSize * pFifo->NbElements);

	pFifo->IndexIn	= 0;
	pFifo->IndexOut	= 0;
	pFifo->State	= FIFO_STATE_EMPTY;
}


/**-----------------------------------------------------------------------------
 * @brief	Ajout d'une donnee en Fifo.
 *
 * @param[in]	pFifo	Pointeur vers la fifo
 * @param[in]	...		Donnee a ajouter
 *
 * @return		FifoState_e
 */
#if 0 // Version 1: Passage de la valeur
FifoState_e Fifo_Push(Fifo_s* pFifo, ...)
{
	uint32_t i;

	// Récupération du pointeur vers la donnee a empiler
	register int* varg = (int*) (&pFifo+1);
	uint8_t* pData = (uint8_t*) (varg);

	// Verification place disponible
	if (pFifo->State == FIFO_FULL)
		return FIFO_FULL;

	// Ajout de la donnee en fifo
	for (i=0; i<pFifo->DataSize; i++)
	{
		pFifo->pData[pFifo->IndexIn++] = (uint8_t) *((uint8_t*)pData + i);
	}

	// Incrémentation de l'index d'entree
	if (pFifo->IndexIn >= pFifo->FifoSize * pFifo->DataSize)
		pFifo->IndexIn = 0;

	// MAJ Statut
	if (pFifo->IndexIn == pFifo->IndexOut)
	{
		pFifo->State = FIFO_FULL;
		return FIFO_LAST_DATA;
	}
	else
		pFifo->State = FIFO_DATA_DISPO;

	return pFifo->State;
}
#else	// Version 2: Passage du pointeur vers la valeur
FifoState_e Fifo_Push(Fifo_s* pFifo, void* pData)
{
	uint32_t i;
	uint8_t* pDataByte = (uint8_t*) pData;

	// Verification des parametres
	if ((pFifo == NULL) || (pData == NULL))
		return FIFO_STATE_ERROR;

	// Verification place disponible
	if (pFifo->State == FIFO_STATE_FULL)
		return FIFO_STATE_FULL;

	// Ajout de la donnee en fifo
	for (i=0; i<pFifo->ElementSize; i++)
	{
		pFifo->pData[pFifo->IndexIn] = *pDataByte;
		pDataByte++;
		pFifo->IndexIn++;
	}

	// Incrémentation de l'index d'entree
	if (pFifo->IndexIn >= pFifo->NbElements * pFifo->ElementSize)
		pFifo->IndexIn = 0;

	// MAJ Statut
	if (pFifo->IndexIn == pFifo->IndexOut)
	{
		pFifo->State = FIFO_STATE_FULL;
		return FIFO_STATE_LAST_DATA;
	}
	else
		pFifo->State = FIFO_STATE_DATA_DISPO;

	return pFifo->State;
}
#endif

/**-----------------------------------------------------------------------------
 * @brief	Retrait d'une donnee d'une Fifo.
 *
 * @param[in]	pFifo	Pointeur vers la fifo
 * @param[in]	pData	Pointeur vers la Donnee retiree
 *
 * @return	FifoState_e
 */
FifoState_e Fifo_Pull(Fifo_s* pFifo, void* pData)
{
	// Verification des parametres
	if ((pFifo == NULL) || (pData == NULL))
		return FIFO_STATE_ERROR;

	// Verification donnee disponible
	if (pFifo->State == FIFO_STATE_EMPTY)
		return FIFO_STATE_EMPTY;

	// Si demande, lecture de la donnee en fifo.
	if( pData != NULL)
		Fifo_Read(pFifo, pData);

	// Incrémentation de l'index de sortie
	pFifo->IndexOut += pFifo->ElementSize;
	if (pFifo->IndexOut >= pFifo->NbElements * pFifo->ElementSize)
		pFifo->IndexOut = 0;

	// MAJ Statut
	if (pFifo->IndexIn == pFifo->IndexOut)
	{
		pFifo->State = FIFO_STATE_EMPTY;
		return FIFO_STATE_LAST_DATA;
	}
	else
		pFifo->State = FIFO_STATE_DATA_DISPO;

	return pFifo->State;
}

/**-----------------------------------------------------------------------------
 * @brief	Lecture de la premiere donnee d'une Fifo.
 *
 * @param[in]	pFifo	Pointeur vers la fifo
 * @param[in]	pData	Pointeur vers la Donnee lue
 *
 * @return	FifoState_e
 */
FifoState_e Fifo_Read(Fifo_s* pFifo, void* pData)
{
	uint8_t* pDataByte =(uint8_t*) pData;

	// Verification des parametres
	if ((pFifo == NULL) || (pData == NULL))
		return FIFO_STATE_ERROR;

	// Verification donnee disponible
	if (pFifo->State == FIFO_STATE_EMPTY)
		return FIFO_STATE_EMPTY;

	// Lecture de la donnee en fifo
	for (uint32_t i = pFifo->IndexOut; i < (pFifo->IndexOut + pFifo->ElementSize); i++)
	{
		 *pDataByte = pFifo->pData[i];
		 pDataByte++;
	}

	return pFifo->State;
}

 /**-----------------------------------------------------------------------------
 * @brief	Ajout d'une chaine dec caracteres en Fifo.
 *
 * @param[in]	pFifo	Pointeur vers la fifo
 * @param[in]	...		Donnee a ajouter
 *
 * @return		FifoState_e
 */
 FifoState_e Fifo_PushString(Fifo_s* pFifo, const char* str)
 {
	uint32_t i=0;

	// Verification place disponible
	if (pFifo->IndexIn + strlen(str) + 1 >= pFifo->NbElements * pFifo->ElementSize)
	{
		// Si pas assez de place dispo, effacement fin buffer et positionnement au debut
		while (pFifo->IndexIn < pFifo->NbElements * pFifo->ElementSize)
		{
			pFifo->pData[pFifo->IndexIn] = 0;
			pFifo->IndexIn++;
		}
		pFifo->IndexIn = 0;
	}

	// Copie de la chaine en fifo
	memcpy(&pFifo->pData[pFifo->IndexIn], str, strlen(str));
	pFifo->IndexIn += strlen(str);

	// Positionnement fin de chaine et recherche chaine suivante
	if (pFifo->pData[pFifo->IndexIn] != 0)
	{
		i = pFifo->IndexIn;
		while (pFifo->pData[i] != 0)
		{
			pFifo->pData[i] = 0;
			i++;
		}
		return FIFO_STATE_STRING_LOST;
	}
	// Incrementation de l'index d'entree (pour lafin de chaine)
	pFifo->IndexIn++;
	if (pFifo->IndexIn >= pFifo->NbElements * pFifo->ElementSize)
		pFifo->IndexIn = 0;

	return FIFO_STATE_DATA_DISPO;
 }

 /**-----------------------------------------------------------------------------
 * @brief	Retrait d'une chaine dec caracteres d'une Fifo.
 *
 * @param[in]	pFifo	Pointeur vers la fifo
 * @param[in]	pData	Pointeur vers la Donnee retiree
 *
 * @return	FifoState_e
 */
 const char* Fifo_PullString(Fifo_s* pFifo)
 {

 }



/**-----------------------------------------------------------------------------
 * @brief	Verification Fifo vide.
 *
 * @param[in]	pFifo	Pointeur vers la fifo
 *
 * @return	TRUE/FALSE
 */
Bool_e Fifo_IsEmpty	(Fifo_s* pFifo)
{
	if (pFifo->State == FIFO_STATE_EMPTY)
		return TRUE;
	else
		return FALSE;
}

/**-----------------------------------------------------------------------------
 * @brief	Verification Fifo pleine.
 *
 * @param[in]	pFifo	Pointeur vers la fifo
 *
 * @return	TRUE/FALSE
 */
Bool_e Fifo_IsFull (Fifo_s* pFifo)
{
	if (pFifo->State == FIFO_STATE_FULL)
		return TRUE;
	else
		return FALSE;
}

