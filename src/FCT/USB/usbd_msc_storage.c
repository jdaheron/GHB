
/* Includes ------------------------------------------------------------------*/

#include "BSP.h"
#include "usbd_msc_storage.h"
#include "diskio.h"
#include "util_TSW.h"


/* Private typedef -----------------------------------------------------------*/


/* Private define ------------------------------------------------------------*/

#define USB_MSC_Timeout_ms	1200


/* Private macro -------------------------------------------------------------*/


/* Private variables ---------------------------------------------------------*/

static Diskio_drvTypeDef *pDisk = NULL;
TSW_s TIMER_USB_MSC_Timeout;


/* Private function prototypes -----------------------------------------------*/


/* Extern function prototypes ------------------------------------------------*/


/* Private functions ---------------------------------------------------------*/

#define STORAGE_LUN_NBR                  1  
//#define STORAGE_BLK_NBR                  0x10000
//#define STORAGE_BLK_SIZ                  0x200

int8_t UsbStorage_Init (uint8_t lun);

int8_t UsbStorage_GetCapacity (uint8_t lun,
                           uint32_t *block_num, 
                           uint16_t *block_size);

int8_t  UsbStorage_IsReady (uint8_t lun);

int8_t  UsbStorage_IsWriteProtected (uint8_t lun);

int8_t UsbStorage_Read (uint8_t lun,
                        uint8_t *buf, 
                        uint32_t blk_addr,
                        uint16_t blk_len);

int8_t UsbStorage_Write (uint8_t lun,
                        uint8_t *buf, 
                        uint32_t blk_addr,
                        uint16_t blk_len);

int8_t UsbStorage_GetMaxLun (void);

/* USB Mass storage Standard Inquiry Data */
int8_t UsbStorage_Inquirydata[] = {//36
  
  /* LUN 0 */
  0x00,		
  0x80,		
  0x02,		
  0x02,
  (STANDARD_INQUIRY_DATA_LEN - 5),
  0x00,
  0x00,	
  0x00,
  'S', 'T', 'M', ' ', ' ', ' ', ' ', ' ', /* Manufacturer : 8 bytes */
  'P', 'r', 'o', 'd', 'u', 'c', 't', ' ', /* Product      : 16 Bytes */
  ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ',
  '0', '.', '0' ,'1',                     /* Version      : 4 Bytes */
}; 

USBD_StorageTypeDef USBD_MSC_fops =
{
		UsbStorage_Init,
		UsbStorage_GetCapacity,
		UsbStorage_IsReady,
		UsbStorage_IsWriteProtected,
		UsbStorage_Read,
		UsbStorage_Write,
		UsbStorage_GetMaxLun,
		UsbStorage_Inquirydata,
  
};


/*******************************************************************************
*******************************************************************************/
void UsbStorage_SetDriver(Diskio_drvTypeDef *drv)
{
	if (drv)
		pDisk = drv;
}

/*******************************************************************************
*******************************************************************************/
void UsbStorage_StartTimeout()
{
	TSW_Start(&TIMER_USB_MSC_Timeout, USB_MSC_Timeout_ms);
}


/*******************************************************************************
*******************************************************************************/
Bool_e UsbStorage_IsTimedOut()
{
	if (TSW_IsRunning(&TIMER_USB_MSC_Timeout) != TRUE)
		return TRUE;

	return FALSE;
}


/*******************************************************************************
*******************************************************************************/
int8_t UsbStorage_Init (uint8_t lun)
{
	if (pDisk == NULL)
		return (-1);

	if (pDisk->disk_initialize() == 0)
		return (0);

	TSW_Start(&TIMER_USB_MSC_Timeout, 2 * USB_MSC_Timeout_ms);

	return (-1);
}


/*******************************************************************************
*******************************************************************************/
int8_t UsbStorage_GetCapacity (uint8_t lun, uint32_t *block_num, uint16_t *block_size)
{
	if (pDisk == NULL)
		return (-1);

	if (pDisk->disk_status() != 0)
		return (-1);

	pDisk->disk_ioctl(GET_SECTOR_COUNT, block_num);
	pDisk->disk_ioctl(GET_SECTOR_SIZE, block_size);

	return (0);
}


/*******************************************************************************
*******************************************************************************/
int8_t  UsbStorage_IsReady (uint8_t lun)
{
	if (pDisk == NULL)
		return (-1);

	if (pDisk->disk_status() & STA_NODISK)
		return (-1);

	// Relance timeout
	TSW_Start(&TIMER_USB_MSC_Timeout, USB_MSC_Timeout_ms);

	if (pDisk->disk_ioctl(CTRL_SYNC, 0) != 0)
		return (-1);

	return (0);
}


/*******************************************************************************
*******************************************************************************/
int8_t  UsbStorage_IsWriteProtected (uint8_t lun)
{
	if (pDisk == NULL)
		return (-1);

	if (pDisk->disk_status() & STA_PROTECT)
		return  (-1);

	return  0;
}


/*******************************************************************************
*******************************************************************************/
int8_t UsbStorage_Read (uint8_t lun,
                 uint8_t *buf, 
                 uint32_t blk_addr,                       
                 uint16_t blk_len)
{
	if (pDisk == NULL)
		return (-1);

	return pDisk->disk_read(buf, blk_addr, blk_len);
}


/*******************************************************************************
*******************************************************************************/
int8_t UsbStorage_Write (uint8_t lun,
                  uint8_t *buf, 
                  uint32_t blk_addr,
                  uint16_t blk_len)
{
	if (pDisk == NULL)
		return (-1);

	return pDisk->disk_write(buf, blk_addr, blk_len);
}


/*******************************************************************************
*******************************************************************************/
int8_t UsbStorage_GetMaxLun (void)
{
	if (pDisk == NULL)
		return (-1);

	return (STORAGE_LUN_NBR - 1);
}


