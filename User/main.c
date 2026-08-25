#include <STC8H.H>
#include "gpio.h"
#include "dma.h"
#include "tim.h"
#include "usart.h"
#include "modbus.h"
#include "numled.h"
#include "key.h"
#include "clock.h"

BMS_Device_t  bms_dev;
extern volatile uint8_t Soc_Flag;

volatile uint8_t g_1s_flag = 0;

void main(void){
    int poll_result;
    SystemClock_Init();
    MX_UART2_Init();
		UART1_Init();       // 新增调试口
    Num_Led_Init();
    Key_Init();
    MX_TIM0_Init();
    EA = 1;
    while (1){
        Key_Action();





        if (g_1s_flag && Soc_Flag == 0){
            g_1s_flag = 0;
            BMS_StartRealtimeRead();
        }
        if (Soc_Flag == 0){
            poll_result = BMS_PollRealtimeData();
            if (poll_result != 0){
                /* 成功显示新数据；失败时rt未被覆盖，显示上次缓存数据 */
                Soc_Show();
            }
        }
    }
}

static unsigned char cnt_key = 0;
static unsigned int cnt_1s = 0;

void App_1msTask(void){
    Led_under_deal();
    if(++cnt_key >= 10){
        Key_Scan();
        cnt_key = 0;
    }
    if(++cnt_1s >= 1000){
        cnt_1s = 0;
        g_1s_flag = 1;         /* trigger periodic read every 1s */
    }
}





