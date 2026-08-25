#include "numled.h"
#include "modbus.h"
#include <intrins.h>
#include "key.h"

static unsigned char led_disp_step = 0;   // 扫描步骤
volatile ByteBit_Union DG1_Flag = {0};
volatile ByteBit_Union DG2_Flag = {0};
volatile ByteBit_Union DG3_Flag = {0};
volatile BMS_RealtimeData_t rt;
volatile unsigned char g_rt_ready = 0;

void Num_Led_Init(void)
{
		// 初始化全部5个IO为推挽输出模式
    led_io_1_out;
    led_io_2_out;
    led_io_3_out;
    led_io_4_out;
    led_io_5_out;

    // 全部输出低电平，熄灭所有LED
    led_io_1 = 0;
    led_io_2 = 0;
    led_io_3 = 0;
    led_io_4 = 0;
    led_io_5 = 0;
}

void Led_under_deal(void) 
{
		// 第一步：先将所有IO设为输入(高阻)，避免互相干扰
		led_io_1=0;
		led_io_2=0;
		led_io_3=0;	
		led_io_4=0;	
		led_io_5=0;
		led_io_1_in;
		led_io_2_in;
		led_io_3_in;
		led_io_4_in;
		led_io_5_in;
		
    // 第二步：根据段数据，将对应的IO设为阳极拉高，其余设为阴极拉低
    if (led_disp_step == 0) {
        if (DG3_B) {led_io_2_out;led_io_2=1;}//B3
        if (DG3_D) {led_io_3_out;led_io_3=1;}//D3
        if (DG3_F) {led_io_4_out;led_io_4=1;}//F3
        if (DG3_G) {led_io_5_out;led_io_5=1;}//G3
        led_io_1_out;			
				led_io_1=0;//阴极线1
    } 
    else if (led_disp_step == 1) {
        if (DG2_B) {led_io_3_out;led_io_3=1;}//B2
        if (DG2_D) {led_io_4_out;led_io_4=1;}//D2
				if (DG2_E) {led_io_5_out;led_io_5=1;}//E2
        if (DG3_A) {led_io_1_out;led_io_1=1;}//A3
        led_io_2_out;			
				led_io_2=0;//阴极线2
    } 
    else if (led_disp_step == 2) {
        if (DG2_A) {led_io_2_out;led_io_2=1;}//A2
        if (DG2_C) {led_io_4_out;led_io_4=1;}//C2
				if (DG2_F) {led_io_5_out;led_io_5=1;}//F2
        if (DG3_C) {led_io_1_out;led_io_1=1;}//C3
        led_io_3_out;			
				led_io_3=0;//阴极线3
    } 
    else if (led_disp_step == 3) {
        if (DG3_E) {led_io_1_out;led_io_1=1;}//E3
				if (DG2_G) {led_io_5_out;led_io_5=1;}//G2
				if (DG1_B) {led_io_3_out;led_io_3=1; }//B1
				if (DG1_C) {led_io_2_out;led_io_2=1; }//C1
				led_io_4_out;			
				led_io_4=0;//阴极线4
    }
    else if (led_disp_step == 4) {
				if (DG1_A) {led_io_3_out;led_io_3=1; }//充放电指示灯
				if (DG1_D) {led_io_2_out;led_io_2=1; }//%号
				led_io_5_out;			
				led_io_5=0;//阴极线5
	}
    led_disp_step++;
    if (led_disp_step >= 5) led_disp_step = 0;
}

/*数码全显或全关*/
void DG_ALL(bit EN_DIS)
{
	if(EN_DIS)	//全显
	{
      DG1_Flag.byte = 0xff;
      DG2_Flag.byte = 0xff;
			DG3_Flag.byte = 0xff;
	}
	else	//全关
	{
		DG1_Flag.byte = 0;
		DG2_Flag.byte = 0;
		DG3_Flag.byte = 0;		
	}
}
//后面的符号灯：充电全开 -- 1 放电开一个 -- 2 全关 -- 3
void DG_LED(unsigned char EN_DIS) {
	if(EN_DIS == 1) { // 充电全开
		DG1_A = 1;
		DG1_D = 1;
	}else if (EN_DIS == 2) { // 放电开一个
		DG1_A = 0;
		DG1_D = 1;
	}else if (EN_DIS == 3) { // 全关
		DG1_A = 0;
		DG1_D = 0;
	}
}

