/**
 ***************************************************************************************************
 * @file	fct_IniFile.h
 * @author	j.daheron
 * @version	1.0.0
 * @date	28 août 2014
 * @brief   Gestion de fichier INI.
 ***************************************************************************************************
 */


/* Define to prevent recursive inclusion **********************************************************/

#ifndef FCT_INIFILE_H_
#define FCT_INIFILE_H_


/* Includes ***************************************************************************************/

#include "BSP.h"
#include "fct_MemoireFAT.h"


/** @defgroup fct_IniFile fct_IniFile
  * @brief Gestion de fichier INI.
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

/** Definition des types de cle. */
typedef enum
{
	IniFile_StatusOK = 0,			/**< Status OK.*/
	IniFile_StatusKO,				/**< Status KO.*/
	IniFile_StatusDefaultValue,		/**< Valeurs par defaut restaurees.*/
	IniFile_StatusFileCreated,		/**< Fichier cree (avec les valeurs par defaut).*/
	IniFile_StatusAccessLocked,		/**< Access a la lecture de fichier bloque (une autre lecture est en cours).*/

} IniFile_Status_e;

/** Definition des types de cle. */
typedef enum
{
	KeyType_SectionName = 0,	/**< Type nom de section.*/
	KeyType_Int,				/**< Type entier.*/
	KeyType_Str,				/**< Type chaine.*/
//	KeyType_Float,				/**< Type flottant.*/

} KeyType_e;

/** Definition d'une cle. */
typedef struct {

	const KeyType_e	KeyType;		/**< Type de cle.*/
	const char*		SectionName;	/**< Nom de la section.*/
	const char*		KeyName;		/**< Nom de la cle.*/
	const void*		Value;			/**< Valeur par defaut.*/
	const void*		MinValue;		/**< Valeur min.*/
	const void*		MaxValue;		/**< Valeur max.*/

} Key_s;

/** Contenu d'un fichier Ini. */
typedef struct {

	const char*		FileName;		/**< Nom du fichier.*/
	const Key_s*	KeyTable;		/**< Table de donnees du fichier.*/
	const uint32_t	NbKey;			/**< Nombre de cles.*/

} IniFile_s;

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
 * @brief	Initilisation d'un fichier INI (Le fichier doit etre ferme).
 * @return 	Status.
 */
IniFile_Status_e
IniFile_Init(
		const IniFile_s*	IniFile		/**<[in] Descripteur du contenu du fichier.*/
);

/**
 * @brief	Ouverture d'un fichier INI pour lecture.
 * @return 	Status.
 */
IniFile_Status_e
IniFile_OpenRead(
		const IniFile_s*	IniFile		/**<[in] Descripteur du contenu du fichier.*/
);

/**
 * @brief	Ouverture d'un fichier INI pour lecture, s'il existe.
 * @return 	Status.
 */
IniFile_Status_e
IniFile_OpenReadIfExist(
		const IniFile_s*	IniFile		/**<[in] Descripteur du contenu du fichier.*/
);

/**
 * @brief	Ouverture d'un fichier INI pour ecriture.
 * @return 	Status.
 */
IniFile_Status_e
IniFile_OpenWrite(
		const IniFile_s*	IniFile		/**<[in] Descripteur du contenu du fichier.*/
);

/**
 * @brief	Fermeture d'un fichier INI.
 * @return 	void.
 */
void
IniFile_Close(
		const IniFile_s*	IniFile		/**<[in] Descripteur du contenu du fichier.*/
);

/**
 * @brief	Ouverture en ecriture  du fichiers INI.
 * @return 	Inif file Status.
 */
IniFile_Status_e
IniFile_OpenWriteFile(
		const IniFile_s*	IniFile		/**<[in] Descripteur du contenu du fichier.*/
);

/**
 * @brief	Ouverture en ecriture du fichiers INI temporaire.
 * @return 	Inif file Status.
 */
IniFile_Status_e
IniFile_OpenWriteTempFile(
		const IniFile_s*	IniFile		/**<[in] Descripteur du contenu du fichier.*/
);

/**
 * @brief	Fermeture du fichiers INI et du temp
 * @return 	void.
 */
void
IniFile_CloseFile(
	void
);

/**
 * @brief	Suppression du fichier INI.
 * @return 	void.
 */
void
IniFile_DeleteFile(
		const IniFile_s*	IniFile		/**<[in] Descripteur du contenu du fichier.*/
);

/**
 * @brief	Renommage du fichier INI.
 * @return 	void.
 */
void
IniFile_RenameFile(
		const IniFile_s*	IniFile		/**<[in] Descripteur du contenu du fichier.*/
);


/**
 * @brief	Lecture d'une valeur dans un fichier ouvert.
 * @return 	Status.
 */
Status_e
IniFile_ReadValueFromKey(
		const Key_s*			pKey,		/**<[in] Cle a lire.*/
		const void*				pValue		/**<[in] Pointeur vers la valeur lue.*/
);

/**
 * @brief	Ecriture d'une valeur dans un fichier ouvert.
 * @return 	Status.
 */
Status_e
IniFile_WriteValueFromKey(
		const Key_s*			pKey,		/**<[in] Cle a ecrire.*/
		const void*				Value		/**<[in] Valeur a ecrire.*/
);

/**
 * @brief	Ecriture d'une valeur brute dans un fichier ouvert.
 * @return 	Status.
 */
Status_e
IniFile_WriteRawData(
		const Key_s*			pKey,		/**<[in] Cle a ecrire.*/
		const void*				Value		/**<[in] Valeur a ecrire.*/
);
	 
/**
 * @brief	Predicat indiquant si un fichier est deja ouvert.
 * @return 	Bool_e.
 */
Bool_e
IniFile_IsAFileOpened(
	void
);
 /**
 * @}
 */ 
 
 
 /**
 * @}
 */ 


#endif /* FCT_INIFILE_H_ */
