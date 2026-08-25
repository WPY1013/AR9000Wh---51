#include "key.h"
#include "gpio.h"
#include "numled.h"
#include "modbus.h"

uint8_t g_KeyFlag = 0;                 // 按键有效标志：0=低电平有效，1=高电平有效
volatile KEY_STATE KeyState = KEY_CHECK;
volatile KEY_TYPE g_KeyActionFlag = NULL_KEY;
volatile uint16_t TimeCnt = 0U;

void Key_Init(void){
		Relay = Relay_OFF;  // 继电器断开
		// P1.5 推挽输出
    P1M1 &= ~(1 << 5);
    P1M0 |=  (1 << 5);

    Relay = Relay_OFF;
		// P1.2配置为高阻输入：M1=1，M0=0
    P1M1 |=  0x04;
    P1M0 &= ~0x04;
    // 状态机变量初始化
    KeyState = KEY_CHECK;
    g_KeyActionFlag = NULL_KEY;
    TimeCnt = 0U;
}

void Key_Scan(void){
	static uint16_t TimeCnt = 0;
	static uint8_t lock = 0;
	switch (KeyState){
		case KEY_CHECK://按键未按下状态，此时判断Key的值
			if(!Key){
				KeyState =  KEY_COMFIRM;  //检测到Key值为0，说明按键开始按下，进入下一状态
			}
			TimeCnt = 0;                  //计数器清零
			lock = 0;
			break;
		case KEY_COMFIRM:
			if(!Key){//查看当前Key是否为0，再次确认是否按下
				if(!lock)   lock = 1;
				TimeCnt++;   
			}
			else{
				if(lock){// 不是第一次进入，释放按键才执行
					/*按键时长判断*/
					if(TimeCnt > 100){// 超过 1s
						g_KeyActionFlag = LONG_KEY;
						TimeCnt = 0;  
					}
					else{// Key值变为1，说明此次操作为短按
						g_KeyActionFlag = SHORT_KEY;// 短按
					}
					/*按键时长判断*/
					KeyState =  KEY_RELEASE;// 需要进入按键释放状态
				}
				else{// 当前Key值为1，确认为抖动，返回上一个状态
						KeyState =  KEY_CHECK;// 回到上一个状态
				}
			}break;
		 case KEY_RELEASE:
			 if(Key){//当前Key值为1，说明按键已经释放，返回开始状态
				 KeyState =  KEY_CHECK;    
			 }break;
		 default: break;
	}
}
volatile int sleep_result = 0;
volatile uint8_t Soc_Flag = 0;
extern signed short int out_current;
void Key_Action(void){
	KEY_TYPE action = g_KeyActionFlag;
	if (action == SHORT_KEY){//短按
		Soc_Flag = 0;
		Relay = Relay_ON;  // 继电器闭合
		Soc_Show();              // 立即显示上一次成功解析并缓存的SOC
		g_1s_flag = 1;           // 同时触发后台读取最新数据
		g_KeyActionFlag = NULL_KEY;
	}
	else if (action == LONG_KEY){//长按
		volatile int ret;
		
		// 立即关闭数码管，避免等待 BMS 响应导致的灭屏延时
		DG_ALL(0);
		Soc_Flag = 1;
		//ret = BMS_SendSleepCommand();
		if(out_current <= 50){
		//等待mppt放电电流小于500mA后断开继电器
//		DelayMs(200);
			Relay = Relay_OFF;  // 继电器断开
			/* 先清除当前长按事件，避免阻塞通信期间覆盖后续短按事件 */
		g_KeyActionFlag = NULL_KEY;
		}
	}

}
