#include "usart.h"

void SetupUSART()
{
	GPIO_InitTypeDef  GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;

	// Enable USART1 and GPIOA clock
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1 | RCC_APB2Periph_GPIOA, ENABLE);

	// Configure USART1 Rx (PA10) as input floating
	GPIO_InitStructure.GPIO_Pin	 = GPIO_Pin_10;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	// Configure USART1 Tx (PA9) as alternate function push-pull */
	GPIO_InitStructure.GPIO_Pin	  = GPIO_Pin_9;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF_PP;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	USART_InitStructure.USART_BaudRate			  = 115200;
	USART_InitStructure.USART_WordLength		  = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits			  = USART_StopBits_1;
	USART_InitStructure.USART_Parity			  = USART_Parity_No;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode				  = USART_Mode_Rx | USART_Mode_Tx;
	USART_Init(USART1, &USART_InitStructure);
	USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);
	USART_Cmd(USART1, ENABLE);
}

void USART_PutChar(char c)
{
	// write a character to the USART
	USART_SendData(USART1, (uint8_t)c);

	/// Loop until the end of transmission
	while (USART_GetFlagStatus(USART1, USART_FLAG_TC) == RESET)
		;
}

void USART_PutStr(char *str)
{
	while (*str)
	{
		USART_PutChar(*str);
		str++;
	}
}

void USART_PutHexByte(unsigned char byte)
{
	char n = (byte >> 4) & 0x0F;
	// Write high order digit
	if (n < 10)
		USART_PutChar(n + '0');
	else
		USART_PutChar(n - 10 + 'A');

	// Write low order digit
	n = (byte & 0x0F);
	if (n < 10)
		USART_PutChar(n + '0');
	else
		USART_PutChar(n - 10 + 'A');
}

int USART_GetStr(char *buf, int len)
{
	int	 i = 0;
	char k = 0;

	while (k != 0x0D)
	{
		// Is a byte available from UART?
		if (USART_GetFlagStatus(USART1, USART_FLAG_RXNE) == SET)
		{
			k = USART_ReceiveData(USART1); // get input char
			USART_PutChar(k);			   // echo input char
			buf[i] = k;					   // store input char

			if ((k == 0x0A) || (k == 0x0D) || (k == 0x1B)) // LF,CR or Esc = EXIT
				break;

			if (++i == len) // Buffer Full = EXIT
				break;

			if ((k == 0x7F) && (i > 0)) // Backspace
				i--;
		}
	}

	buf[i] = 0;
	return i;
}

// UART2 interrupt
void USART1_IRQHandler(void)
{
	uint8_t data;
	if (USART_GetITStatus(USART1, USART_IT_RXNE) != RESET)
	{
		data = USART_ReceiveData(USART1);
		// add usart receive here
		// uart_receive_input(data);

		USART_ClearFlag(USART1, USART_FLAG_RXNE);
		USART_ClearITPendingBit(USART1, USART_IT_RXNE);
	}
}
