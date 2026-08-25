#include "modbus.h"
#include <STC8H.H>
#include "usart.h"
/* ================================================================
 * ???????? XDATA ?
 * ================================================================ */
static unsigned char xdata tx_buf[256];
static unsigned char xdata rx_buf[256];
unsigned int xdata BMS_RegisterValues[31];

static unsigned char xdata rt_rx_buf[32];
static volatile unsigned char rt_rx_cnt = 0;
static volatile bit rt_read_active = 0;
static volatile bit rt_ready = 0;
static volatile unsigned char rt_frame_len = 0;
static unsigned long rt_deadline = 0;

/* ================================================================
 * ???????????????
 * ================================================================ */
static void UART_SendByte(unsigned char byte) {
    S2BUF = byte;
    while ((S2CON & 0x02) == 0);
    S2CON &= (unsigned char)~0x02;
}

static void UART_SendBytes(unsigned char *Ddata, unsigned char len) {
    unsigned char i;
    for (i = 0; i < len; i++) {
        UART2_SendByte(Ddata[i]);
    }
}

static int UART_ReceiveBytes(unsigned char *buf, unsigned char len, unsigned int timeout_ms) {
    unsigned char cnt = 0;
    unsigned long start, now;
    start = GetTickMs();
    while (cnt < len) {
				if (S2CON & 0x01) {
						buf[cnt++] = S2BUF;
						S2CON &= (unsigned char)~0x01;
						start = GetTickMs();
				}
        now = GetTickMs();
        if ((now - start) >= timeout_ms) {
            return -1;
        }
    }
    return 0;
}

static void BMS_ClearRxState(void)
{
    unsigned char dummy;

    while (S2CON & 0x01) {
        dummy = S2BUF;
        S2CON &= (unsigned char)~0x01;
    }

    dummy = dummy;
}

/* ================================================================
 * CRC16 ????????????
 * ================================================================ */
unsigned int BMS_CRC16(unsigned char *Ddata, unsigned char len) {
    unsigned int crc = 0xFFFF;
    unsigned char i, j;
    for (i = 0; i < len; i++) {
        crc ^= Ddata[i];
        for (j = 0; j < 8; j++) {
            if (crc & 0x0001)
                crc = (crc >> 1) ^ 0xA001;
            else
                crc >>= 1;
        }
    }
    return crc;
}

/* ================================================================
 * ?????????
 * ================================================================ */
static int BMS_SendReceive(unsigned char tx_len, unsigned char expected_rx_len) {
    unsigned char actual_rx_len;
    unsigned int crc_rx, crc_calc;
    int ret;

    if (expected_rx_len < 5U) return -6;

    BMS_ClearRxState();

    UART_SendBytes(tx_buf, tx_len);

    ret = UART_ReceiveBytes(rx_buf, 2, BMS_USART_TIMEOUT);
    if (ret != 0) return ret;

    actual_rx_len = (rx_buf[1] & 0x80U) ? 5U : expected_rx_len;

    ret = UART_ReceiveBytes(&rx_buf[2], actual_rx_len - 2, BMS_USART_TIMEOUT);
    if (ret != 0) return ret;

    crc_rx = ((unsigned int)rx_buf[actual_rx_len - 1] << 8) | rx_buf[actual_rx_len - 2];
    crc_calc = BMS_CRC16(rx_buf, actual_rx_len - 2);
    if (crc_rx != crc_calc) return -4;

    if (rx_buf[0] != BMS_SLAVE_ID) return -5;

    if (rx_buf[1] & 0x80U) return -(100 + rx_buf[2]);

    return 0;
}

/* ================================================================
 * ???????
 * ================================================================ */
