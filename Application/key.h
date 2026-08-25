#ifndef __KEY_H__
#define __KEY_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <STC8H.H>


#define GPIO_KEY_PORT   P1
#define GPIO_KEY_PIN    2
//#define GPIO_NoSleep_PORT P1
//#define GPIO_NoSleep_PIN  4
#define Relay_ON  1   // 开启 = 高电平
#define Relay_OFF 0   // 关闭 = 低电平

sbit Relay = P1^5;  
sbit Key = P1^2;
// 读取引脚电平的宏
//#define Key   ((GPIO_KEY_PORT & (1 << GPIO_KEY_PIN)) ? 1 : 0)
//#define NoSleep ((GPIO_Sleep_PORT & (1 << GPIO_Sleep_PIN)) ? 1 : 0)

// 初始化需直接操作寄存器，可定义两个宏用于设置输出
//#define NoSleep_HIGH   (GPIO_NoSleep_PORT |= (1 << GPIO_Sleep_PIN))
//#define NoSleep_LOW    (GPIO_NoSleep_PORT &= ~(1 << GPIO_Sleep_PIN))
//#define Key_INPUT    (GPIO_KEY_PORT |= (1 << GPIO_KEY_PIN))   // 写 1 为输入

typedef enum
{
    KEY_CHECK = 0,
    KEY_COMFIRM = 1,
    KEY_RELEASE = 2 
}KEY_STATE;



typedef enum
{
    NULL_KEY = 0,
    SHORT_KEY,
    LONG_KEY
} KEY_TYPE;

extern volatile KEY_STATE KeyState;
extern volatile KEY_TYPE g_KeyActionFlag;
extern volatile unsigned int TimeCnt;
extern volatile unsigned char g_1s_flag;

void Key_Init(void);
void Key_Scan(void);
void Key_Action(void);

#ifdef __cplusplus
}
#endif

#endif /* __KEY_H__ */