/*
num1:第一位数码管显示的百位数字（1）
num2:第二位数码管显示的十位数字或报错头E（0-9，E）
num3:第三位数码管显示的个位数字或报错码（0-9，A，b，c，d，E，F）
*/
void DG_Display(unsigned int num1,unsigned int num2,unsigned int num3)
{	
		bit ea_save;               // 保存 EA 状态
    ea_save = EA;              // 备份当前中断状态
    EA = 0;                    // 关中断，避免临界冲突
		DG_ALL(0);//全写0
		// 设置第一位数码管的段(0/1)
		switch(num1)
		{
				case 1: DG1_B=1;DG1_C=1; break;// 1
				default: break;		
		}
		// 设置第二位数码管的段，0-9，E
		switch(num2)
		{
				case 0: DG2_A=1;DG2_B=1;DG2_C=1;DG2_D=1;DG2_E=1;DG2_F=1; break;//0
				case 1: DG2_B=1;DG2_C=1; break;//1
				case 2: DG2_A=1;DG2_B=1;DG2_D=1;DG2_E=1;DG2_G=1; break;//2
				case 3: DG2_A=1;DG2_B=1;DG2_C=1;DG2_D=1;DG2_G=1; break;//3
				case 4: DG2_B=1;DG2_C=1;DG2_F=1;DG2_G=1; break;//4
				case 5: DG2_A=1;DG2_C=1;DG2_D=1;DG2_F=1;DG2_G=1; break;//5
				case 6: DG2_A=1;DG2_C=1;DG2_D=1;DG2_E=1;DG2_F=1;DG2_G=1; break;//6
				case 7: DG2_A=1;DG2_B=1;DG2_C=1; break;//7
				case 8: DG2_A=1;DG2_B=1;DG2_C=1;DG2_D=1;DG2_E=1;DG2_F=1;DG2_G=1; break;//8
				case 9: DG2_A=1;DG2_B=1;DG2_C=1;DG2_D=1;DG2_F=1;DG2_G=1; break;//9
				case 10: DG2_A=1;DG2_D=1;DG2_E=1;DG2_F=1;DG2_G=1; break;//E
				default: break;		
		}
		// 设置第三位数码管的段 （0-9，A，b，c，d，E，F）
		switch(num3) 
		{
				case 0: DG3_A=1;DG3_B=1;DG3_C=1;DG3_D=1;DG3_E=1;DG3_F=1; break;//0
				case 1: DG3_B=1;DG3_C=1; break;//1
				case 2: DG3_A=1;DG3_B=1;DG3_D=1;DG3_E=1;DG3_G=1; break;//2
				case 3: DG3_A=1;DG3_B=1;DG3_C=1;DG3_D=1;DG3_G=1; break;//3
				case 4: DG3_B=1;DG3_C=1;DG3_F=1;DG3_G=1; break;//4
				case 5: DG3_A=1;DG3_C=1;DG3_D=1;DG3_F=1;DG3_G=1; break;//5
				case 6: DG3_A=1;DG3_C=1;DG3_D=1;DG3_E=1;DG3_F=1;DG3_G=1; break;//6
				case 7: DG3_A=1;DG3_B=1;DG3_C=1; break;//7
				case 8: DG3_A=1;DG3_B=1;DG3_C=1;DG3_D=1;DG3_E=1;DG3_F=1;DG3_G=1; break;//8
				case 9: DG3_A=1;DG3_B=1;DG3_C=1;DG3_D=1;DG3_F=1;DG3_G=1; break;//9
				case 10: DG3_A=1;DG3_B=1;DG3_C=1;DG3_E=1;DG3_F=1;DG3_G=1; break;//A
				case 11: DG3_C=1;DG3_D=1;DG3_E=1;DG3_F=1;DG3_G=1; break;//b
				case 12: DG3_D=1;DG3_E=1;DG3_G=1; break;//c
				case 13: DG3_B=1;DG3_C=1;DG3_D=1;DG3_E=1;DG3_G=1; break;//d
			//case 13: DG3_A=1;DG3_B=1;DG3_D=1;DG3_E=1;DG3_F=1;DG3_G=1;break;//e
				case 14: DG3_A=1;DG3_D=1;DG3_E=1;DG3_F=1;DG3_G=1; break;//E
				case 15: DG3_A=1;DG3_E=1;DG3_F=1;DG3_G=1;break;//F		
				default: break;
		 }
		 // ===== 临界区结束 =====
     EA = ea_save;    // 恢复中断状态
}

signed short int out_current;
void Soc_Show(void)
{
    unsigned int fault_code = 0U;
    unsigned int idx;
    ErrFlag_t terr_flag;
    
    unsigned short int soc;
    unsigned int soc_bai, soc_shi, soc_ge;

    terr_flag = rt.err_flag;
    if(terr_flag.raw == 0){/* always show SOC */
        out_current = rt.out_current;
        if(out_current <= 10){/* 0.1A */
            soc = rt.soc / 10;
            soc_bai = soc / 100;
            soc_shi = (soc / 10) % 10;
            soc_ge = soc % 10;
            DG_Display(soc_bai, soc_shi, soc_ge);
            DG_LED(2);
        }else{/* charge */
            soc = rt.soc / 10;
            soc_bai = soc / 100;
            soc_shi = (soc / 10) % 10;
            soc_ge = soc % 10;
            DG_Display(soc_bai, soc_shi, soc_ge);
            DG_LED(1);
        }
//        Relay = Relay_ON;
    }else{/* fault */
        for (idx = 0U; idx < 16U; idx++) {
            if ((terr_flag.raw & ((unsigned short int)1U << idx)) != 0U) {
                fault_code = idx;
            }
        }
        DG_Display(0, 10, fault_code);
        DG_LED(3);
    }
}