int BMS_WriteRegister1(unsigned char cmd, unsigned int addr, unsigned int value,
                       unsigned int *p_data, unsigned int *buf) {
    unsigned char idx;
    unsigned int crc;
    unsigned int i;
    unsigned char rx_len;
    int ret;

    if (cmd != 0x03U && cmd != 0x10U) return -7;

    if (cmd == 0x03U) {
        if (value == 0U || value > 125U || buf == 0) return -8;
    } else {
        if (value == 0U || value > 123U || p_data == 0) return -8;
    }

    idx = 0;
    tx_buf[idx++] = BMS_SLAVE_ID;
    tx_buf[idx++] = cmd;
    tx_buf[idx++] = (unsigned char)(addr >> 8);
    tx_buf[idx++] = (unsigned char)(addr & 0xFFU);

    if (cmd == 0x03) {
        tx_buf[idx++] = (unsigned char)(value >> 8);
        tx_buf[idx++] = (unsigned char)(value & 0xFFU);
    } else {
        tx_buf[idx++] = (unsigned char)(value >> 8);
        tx_buf[idx++] = (unsigned char)(value & 0xFFU);
        tx_buf[idx++] = value * 2;
        for (i = 0; i < value; i++) {
            tx_buf[idx++] = (unsigned char)(p_data[i] >> 8);
            tx_buf[idx++] = (unsigned char)(p_data[i] & 0xFF);
        }
    }

    crc = BMS_CRC16(tx_buf, idx);
    tx_buf[idx++] = (unsigned char)(crc & 0xFF);
    tx_buf[idx++] = (unsigned char)(crc >> 8);

    if (cmd == 0x03) {
        if (buf == 0) return -8;
        rx_len = 5 + value * 2;
        ret = BMS_SendReceive(idx, rx_len);
        if (ret != 0) return ret;
        for (i = 0; i < value; i++) {
            buf[i] = ((unsigned int)rx_buf[3 + i * 2] << 8) | rx_buf[4 + i * 2];
        }
        return 0;
    } else {
        return BMS_SendReceive(idx, 8);
    }
}

int BMS_ReadRegisters(unsigned char addr, unsigned char count, unsigned int *buf) {
    unsigned char idx;
    unsigned int crc;
    unsigned char rx_len;
    int ret;
    unsigned char i;

    idx = 0;
    tx_buf[idx++] = BMS_SLAVE_ID;
    tx_buf[idx++] = 0x03;
    tx_buf[idx++] = (unsigned char)(addr >> 8);
    tx_buf[idx++] = (unsigned char)(addr & 0xFFU);
    tx_buf[idx++] = 0x00;
    tx_buf[idx++] = count;

    crc = BMS_CRC16(tx_buf, idx);
    tx_buf[idx++] = (unsigned char)(crc & 0xFF);
    tx_buf[idx++] = (unsigned char)(crc >> 8);

    rx_len = 5 + count * 2;
    ret = BMS_SendReceive(idx, rx_len);
    if (ret != 0) return ret;

    for (i = 0; i < count; i++) {
        buf[i] = ((unsigned int)rx_buf[3 + i * 2] << 8) | rx_buf[4 + i * 2];
    }
    return 0;
}

int BMS_WriteRegister(unsigned char addr, unsigned int value) {
    unsigned char idx;
    unsigned int crc;

    idx = 0;
    tx_buf[idx++] = BMS_SLAVE_ID;
    tx_buf[idx++] = 0x06;
    tx_buf[idx++] = (unsigned char)(addr >> 8);
    tx_buf[idx++] = (unsigned char)(addr & 0xFFU);
    tx_buf[idx++] = (unsigned char)(value >> 8);
    tx_buf[idx++] = (unsigned char)(value & 0xFF);

    crc = BMS_CRC16(tx_buf, idx);
    tx_buf[idx++] = (unsigned char)(crc & 0xFF);
    tx_buf[idx++] = (unsigned char)(crc >> 8);

    return BMS_SendReceive(idx, 8);
}

