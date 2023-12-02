#ifndef __USART_H
#define __USART_H

#include "defines.h"

void SetupUSART(void);
void USART_PutChar(char c);
void USART_PutStr(char *str);
void USART_PutHexByte(unsigned char byte);
int	 USART_GetStr(char *buf, int len);

#endif 
