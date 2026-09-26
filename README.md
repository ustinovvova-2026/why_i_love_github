#include <stdbool.h>
#include <stdint.h>

#include "platform.h"

#ifdef USE_BARO

#include "common/maths.h"
#include "common/utils.h"

#include "drivers/bus.h"
#include "drivers/time.h"
#include "drivers/io.h"

#include "barometer.h"
#include "barometer_dps310.h"

#define DPS310_REG_PROD_ID         0x0D
#define DPS310_REG_BARO_DATA       0x00
#define DPS310_REG_TEMP_DATA       0x03
#define DPS310_REG_CONF_BARO       0x06
#define DPS310_REG_CONF_TEMP       0x07
#define DPS310_REG_CONF_MEAS       0x08
#define DPS310_REG_CONF_CFG        0x09
#define DPS310_REG_INT_STATUS      0x0A
#define DPS310_REG_FIFO_STATUS     0x0B
#define DPS310_REG_RESET           0x0C

#define DPS310_REG_COEF            0x10

#define DPS310_MEAS_CTRL_BARO_RUN  0x01
#define DPS310_MEAS_CTRL_TEMP_RUN  0x02
#define DPS310_MEAS_CTRL_BG_RUN    0x05
#define DPS310_MEAS_CTRL_TG_RUN    0x06

static const int32_t dps310_scale_factors[] = {
    524288, 1572864, 3670016, 8388608, 253952, 519168, 1056768, 2129920
};

typedef struct dps310_calib_s {
    int16_t c0;
    int16_t c1;
    int32_t c00;
    int32_t c10;
    int16_t c01;
    int16_t c11;
    int16_t c20;
    int16_t c21;
    int16_t c30;
} dps310_calib_t;

static dps310_calib_t calib;
static uint8_t tmp_coef_src = 0;

static bool dps310_read_calibration(baroDev_t *baro)
{
    uint8_t buf;
    bool ack = busReadRegisterBuffer(baro->busDev, DPS310_REG_COEF, buf, 18);
    if (!ack) return false;

    calib.c0 = ((buf << 4) | (buf >> 4));
    if (calib.c0 & 0x0800) calib.c0 |= 0xF000;

    calib.c1 = (((buf & 0x0F) << 8) | buf);
    if (calib.c1 & 0x0800) calib.c1 |= 0xF000;

    calib.c00 = ((buf << 12) | (buf << 4) | (buf >> 4));
    if (calib.c00 & 0x080000) calib.c00 |= 0xFFF00000;

    calib.c10 = (((buf & 0x0F) << 16) | (buf << 8) | buf);
    if (calib.c10 & 0x080000) calib.c10 |= 0xFFF00000;

    calib.c01 = ((buf << 8) | buf);
    calib.c11 = ((buf << 8) | buf);
    calib.c20 = ((buf << 8) | buf);
    calib.c21 = ((buf << 8) | buf);
    calib.c30 = ((buf << 8) | buf);

    busReadRegister(baro->busDev, 0x28, &tmp_coef_src);
    tmp_coef_src = (tmp_coef_src >> 7) & 0x01;

    return true;
}

bool dps310Init(baroDev_t *baro)
{
    uint8_t id = 0;
    bool ack = busReadRegister(baro->busDev, DPS310_REG_PROD_ID, &id);
    
    if (!ack) {
        return false; 
    }

    busWriteRegister(baro->busDev, DPS310_REG_RESET, 0x09);
    delay(10);

    if (!dps310_read_calibration(baro)) {
        return false;
    }

    busWriteRegister(baro->busDev, DPS310_REG_CONF_BARO, 0x33);
    busWriteRegister(baro->busDev, DPS310_REG_CONF_TEMP, 0x33 | (tmp_coef_src << 7));
    busWriteRegister(baro->busDev, DPS310_REG_CONF_CFG, 0x0C);
    busWriteRegister(baro->busDev, DPS310_REG_CONF_MEAS, 0x07);

    baro->combinedAPSR = 0;
    return true;
}

bool dps310UtUpdate(baroDev_t *baro)
{
    UNUSED(baro);
    return true;
}

bool dps310UpUpdate(baroDev_t *baro)
{
    UNUSED(baro);
    return true;
}

bool dps310Read(baroDev_t *baro, int32_t *pressure, int32_t *temperature)
{
    uint8_t p_buf;
    uint8_t t_buf;

    if (!busReadRegisterBuffer(baro->busDev, DPS310_REG_BARO_DATA, p_buf, 3)) return false;
    if (!busReadRegisterBuffer(baro->busDev, DPS310_REG_TEMP_DATA, t_buf, 3)) return false;

    int32_t raw_p = ((p_buf << 16) | (p_buf << 8) | p_buf);
    if (raw_p & 0x800000) raw_p |= 0xFF000000;

    int32_t raw_t = ((t_buf << 16) | (t_buf << 8) | t_buf);
    if (raw_t & 0x800000) raw_t |= 0xFF000000;

    double kP = (double)dps310_scale_factors;
    double kT = (double)dps310_scale_factors;

    double raw_p_sc = (double)raw_p / kP;
    double raw_t_sc = (double)raw_t / kT;

    double comp_t = (double)calib.c0 * 0.5 + (double)calib.c1 * raw_t_sc;
    if (temperature) {
        *temperature = (int32_t)(comp_t * 100.0);
    }

    if (pressure) {
        double comp_p = (double)calib.c00 + raw_p_sc * ((double)calib.c10 + raw_p_sc * ((double)calib.c20 + raw_p_sc * (double)calib.c30)) + raw_t_sc * (double)calib.c01 + raw_t_sc * raw_p_sc * ((double)calib.c11 + raw_p_sc * (double)calib.c21);
        *pressure = (int32_t)comp_p;
    }

    return true;
}

#endif