int BMS_WriteRegisters(unsigned char addr, unsigned char count, unsigned int *values) {
    unsigned char idx;
    unsigned int crc;
    unsigned char i;

    idx = 0;
    tx_buf[idx++] = BMS_SLAVE_ID;
    tx_buf[idx++] = 0x10;
    tx_buf[idx++] = (unsigned char)(addr >> 8);
    tx_buf[idx++] = (unsigned char)(addr & 0xFFU);
    tx_buf[idx++] = 0x00;
    tx_buf[idx++] = count;
    tx_buf[idx++] = count * 2;

    for (i = 0; i < count; i++) {
        tx_buf[idx++] = (unsigned char)(values[i] >> 8);
        tx_buf[idx++] = (unsigned char)(values[i] & 0xFF);
    }

    crc = BMS_CRC16(tx_buf, idx);
    tx_buf[idx++] = (unsigned char)(crc & 0xFF);
    tx_buf[idx++] = (unsigned char)(crc >> 8);

    return BMS_SendReceive(idx, 8);
}

int BMS_SendCommand(unsigned int cmd, unsigned int Ddata) {
    int ret;
    ret = BMS_WriteRegister(1, Ddata);
    if (ret != 0) return ret;
    DelayMs(10);
    return BMS_WriteRegister(2, cmd);
}

/* ================================================================
 * ???? ? ?????????? xdata
 * ================================================================ */
int BMS_ReadSystemInfo(BMS_SystemInfo_t *sys) {
    unsigned int xdata buf[5] = 0;
    int ret1;
    ret1 = BMS_ReadRegisters(0, 5, buf);
    if (ret1 != 0) return ret1;
    sys->res1             = buf[0];
    sys->modbus_cmd_data  = buf[1];
    sys->modbus_cmd       = buf[2];
    sys->software_version = buf[3];
    sys->hardware_version = buf[4];
    return 0;
}

int BMS_ReadIDInfo(BMS_IDInfo_t *id) {
    int ret;
    unsigned char i;
    ret = BMS_ReadRegisters(5, 26, &BMS_RegisterValues[5]);
    if (ret != 0) return ret;
    for (i = 0; i < 26; i++) {
        ((unsigned char *)id)[i * 2]     = (unsigned char)(BMS_RegisterValues[i + 5] & 0xFFU);
        ((unsigned char *)id)[i * 2 + 1] = (unsigned char)(BMS_RegisterValues[i + 5] >> 8);
    }
    return 0;
}

int BMS_ReadEventCount(BMS_EventCount_t *event) {
    unsigned int xdata buf[15];
    int ret;
    ret = BMS_ReadRegisters(31, 15, buf);
    if (ret != 0) return ret;
    event->restart_times       = buf[0];
    event->sleep_times         = buf[1];
    event->power_lost_times    = buf[2];
    event->total_run_time      = buf[3];
    event->err_cell_ov_times   = buf[4];
    event->err_cell_uv_times   = buf[5];
    event->err_pack_ov_times   = buf[6];
    event->err_pack_uv_times   = buf[7];
    event->err_chg_oc_times    = buf[8];
    event->err_dsg_oc_times    = buf[9];
    event->err_chg_ot_times    = buf[10];
    event->err_chg_ut_times    = buf[11];
    event->err_dsg_ot_times    = buf[12];
    event->err_dsg_ut_times    = buf[13];
    event->err_sc_times        = buf[14];
    return 0;
}

int BMS_ReadCalibConfig(BMS_CalibConfig_t *calib) {
    unsigned int xdata buf[3];
    int ret;
    ret = BMS_ReadRegisters(46, 3, buf);
    if (ret != 0) return ret;
    calib->ntc_material    = buf[0];
    calib->zero_adjust     = (signed int)buf[1];
    calib->current_adjust  = buf[2];
    ret = BMS_ReadRegisters(50, 2, buf);
    if (ret != 0) return ret;
    calib->cell_material   = buf[0];
    calib->factory_ah      = buf[1];
    return 0;
}

