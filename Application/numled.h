#ifndef _LED_H_
#define _LED_H_

#include <STC8H.H>


typedef union
{
	unsigned char byte;
	struct{
		unsigned char bit0:1;
		unsigned char bit1:1;
		unsigned char bit2:1;
		unsigned char bit3:1;
		unsigned char bit4:1;
		unsigned char bit5:1;
		unsigned char bit6:1;
		unsigned char bit7:1;
	}Bits;
}ByteBit_Union;


//根据具体端口进行定义
// 位掩码定义
#define IO1  0x08
#define IO2  0x10   
#define IO3  0x20
#define IO4  0x40   
#define IO5  0x80

// 引脚位变量
#define led_io_1  P33
#define led_io_2  P34
#define led_io_3  P35
#define led_io_4  P36
#define led_io_5  P37

// 方向控制宏（若跨端口改引脚，需把 P0 改成 P1 等）
#define led_io_1_in  P3M1 |= IO1; P3M0 &= ~IO1; 
#define led_io_2_in  P3M1 |= IO2; P3M0 &= ~IO2;
#define led_io_3_in  P3M1 |= IO3; P3M0 &= ~IO3;
#define led_io_4_in  P3M1 |= IO4; P3M0 &= ~IO4;
#define led_io_5_in  P3M1 |= IO5; P3M0 &= ~IO5;

#define led_io_1_out P3M1 &= ~IO1; P3M0 |= IO1;
#define led_io_2_out P3M1 &= ~IO2; P3M0 |= IO2;
#define led_io_3_out P3M1 &= ~IO3; P3M0 |= IO3;
#define led_io_4_out P3M1 &= ~IO4; P3M0 |= IO4;
#define led_io_5_out P3M1 &= ~IO5; P3M0 |= IO5;
// -------------------- 外部变量声明 --------------------
extern volatile ByteBit_Union DG1_Flag;
extern volatile ByteBit_Union DG2_Flag;
extern volatile ByteBit_Union DG3_Flag;
extern volatile unsigned char g_rt_ready;

#define DG1_A DG1_Flag.Bits.bit0
#define DG1_B DG1_Flag.Bits.bit1
#define DG1_C DG1_Flag.Bits.bit2
#define DG1_D DG1_Flag.Bits.bit3
//#define DG1_E DG1_Flag.Bits.bit4
//#define DG1_F DG1_Flag.Bits.bit5
//#define DG1_G DG1_Flag.Bits.bit6

#define DG2_A DG2_Flag.Bits.bit0
#define DG2_B DG2_Flag.Bits.bit1
#define DG2_C DG2_Flag.Bits.bit2
#define DG2_D DG2_Flag.Bits.bit3
#define DG2_E DG2_Flag.Bits.bit4
#define DG2_F DG2_Flag.Bits.bit5
#define DG2_G DG2_Flag.Bits.bit6

#define DG3_A DG3_Flag.Bits.bit0
#define DG3_B DG3_Flag.Bits.bit1
#define DG3_C DG3_Flag.Bits.bit2
#define DG3_D DG3_Flag.Bits.bit3
#define DG3_E DG3_Flag.Bits.bit4
#define DG3_F DG3_Flag.Bits.bit5
#define DG3_G DG3_Flag.Bits.bit6


void Num_Led_Init(void);
void Led_under_deal(void);	//数码管灯阵扫描函数
void DG_ALL(bit EN_DIS);	//数码全显或全关
void DG_LED(unsigned char EN_DIS);//后面的符号灯：充电全开 -- 1 放电开一个 -- 2 全关 -- 3
void DG_Display(unsigned int num1,unsigned int num2,unsigned int num3);
void Soc_Show(void);

#endif
