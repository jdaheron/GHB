/**
 *******************************************************************************
 * @file	if_PC.h
 * @author	j.daheron
 * @version	1.0.0
 * @date	18 nov. 2013
 * @brief   Gestion de l'interface PC.
 *******************************************************************************
 */

/* Define to prevent recursive inclusion -------------------------------------*/

#ifndef IF_PC_H_
#define IF_PC_H_


/* Includes ------------------------------------------------------------------*/

#include "BSP.h"


/* Exported constants --------------------------------------------------------*/


/* Exported types ------------------------------------------------------------*/


/* Exported macro ------------------------------------------------------------*/


/* Exported functions ------------------------------------------------------- */

/**
 * -------------------------------------------------------------------------------------------------
 * @brief	Init de l'interface PC.
 * @return 	void
 */
void
PC_Init(
		void
);

/**
 * -------------------------------------------------------------------------------------------------
 * @brief	Lecture d'une trame recue du PC.
 * @return 	TRUE/FALSE
 */
Bool_e
PC_Read(
		uint8_t* pBuffer,		/**<[in] Pointeur vers buffer dans lequel copier la trame recue. */
		uint16_t* pTailleLue	/**<[in] Pointeur vers taille du buffer lu. */
);

/**
 * -------------------------------------------------------------------------------------------------
 * @brief	Envoi d'une trame au PC.
 * @return 	void
 */
void
PC_Write(
		uint8_t* pBuffer,		/**<[in] Pointeur vers le buffer a emettre. */
		uint16_t Taille			/**<[in] Taille du buffer a emettre. */
);

/**
 * -------------------------------------------------------------------------------------------------
 * @brief	Envoi d'une chaine au PC.
 * @return 	void
 */
void
PC_WriteString(
		char* pBuffer		/**<[in] Chaine a emettre. */
);

#endif /* IF_PC_H_ */




