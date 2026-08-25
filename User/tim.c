#include <STC8H.H>
#include "tim.h"
#include "clock.h"

#define T0_TICK_HZ       1000UL
#define T0_1MS_COUNT       	((SYSTEM_CLOCK_HZ + T0_TICK_HZ / 2UL) / T0_TICK_HZ)
#define T0_1MS_RELOAD       (65536UL - T0_1MS_COUNT)

volatile unsigned long g_ms_ticks = 0UL;
volatile unsigned char g_1ms_flag = 0U;
void MX_TIM0_Init(void)
{
    unsigned int reload;

    reload = (unsigned int)T0_1MS_RELOAD;

    TMOD &= 0xF0;
    TMOD |= 0x01;       /* Timer0，16 位模式 */

    AUXR |= 0x80;      /* Timer0 使用 1T 模式 */

    TH0 = (unsigned char)(reload >> 8);
    TL0 = (unsigned char)reload;

    TF0 = 0;//清楚TF0标志
    ET0 = 1;
    PT0 = 1;//使能定时器中断
    TR0 = 1;//开启定时器
}

void Timer0_ISR(void) interrupt 1
{
    unsigned int reload;

    reload = (unsigned int)T0_1MS_RELOAD;
    TH0 = (unsigned char)(reload >> 8);
    TL0 = (unsigned char)reload;

    TF0 = 0;
    g_ms_ticks++;
    g_1ms_flag = 1U;

    App_1msTask();
}


unsigned long HAL_GetTick(void)
{
    unsigned long tick;
    bit ea_backup;

    ea_backup = EA;
    EA = 0;
    tick = g_ms_ticks;
    EA = ea_backup;

    return tick;
}
unsigned long GetTickMs(void)
{
    return HAL_GetTick();
}

void DelayMs(unsigned int ms)
{
    unsigned long start;

    start = HAL_GetTick();

    while ((HAL_GetTick() - start) < (unsigned long)ms)
    {
        ;
    }
}
