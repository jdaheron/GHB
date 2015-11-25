/**
 ***************************************************************************************************
 * @file	equ_AM23xx.c
 * @author	j.daheron
 * @version	1.0.0
 * @date	2015.10.18
 * @brief   Gestion du capteur Temperature + Humidité.
 ***************************************************************************************************
 */


/* Includes ***************************************************************************************/

#include "equ_AM23xx.h"


/* External Variables *****************************************************************************/


#define AM23xx_I2C_ADDRESS	0xB8



Bool_e AM23xx_Read(int16_t* pTemp, uint16_t* pHumid)
{
	uint8_t Buff[10];
	Bool_e IsValid = FALSE;


	// Wakup
	I2C1_SendStart();
	I2C1_SendAddress(AM23xx_I2C_ADDRESS, 0);
	I2C1_SendByte(0);
	//Delay_ms(2);
	I2C1_SendStop();
	Delay_ms(20);

	for (int NbTry = 0; NbTry < 3; NbTry++)
	{
		I2C1_SendStart();
		I2C1_SendAddress(AM23xx_I2C_ADDRESS, 0);
		I2C1_SendByte(0x03);
		I2C1_SendByte(0x00);
		I2C1_SendByte(0x04);
		I2C1_SendStop();

		Delay_ms(5);

		I2C1_SendStart();
		I2C1_SendAddress(AM23xx_I2C_ADDRESS, 1);
		for (int i = 0; i<100; i++);
		Buff[0] = I2C1_ReceiveByte(TRUE);//fct
		Buff[1] = I2C1_ReceiveByte(TRUE);//len
		Buff[2] = I2C1_ReceiveByte(TRUE);//humid (msb)
		Buff[3] = I2C1_ReceiveByte(TRUE);//humid (lsb)
		Buff[4] = I2C1_ReceiveByte(TRUE);//temp (msb)
		Buff[5] = I2C1_ReceiveByte(TRUE);//temp (lsb)
		Buff[6] = I2C1_ReceiveByte(TRUE);//crc
		Buff[7] = I2C1_ReceiveByte(FALSE);//crc
		I2C1_SendStop();


		//for (int i = 0; i<10; i++)
		//	_printf("0x%02X ", Buff[i]);
		//_printf("\n");

		if ((Buff[0] == 0x03) && (Buff[1] == 4))
		{
			*pTemp  = (Buff[4] << 8) + Buff[5];
			*pHumid = (Buff[2] << 8) + Buff[3];

			IsValid = TRUE;
			break;
		}
	}

	return IsValid;
}


/**

 /**
 * @}
 */ 

/* End Of File ************************************************************************************/
