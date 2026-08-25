#include <STC8H.H>
#include "clock.h"
#include "intrins.h"
#include "modbus.h"
#include <stdio.h>

#define UART_BAUD       115200UL
#define UART2_DIVISOR  ((SYSTEM_CLOCK_HZ + UART_BAUD * 2UL) / (UART_BAUD * 4UL))
#define UART2_RELOAD   (65536UL - UART2_DIVISOR)


#define BRT   (65536 - (IRC_CLOCK_HZ/115200+2)/4)

volatile unsigned char uart_rx_byte;
volatile bit uart_rx_ready;
void UART1_Init(void)
{
    P_SW1 &= (unsigned char)~0xC0;  // UART1 使用 P3.0/P3.1

    SCON = 0x50;                    // 8位数据，可变波特率
    AUXR |= 0x40;                   // Timer1 使用1T模式
    TMOD &= 0x0F;
    TMOD |= 0x20;                   // Timer1 方式2

    TH1 = 0xB4;       // 35MHz，UART1，115200，Timer1 1T
		TL1 = TH1;

    TR1 = 1;
    TI = 0;
}
void UART1_SendByte(unsigned char dat)
{
    TI = 0;
    SBUF = dat;
    while (!TI);
    TI = 0;
}

char putchar(char c)
{
    UART1_SendByte((unsigned char)c);
    return c;
}

void MX_UART2_Init(void)
{
    unsigned int reload;

    /* P1.0=RXD2?P1.1=TXD2 */
//		P_SW1 &= (unsigned char)~0xC0;//UART1??P3.0/P3.1
    P_SW2 &= (unsigned char)~0x01;//UART2??P1.0/P1.1 

    /* P1.1?????P1.0????? */
    P1M0 = (P1M0 & (unsigned char)~0x03) | 0x02;
    P1M1 &= (unsigned char)~0x03;
//		/* P3.1?????P3.0????? */
//    P3M0 = (P3M0 & (unsigned char)~0x03) | 0x02;
//    P3M1 &= (unsigned char)~0x03;
		//??2??????
		P1PU &= (unsigned char)~0x03;
    /*
     * bit6 S2ST2 = 1???2??Timer2????????
     * bit4 S2REN = 1?????
     * 8?????????
     */
    S2CON = 0x50;

    AUXR |= 0x04;       /* Timer2??1T?? */
    AUXR &= (unsigned char)~0x10;

    reload = (unsigned int)UART2_RELOAD;
       

    T2L = (unsigned char)reload;
    T2H = (unsigned char)(reload >> 8);

    S2CON &= (unsigned char)~0x03; /* ?S2TI?S2RI */
    IE2 |= 0x01;            /* ????2???? */

    AUXR |= 0x10;       /* ??Timer2 */
}

void UART2_SendByte(unsigned char dat)
{
    S2CON &= (unsigned char)~0x02;
    S2BUF = dat;
    while ((S2CON & 0x02) == 0);
    S2CON &= (unsigned char)~0x02;
}

void UART2_SendBuffer(const unsigned char *buf, unsigned int len)
{
    while (len--) UART2_SendByte(*buf++);
}

bit UART2_ReadByte(unsigned char *dat)
{
    if ((S2CON & 0x01) == 0) {
        return 0;                 // ??????
    }

    *dat = S2BUF;
    S2CON &= (unsigned char)~0x01; // ? S2RI
    return 1;
}

void UART2_ISR(void) interrupt UART2_VECTOR
{
    if (S2CON & 0x01) {
        unsigned char dat = S2BUF;
        S2CON &= (unsigned char)~0x01;
        BMS_OnRxByte(dat);
    }
}


