#ifndef __USART_H__
#define __USART_H__

void UART1_Init(void);
void UART1_SendByte(unsigned char dat);

void MX_UART2_Init(void);
void UART2_SendByte(unsigned char dat);
void UART2_SendBuffer(const unsigned char *buf, unsigned int len);
bit UART2_ReadByte(unsigned char *dat);
void UART2_ISR(void);

#endif