int BMS_ReadVoltageProtect(BMS_VoltageProtect_t *volt) {
    unsigned int xdata buf[8];
    int ret;
    ret = BMS_ReadRegisters(66, 8, buf);
    if (ret != 0) return ret;
    volt->cell_voltage_max          = buf[0];
    volt->cell_voltage_max_recover  = buf[1];
    volt->cell_voltage_min          = buf[2];
    volt->cell_voltage_min_recover  = buf[3];
    volt->pack_voltage_max          = buf[4];
    volt->pack_voltage_max_recover  = buf[5];
    volt->pack_voltage_min          = buf[6];
    volt->pack_voltage_min_recover  = buf[7];
    return 0;
}

int BMS_ReadTempProtect(BMS_TempProtect_t *temp) {
    unsigned int xdata buf[10];
    int ret;
    ret = BMS_ReadRegisters(74, 10, buf);
    if (ret != 0) return ret;
    temp->pack_tmp_max_chg          = (signed int)buf[0];
    temp->pack_tmp_max_chg_recover  = (signed int)buf[1];
    temp->pack_tmp_max_dsg          = (signed int)buf[2];
    temp->pack_tmp_max_dsg_recover  = (signed int)buf[3];
    temp->pack_tmp_min_chg          = (signed int)buf[4];
    temp->pack_tmp_min_chg_recover  = (signed int)buf[5];
    temp->pack_tmp_min_dsg          = (signed int)buf[6];
    temp->pack_tmp_min_dsg_recover  = (signed int)buf[7];
    temp->mos_tmp_max               = (signed int)buf[8];
    temp->mos_tmp_max_recover       = (signed int)buf[9];
    return 0;
}

int BMS_ReadCurrentProtect(BMS_CurrentProtect_t *curr) {
    unsigned int xdata buf[18];
    int ret;
    unsigned char i;
    ret = BMS_ReadRegisters(86, 18, buf);
    if (ret != 0) return ret;
    for (i = 0; i < 3; i++) {
        curr->dsg[i].cur          = buf[i * 3 + 0];
        curr->dsg[i].time_action  = buf[i * 3 + 1];
        curr->dsg[i].time_recover = buf[i * 3 + 2];
    }
    for (i = 0; i < 3; i++) {
        curr->chg[i].cur          = buf[9 + i * 3 + 0];
        curr->chg[i].time_action  = buf[9 + i * 3 + 1];
        curr->chg[i].time_recover = buf[9 + i * 3 + 2];
    }
    return 0;
}

int BMS_ReadRealtimeData(BMS_RealtimeData_t *rt) {
    unsigned int xdata buf[10];
    int ret;
    ret = BMS_ReadRegisters(104, 10, buf);
    if (ret != 0) return ret;
    rt->err_flag.raw   = buf[0];
    rt->err_flag_h     = buf[1];
    rt->status         = buf[2];
    rt->cell_tmp       = (signed int)buf[3];
    rt->mos_tmp        = (signed int)buf[4];
    rt->soc            = buf[5];
    rt->cell_voltage   = buf[6];
    rt->cell_current   = (signed int)buf[7];
    rt->out_voltage    = buf[8];
    rt->out_current    = (signed int)buf[9];
    return 0;
}

int BMS_ReadAll(BMS_Device_t *dev) {
    int ret;
    ret = BMS_ReadSystemInfo(&dev->sys);    if (ret) return ret;
    ret = BMS_ReadIDInfo(&dev->id);         if (ret) return ret;
    ret = BMS_ReadEventCount(&dev->event);  if (ret) return ret;
    ret = BMS_ReadCalibConfig(&dev->calib); if (ret) return ret;
    ret = BMS_ReadVoltageProtect(&dev->volt); if (ret) return ret;
    ret = BMS_ReadTempProtect(&dev->temp);  if (ret) return ret;
    ret = BMS_ReadCurrentProtect(&dev->curr); if (ret) return ret;
    ret = BMS_ReadRealtimeData(&dev->rt);   if (ret) return ret;
    return 0;
}

/* ================================================================
 * ???? ? ????????? xdata
 * ================================================================ */
