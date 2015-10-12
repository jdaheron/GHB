/*
 * printf.h
 *
 *  Created on: 19 fevr. 2013
 *      Author: f.teutsch
 */

#ifndef _PRINTF_H_
#define _PRINTF_H_


#include "BSP.h"


//int32_t _printf(const uint8_t *format, ...);
//int32_t _sprintf(uint8_t *out, const uint8_t *format, ...);

int _printf(const char *format, ...);
int _no_printf(const char *format, ...);
int _sprintf(char *out, const char *format, ...);
void _printfFloat(float Value, uint8_t NbChiffreApresLaVirgule);


#endif /* PRINTF_H_ */
