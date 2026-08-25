#ifndef __MODBUS_H__
#define __MODBUS_H__

#ifdef __cplusplus
extern "C" {
#endif

/* ================================================================
 * ? 51 ???????????
 * ================================================================ */
typedef unsigned char   uint8_t;
typedef signed char     int8_t;
typedef unsigned int    uint16_t;
typedef signed int      int16_t;
typedef unsigned long   uint32_t;
typedef signed long     int32_t;
/* ?? NULL */
#ifndef NULL
#define NULL    ((void *)0)
#endif
/* ================================================================
 * ????
 * ================================================================ */
#define BMS_SLAVE_ID            0x01U
#define BMS_USART_TIMEOUT       2000U        /* ms??????????????? */

/* ================================================================
 * ??????
 * ================================================================ */
typedef enum {
    CELL_MATERIAL_LiFePO4   = 1,            /* ???? */
    CELL_MATERIAL_NCM       = 2,            /* ??? */
    CELL_MATERIAL_NaIon     = 3             /* ?? */
} CellMaterial_t;


/* ================================================================
 * NTC ????
 * ================================================================ */
typedef enum {
    NTC_B3930 = 1,
    NTC_B3435 = 2
} NTCMaterial_t;

/* ================================================================
 * ????? (?? 104 ? Err_Flag_L)
 * ================================================================ */
typedef union {
    uint16_t raw;
    struct {
        uint16_t CHG             : 1;  /* Bit0  ????(????) */
        uint16_t DSG             : 1;  /* Bit1  ????(????) */
        uint16_t Cell_OV         : 1;  /* Bit2  ???? */
        uint16_t Cell_UV         : 1;  /* Bit3  ???? */
        uint16_t Pack_OV         : 1;  /* Bit4  ????? */
        uint16_t Pack_UV         : 1;  /* Bit5  ????? */
        uint16_t TMP_Max_CHG     : 1;  /* Bit6  ???? */
        uint16_t TMP_Max_DSG     : 1;  /* Bit7  ???? */
        uint16_t TMP_Min_CHG     : 1;  /* Bit8  ???? */
        uint16_t TMP_Min_DSG     : 1;  /* Bit9  ???? */
        uint16_t CHG_OC          : 1;  /* Bit10 ???? */
        uint16_t DSG_OC          : 1;  /* Bit11 ???? */
        uint16_t TMP_Max_MOS     : 1;  /* Bit12 MOS?? */
        uint16_t CHG_MOS_Damaged : 1;  /* Bit13 ??MOS?? */
        uint16_t DSG_MOS_Damaged : 1;  /* Bit14 ??MOS?? */
        uint16_t SC              : 1;  /* Bit15 ?? */
    } bits;
} ErrFlag_t;

/* ================================================================
 * ???? (?? 104~113 ? SRAM, RO)
 * ================================================================ */
typedef struct {
    ErrFlag_t   err_flag;                   /* 104  ???? */
    uint16_t    err_flag_h;                 /* 105  ??????(??) */
    uint16_t    status;                     /* 106  ???(??) */
    int16_t     cell_tmp;                   /* 107  ????   /10,1  ? */
    int16_t     mos_tmp;                    /* 108  MOS??    /10,1  ? */
    uint16_t    soc;                        /* 109  SOC        /10,1  % */
    uint16_t    cell_voltage;               /* 110  ????   mV */
    int16_t     cell_current;               /* 111  ????   /10,1  A */
    uint16_t    out_voltage;                /* 112  ????   /100,1 V */
    int16_t     out_current;                /* 113  ????   /100,1 A */
} BMS_RealtimeData_t;       /* ????: 104,  ? 10 ???? */

/* ================================================================
 * ???? (?? 0~4 ? EEPROM, RO)
 * ================================================================ */
typedef struct {
    uint16_t    res1;                       /* 0  ??????? */
    uint16_t    modbus_cmd_data;            /* 1  Modbus?????? */
    uint16_t    modbus_cmd;                 /* 2  Modbus?? */
    uint16_t    software_version;           /* 3  ????  /100,2  (?208?V2.08) */
    uint16_t    hardware_version;           /* 4  ????  /100,2  (?302?????3,????2) */
} BMS_SystemInfo_t;         /* ????: 0,  ? 5 ???? */

/* ================================================================
 * ID ?? (?? 5~30 ? EEPROM, RO)
 * ================================================================ */
#define CUSTOMER_ID_LEN     20
#define BATTERY_ID_LEN      32

typedef struct {
    uint8_t     customer_id[CUSTOMER_ID_LEN]; /* 5~14  ????+?? (ASCII) */
    uint8_t     battery_id[BATTERY_ID_LEN];   /* 15~30 ??ID (24??) */
} BMS_IDInfo_t;             /* ????: 5,  ? 26 ???? / 52 ?? U8 */

/* ================================================================
 * ????? (?? 31~45 ? EEPROM, RO)
 * ================================================================ */
typedef struct {
    uint16_t    restart_times;              /* 31  ???? */
    uint16_t    sleep_times;                /* 32  ???? */
    uint16_t    power_lost_times;           /* 33  ???? */
    uint16_t    total_run_time;             /* 34  ?????? h */
    uint16_t    err_cell_ov_times;          /* 35  ???????? */
    uint16_t    err_cell_uv_times;          /* 36  ???????? */
    uint16_t    err_pack_ov_times;          /* 37  ???????? */
    uint16_t    err_pack_uv_times;          /* 38  ???????? */
    uint16_t    err_chg_oc_times;           /* 39  ???????? */
    uint16_t    err_dsg_oc_times;           /* 40  ???????? */
    uint16_t    err_chg_ot_times;           /* 41  ???????? */
    uint16_t    err_chg_ut_times;           /* 42  ???????? */
    uint16_t    err_dsg_ot_times;           /* 43  ???????? */
    uint16_t    err_dsg_ut_times;           /* 44  ???????? */
    uint16_t    err_sc_times;               /* 45  ?????? */
} BMS_EventCount_t;         /* ????: 31,  ? 15 ???? */

/* ================================================================
 * ?? & ???? (?? 46~51 ? EEPROM, RW)
 * ================================================================ */
typedef struct {
    uint16_t    ntc_material;               /* 46  NTC??: 1=3930, 2=3435 */
    int16_t     zero_adjust;                /* 47  ??????  -1000~1000 */
    uint16_t    current_adjust;             /* 48  ??????  0~10000 */
    /* 49 ?? */
    uint16_t    cell_material;              /* 50  ????: 1=????, 2=???, 3=?? */
    uint16_t    factory_ah;                 /* 51  ????  /10,1  Ah */
} BMS_CalibConfig_t;        /* ????: 46,  ? 6 ?????? (49???) */

/* ================================================================
 * ?????? (?? 66~73 ? EEPROM, RW)
 * ================================================================ */
typedef struct {
    uint16_t    cell_voltage_max;           /* 66  ??????    mV */
    uint16_t    cell_voltage_max_recover;   /* 67  ??????    mV */
    uint16_t    cell_voltage_min;           /* 68  ??????    mV */
    uint16_t    cell_voltage_min_recover;   /* 69  ??????    mV */
    uint16_t    pack_voltage_max;           /* 70  ???????  /100,2  V */
    uint16_t    pack_voltage_max_recover;   /* 71  ???????  /100,2  V */
    uint16_t    pack_voltage_min;           /* 72  ???????  /100,2  V */
    uint16_t    pack_voltage_min_recover;   /* 73  ???????  /100,2  V */
} BMS_VoltageProtect_t;     /* ????: 66,  ? 8 ???? */

/* ================================================================
 * ?????? (?? 74~83 ? EEPROM, RW)  /10,1  ?
 * ================================================================ */
typedef struct {
    int16_t     pack_tmp_max_chg;           /* 74  ?????? */
    int16_t     pack_tmp_max_chg_recover;   /* 75  ?????? */
    int16_t     pack_tmp_max_dsg;           /* 76  ?????? */
    int16_t     pack_tmp_max_dsg_recover;   /* 77  ?????? */
    int16_t     pack_tmp_min_chg;           /* 78  ?????? */
    int16_t     pack_tmp_min_chg_recover;   /* 79  ?????? */
    int16_t     pack_tmp_min_dsg;           /* 80  ?????? */
    int16_t     pack_tmp_min_dsg_recover;   /* 81  ?????? */
    int16_t     mos_tmp_max;                /* 82  MOS???? */
    int16_t     mos_tmp_max_recover;        /* 83  MOS???? */
} BMS_TempProtect_t;        /* ????: 74,  ? 10 ???? */

/* ================================================================
 * ?????? (?? 86~103 ? EEPROM, RW)
 * ================================================================ */
typedef struct {
    uint16_t    cur;                        /* ????  /10,1  A */
    uint16_t    time_action;                /* ????  /10,1  s */
    uint16_t    time_recover;               /* ????  /10,1  s */
} BMS_CurrentProtectStage_t;

typedef struct {
    BMS_CurrentProtectStage_t   dsg[3];     /* ???? 3? */
    BMS_CurrentProtectStage_t   chg[3];     /* ???? 3? */
} BMS_CurrentProtect_t;     /* ????: 86,  ? 18 ???? */

/* ================================================================
 * ?? BMS ????
 * ================================================================ */
typedef struct {
    BMS_SystemInfo_t        sys;
    BMS_IDInfo_t            id;
    BMS_EventCount_t        event;
    BMS_CalibConfig_t       calib;
    BMS_VoltageProtect_t    volt;
    BMS_TempProtect_t       temp;
    BMS_CurrentProtect_t    curr;
    BMS_RealtimeData_t      rt;
} BMS_Device_t;

/* ================================================================
 * Modbus ??????
 * ================================================================ */

int BMS_ReadRegisters(unsigned char addr, unsigned char count, unsigned int *buf);
int BMS_WriteRegister(unsigned char addr, unsigned int value);
int BMS_WriteRegisters(unsigned char addr, unsigned char count, unsigned int *values);
int BMS_SendCommand(unsigned int cmd, unsigned int Ddata);
int BMS_WriteRegister1(unsigned char cmd, unsigned int addr, unsigned int value,
                       unsigned int *p_data, unsigned int *buf);

int BMS_ReadSystemInfo(BMS_SystemInfo_t *sys);
int BMS_ReadIDInfo(BMS_IDInfo_t *id);
int BMS_ReadEventCount(BMS_EventCount_t *event);
int BMS_ReadCalibConfig(BMS_CalibConfig_t *calib);
int BMS_ReadVoltageProtect(BMS_VoltageProtect_t *volt);
int BMS_ReadTempProtect(BMS_TempProtect_t *temp);
int BMS_ReadCurrentProtect(BMS_CurrentProtect_t *curr);
int BMS_ReadRealtimeData(BMS_RealtimeData_t *rt);
int BMS_ReadAll(BMS_Device_t *dev);

int BMS_WriteCalibConfig(BMS_CalibConfig_t *calib);
int BMS_WriteVoltageProtect(BMS_VoltageProtect_t *volt);
int BMS_WriteTempProtect(BMS_TempProtect_t *temp);
int BMS_WriteCurrentProtect(BMS_CurrentProtect_t *curr);
int BMS_SendSleepCommand(void);

unsigned int BMS_CRC16(unsigned char *Ddata, unsigned char len);

/* ????????? XDATA */
extern unsigned int xdata BMS_RegisterValues[31];

/* ???????? */
extern void DelayMs(unsigned int ms);
extern unsigned long GetTickMs(void);

void BMS_StartRealtimeRead(void);
void BMS_OnRxByte(unsigned char b);
int BMS_PollRealtimeData(void);

extern volatile BMS_RealtimeData_t rt;

#ifdef __cplusplus
}
#endif

#endif  /* __MODBUS_H__ */