int BMS_WriteCalibConfig(BMS_CalibConfig_t *calib) {
    unsigned int xdata buf[3];
    int ret;
    buf[0] = calib->ntc_material;
    buf[1] = (unsigned int)calib->zero_adjust;
    buf[2] = calib->current_adjust;
    ret = BMS_WriteRegisters(46, 3, buf);
    if (ret != 0) return ret;
    buf[0] = calib->cell_material;
    buf[1] = calib->factory_ah;
    return BMS_WriteRegisters(50, 2, buf);
}

int BMS_WriteVoltageProtect(BMS_VoltageProtect_t *volt) {
    unsigned int xdata buf[8];
    buf[0] = volt->cell_voltage_max;
    buf[1] = volt->cell_voltage_max_recover;
    buf[2] = volt->cell_voltage_min;
    buf[3] = volt->cell_voltage_min_recover;
    buf[4] = volt->pack_voltage_max;
    buf[5] = volt->pack_voltage_max_recover;
    buf[6] = volt->pack_voltage_min;
    buf[7] = volt->pack_voltage_min_recover;
    return BMS_WriteRegisters(66, 8, buf);
}

int BMS_WriteTempProtect(BMS_TempProtect_t *temp) {
    unsigned int xdata buf[10];
    buf[0] = (unsigned int)temp->pack_tmp_max_chg;
    buf[1] = (unsigned int)temp->pack_tmp_max_chg_recover;
    buf[2] = (unsigned int)temp->pack_tmp_max_dsg;
    buf[3] = (unsigned int)temp->pack_tmp_max_dsg_recover;
    buf[4] = (unsigned int)temp->pack_tmp_min_chg;
    buf[5] = (unsigned int)temp->pack_tmp_min_chg_recover;
    buf[6] = (unsigned int)temp->pack_tmp_min_dsg;
    buf[7] = (unsigned int)temp->pack_tmp_min_dsg_recover;
    buf[8] = (unsigned int)temp->mos_tmp_max;
    buf[9] = (unsigned int)temp->mos_tmp_max_recover;
    return BMS_WriteRegisters(74, 10, buf);
}

int BMS_WriteCurrentProtect(BMS_CurrentProtect_t *curr) {
    unsigned int xdata buf[18];
    unsigned char i;
    for (i = 0; i < 3; i++) {
        buf[i * 3 + 0] = curr->dsg[i].cur;
        buf[i * 3 + 1] = curr->dsg[i].time_action;
        buf[i * 3 + 2] = curr->dsg[i].time_recover;
    }
    for (i = 0; i < 3; i++) {
        buf[9 + i * 3 + 0] = curr->chg[i].cur;
        buf[9 + i * 3 + 1] = curr->chg[i].time_action;
        buf[9 + i * 3 + 2] = curr->chg[i].time_recover;
    }
    return BMS_WriteRegisters(86, 18, buf);
}

/* ================================================================
 * ????
 * ================================================================ */
int BMS_SendSleepCommand(void) {
    unsigned char idx;
    unsigned int crc;
    int ret;

    ET1 = 0;
    IE2 &= (unsigned char)~0x01;
    rt_read_active = 0;

    ret = BMS_WriteRegister(1U, 0x0000U);
    if (ret == 0) {
        DelayMs(10U);

        idx = 0;
        tx_buf[idx++] = BMS_SLAVE_ID;
        tx_buf[idx++] = 0x06U;
        tx_buf[idx++] = 0x00U;
        tx_buf[idx++] = 0x02U;
        tx_buf[idx++] = 0x10U;
        tx_buf[idx++] = 0x00U;

        crc = BMS_CRC16(tx_buf, idx);
        tx_buf[idx++] = (unsigned char)(crc & 0xFFU);
        tx_buf[idx++] = (unsigned char)(crc >> 8);

        UART_SendBytes(tx_buf, idx);
    }

    ET1 = 1;
    return ret;
}

/* ================================================================
 * Non-blocking realtime read (interrupt-driven)
 * ================================================================ */
