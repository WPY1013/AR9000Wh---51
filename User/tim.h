#ifndef __TIM_H__
#define __TIM_H__


extern volatile unsigned long g_ms_ticks;
extern volatile unsigned char g_1ms_flag;

void MX_TIM0_Init(void);
void Timer0_ISR(void);
void App_1msTask(void);
unsigned long HAL_GetTick(void);
unsigned long GetTickMs(void);
void DelayMs(unsigned int ms);


#endif