static int BMS_ParseRealtimeFrame(unsigned char *buf, unsigned char len)
{
    unsigned int crc_rx, crc_calc;

    if (len < 5U) return -6;
    crc_rx = ((unsigned int)buf[len - 1] << 8) | buf[len - 2];
    crc_calc = BMS_CRC16(buf, len - 2);
    if (crc_rx != crc_calc) return -4;
    if (buf[0] != BMS_SLAVE_ID) return -5;
    if (buf[1] & 0x80U) return -(100 + buf[2]);

    rt.err_flag.raw  = ((unsigned int)buf[3]  << 8) | buf[4];
    rt.err_flag_h    = ((unsigned int)buf[5]  << 8) | buf[6];
    rt.status        = ((unsigned int)buf[7]  << 8) | buf[8];
    rt.cell_tmp      = (signed int)(((unsigned int)buf[9]  << 8) | buf[10]);
    rt.mos_tmp       = (signed int)(((unsigned int)buf[11] << 8) | buf[12]);
    rt.soc           = ((unsigned int)buf[13] << 8) | buf[14];
    rt.cell_voltage  = ((unsigned int)buf[15] << 8) | buf[16];
    rt.cell_current  = (signed int)(((unsigned int)buf[17] << 8) | buf[18]);
    rt.out_voltage   = ((unsigned int)buf[19] << 8) | buf[20];
    rt.out_current   = (signed int)(((unsigned int)buf[21] << 8) | buf[22]);
    return 0;
}

void BMS_StartRealtimeRead(void)
{
    unsigned char idx = 0;
    unsigned int crc;

    if (rt_read_active) {
        if ((GetTickMs() - rt_deadline) < BMS_USART_TIMEOUT) return;
        rt_read_active = 0;
        IE2 &= (unsigned char)~0x01;
    }

    rt_read_active = 1;
    rt_ready = 0;

    tx_buf[idx++] = BMS_SLAVE_ID;
    tx_buf[idx++] = 0x03;
    tx_buf[idx++] = 0x00;
    tx_buf[idx++] = 104;
    tx_buf[idx++] = 0x00;
    tx_buf[idx++] = 10;
    crc = BMS_CRC16(tx_buf, idx);
    tx_buf[idx++] = (unsigned char)(crc & 0xFF);
    tx_buf[idx++] = (unsigned char)(crc >> 8);

    BMS_ClearRxState();
    rt_rx_cnt = 0;
    rt_deadline = GetTickMs();

    UART_SendBytes(tx_buf, idx);
    IE2 |= 0x01;
}

void BMS_OnRxByte(unsigned char b)
{
    unsigned char expected;

    if (!rt_read_active) return;

    if (rt_rx_cnt < (unsigned char)sizeof(rt_rx_buf)) {
        rt_rx_buf[rt_rx_cnt++] = b;
    } else {
        rt_read_active = 0;
        rt_rx_cnt = 0;
        IE2 &= (unsigned char)~0x01;
        return;
    }

    if (rt_rx_cnt < 2) return;

    if (rt_rx_buf[1] & 0x80U) {
        expected = 5;
    } else {
        if (rt_rx_cnt < 3) return;
        expected = 5 + rt_rx_buf[2];
        if (expected > (unsigned char)sizeof(rt_rx_buf)) {
            rt_read_active = 0;
            rt_rx_cnt = 0;
            IE2 &= (unsigned char)~0x01;
            return;
        }
    }

    if (rt_rx_cnt >= expected) {
        rt_read_active = 0;
        IE2 &= (unsigned char)~0x01;
        rt_frame_len = expected;
        rt_ready = 1;
    }
}

int BMS_PollRealtimeData(void)
{
    if (rt_ready) {
        rt_ready = 0;
        if (BMS_ParseRealtimeFrame(rt_rx_buf, rt_frame_len) == 0) {
            return 1;
        }
        return -1;
    }
    if (rt_read_active && (GetTickMs() - rt_deadline) >= BMS_USART_TIMEOUT) {
        rt_read_active = 0;
        IE2 &= (unsigned char)~0x01;
        return -1;
    }
    return 0;
}



