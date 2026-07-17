/******************************************************************************
 * @file    bme280.c
 * @brief   BME280 温湿度气压传感器应用层实现
 *
 * @author  李宇坤
 * @date    2026-07
 * @version V1.0
 *
 * @details
 * 本文件实现 Linux 平台下 BME280 传感器应用层驱动，
 * 基于 I2C 通信方式完成 BME280 温度、湿度以及气压数据采集。
 *
 * 本模块通过调用底层 i2c.c 提供的 I2C 通信接口，
 * 实现 BME280 设备初始化、寄存器配置、校准参数读取以及
 * 环境数据转换功能。
 * 主要功能包括：
 * 1. BME280设备初始化；
 * 2. BME280芯片ID检测；
 * 3. BME280工作模式配置；
 * 4. 温度、湿度、气压原始数据读取；
 * 5. 基于校准参数的数据补偿计算；
 * 6. 向上层应用提供标准化环境数据接口。
 * 通信接口：
 * I2C
 * 应用场景：
 * RK3588B Linux 智能环境综合监测系统
 ******************************************************************************/

#include "bme280.h"
#include "i2c.h"


#include <stdio.h>
#include <stdint.h>

#define BME280_I2C_ADDR        0x76 // BME280 I2C 地址
#define BME280_CHIP_ID         0x60 // BME280 芯片ID

#define BME280_REG_CHIP_ID     0xD0 // 芯片ID
#define BME280_REG_RESET       0xE0 // 复位
#define BME280_REG_CTRL_HUM    0xF2 // 湿度过采样
#define BME280_REG_STATUS      0xF3 // 状态
#define BME280_REG_CTRL_MEAS   0xF4 // 温度采样倍率、气压采样倍率、工作模式
#define BME280_REG_CONFIG      0xF5 // 待机时间、滤波系数、SPI模式
#define BME280_REG_PRESS_MSB   0xF7 // 气压数据
#define BME280_REG_TEMP_MSB    0xFA // 温度数据
#define BME280_REG_HUM_MSB     0xFD // 湿度数据

#define BME280_OSRS_H_1        0x01 // 湿度过采样
#define BME280_OSRS_T_1        0x01 // 温度过采样
#define BME280_OSRS_P_1        0x01 // 压强过采样
#define BME280_MODE_NORMAL     0x03 // 工作模式

#define BME280_CONFIG_DEFAULT  0x00


typedef struct
{
    /* Temperature */
    uint16_t dig_T1;
    int16_t  dig_T2;
    int16_t  dig_T3;


    /* Pressure */
    uint16_t dig_P1;
    int16_t  dig_P2;
    int16_t  dig_P3;
    int16_t  dig_P4;
    int16_t  dig_P5;
    int16_t  dig_P6;
    int16_t  dig_P7;
    int16_t  dig_P8;
    int16_t  dig_P9;


    /* Humidity */
    uint8_t  dig_H1;
    int16_t  dig_H2;
    uint8_t  dig_H3;
    int16_t  dig_H4;
    int16_t  dig_H5;
    int8_t   dig_H6;

}bme280_calib_data_t;

static int bme280_fd = -1;
static bme280_calib_data_t calib;
static int32_t t_fine;

static int bme280_read_id(uint8_t *id);
static int bme280_read_calibration(void);
static int bme280_config(void);

static float bme280_compensate_temperature(int32_t temp_raw);
static float bme280_compensate_pressure(int32_t press_raw);
static float bme280_compensate_humidity(int32_t hum_raw);

int bme280_init(void)
{
    int ret;
    uint8_t id = 0;
    /* 1. 打开 I2C 设备 */
    bme280_fd = i2c_open("/dev/i2c-1");
    if(bme280_fd < 0)
    {
        printf("BME280 打开设备错误！\n");
        return -1;
    }

    /* 2. 设置BME280 I2C 地址 */
    ret = i2c_set_addr(bme280_fd, BME280_I2C_ADDR);
    if(ret < 0)
    {
        printf("BME280 设置I2C地址错误！\n");
        i2c_close(bme280_fd);
        bme280_fd = -1;
        return -1;
    }

    /* 3. 读取 BME280 芯片ID */
    ret = bme280_read_id(&id);
    if(ret < 0)
    {
        printf("BME280 读取ID错误！\n");
        i2c_close(bme280_fd);
        bme280_fd = -1;
        return -1;
    }

    if (id != BME280_CHIP_ID) {
        printf("BME280 芯片ID错误！\n");
        i2c_close(bme280_fd);
        bme280_fd = -1;
        return -1;
    }

    /* 4. 读取校准参数*/
    ret = bme280_read_calibration();

    if(ret < 0)
    {
        printf("BME280读取校准参数失败！\n");

        i2c_close(bme280_fd);
        bme280_fd = -1;

        return -1;
    }

    /* 4. 配置 BME280 工作模式、过采样、滤波 */
    ret = bme280_config();
    if(ret < 0)
    {
        printf("BME280 配置错误！\n");
        i2c_close(bme280_fd);
        bme280_fd = -1;
        return -1;
    }
    printf("BME280 init done.\n");
    return 0;
}
static int bme280_read_id(uint8_t *id)
{
    int ret;
    if(bme280_fd < 0)
    {
        printf("BME280 未初始化！\n");
        return -1;
    }
    if(id == NULL)
    {
        printf("BME280 读取ID参数错误！\n");
        return -1;
    }

    ret = i2c_read_reg(bme280_fd, BME280_REG_CHIP_ID, id, 1);
    if (ret < 0)
    {
        printf("BME280 读取 CTRL_MEAS 寄存器失败！\n");
        return -1;
    }
    return 0;
}
static int bme280_config(void)
{
    int ret;
    uint8_t data;

    /* 1. 配置湿度过采样 */
    data = BME280_OSRS_H_1;
    ret = i2c_write_reg(bme280_fd, BME280_REG_CTRL_HUM, &data, 1);
    if (ret < 0)
    {
        printf("BME280 配置湿度失败！\n");
        return -1;
    }

    /* 2. 配置温度过采样 */
    data =
        (BME280_OSRS_T_1 << 5)//这个值是温度的配置，0b11100000，111是温度的配置，00是待机时间，00是滤波系数，00是SPI模式
        |
        (BME280_OSRS_P_1 << 2)//这个值是压强配置，0b00000111，000是压强配置，01是待机时间，11是滤波系数，00是SPI模式
        |
        BME280_MODE_NORMAL;//这个值是工作模式，0b00000011，000是待机时间，11是滤波系数，11是SPI模式
    ret = i2c_write_reg(bme280_fd, BME280_REG_CTRL_MEAS, &data, 1);
    if (ret < 0)
    {
        printf("BME280 配置温压模式失败！\n");
        return -1;
    }
    
    /* 3. 配置滤波和待机时间 */
    data = BME280_CONFIG_DEFAULT;
    ret = i2c_write_reg(bme280_fd, BME280_REG_CONFIG, &data, 1);
    if (ret < 0)
    {
        printf("BME280 配置CONFIG失败！\n");
        return -1;
    }

    return 0;
}

int bme280_read_data(bme280_data_t *data)
{
    int ret;
    int32_t press_raw, temp_raw, hum_raw;
    uint8_t buf[8];

    if (bme280_fd < 0)
    {
        printf("BME280 未初始化！\n");
        return -1;
    }
    if (data == NULL)
    {
        printf("BME280 读取数据参数错误！\n");
        return -1;
    }  

    ret = i2c_read_reg(bme280_fd, BME280_REG_PRESS_MSB, buf, 8); //BME280 的数据寄存器是连续排列的,直接读8个数据
    if (ret < 0)
    {
        printf("BME280 读取数据失败！\n");
        return -1;
    }
    
    press_raw = (buf[0] << 12) | (buf[1] << 4) | (buf[2] >> 4);
    temp_raw = (buf[3] << 12) | (buf[4] << 4) | (buf[5] >> 4);
    hum_raw = (buf[6] << 8) | buf[7];

    //处理原始数据
    data->temperature = bme280_compensate_temperature(temp_raw);
    printf("BME280 temperature: %.2f\n", data->temperature);

    data->pressure = bme280_compensate_pressure(press_raw);
    printf("BME280 pressure: %.2f\n", data->pressure);

    data->humidity = bme280_compensate_humidity(hum_raw);
    printf("BME280 humidity: %.2f\n", data->humidity);

    return 0;
}

void bme280_deinit(void)
{
    if(bme280_fd >= 0)
    {
        i2c_close(bme280_fd);
        bme280_fd = -1;
    }
}

static float bme280_compensate_temperature(int32_t adc_T)
{
    float var1;
    float var2;


    var1 = (((float)adc_T)/16384.0 -
            ((float)calib.dig_T1)/1024.0)
            *
            ((float)calib.dig_T2);


    var2 = ((((float)adc_T)/131072.0 -
            ((float)calib.dig_T1)/8192.0)
            *
            (((float)adc_T)/131072.0 -
            ((float)calib.dig_T1)/8192.0))
            *
            ((float)calib.dig_T3);


    t_fine = (int32_t)(var1 + var2);


    return ((float)t_fine) / 5120.0;
}

static float bme280_compensate_pressure(int32_t adc_P)
{
    int64_t var1;
    int64_t var2;
    int64_t p;


    var1 = ((int64_t)t_fine) - 128000;

    var2 = var1 * var1 * calib.dig_P6;

    var2 = var2 +
        ((var1 * calib.dig_P5)<<17);

    var2 = var2 +
        (((int64_t)calib.dig_P4)<<35);


    var1 = ((var1 * var1 * calib.dig_P3)>>8)
          +
          ((var1 * calib.dig_P2)<<12);


    var1 =
    (((((int64_t)1)<<47)+var1))
    *
    calib.dig_P1 >>33;


    if(var1==0)
        return 0;


    p = 1048576-adc_P;


    p = (((p<<31)-var2)*3125)/var1;


    return (p/256.0)/100.0;
}
static float bme280_compensate_humidity(int32_t adc_H)
{
    int32_t v_x1_u32r;


    v_x1_u32r = t_fine - 76800;


    v_x1_u32r =
        (((((adc_H << 14) -
        (((int32_t)calib.dig_H4) << 20) -
        (((int32_t)calib.dig_H5) * v_x1_u32r))
        + 16384) >> 15)
        *
        (((((((v_x1_u32r *
        ((int32_t)calib.dig_H6)) >> 10)
        *
        (((v_x1_u32r *
        ((int32_t)calib.dig_H3)) >> 11)
        + 32768)) >> 10)
        + 2097152)
        *
        ((int32_t)calib.dig_H2)
        + 8192) >> 14));


    v_x1_u32r =
        (v_x1_u32r -
        (((((v_x1_u32r >> 15) *
        (v_x1_u32r >> 15)) >> 7)
        *
        ((int32_t)calib.dig_H1)) >> 4));


    if(v_x1_u32r < 0)
        v_x1_u32r = 0;


    if(v_x1_u32r > 419430400)
        v_x1_u32r = 419430400;


    return (float)(v_x1_u32r >> 12) / 1024.0;
}

static int bme280_read_calibration(void)
{
    int ret;

    uint8_t calib_buf[26];
    uint8_t hum_buf[7];


    /*
     * 1. 读取温度和压力校准参数
     *
     * 地址:
     * 0x88 ~ 0xA1
     *
     * 共26字节
     */
    ret = i2c_read_reg(
            bme280_fd,
            0x88,
            calib_buf,
            26);

    if(ret < 0)
    {
        printf("BME280读取温压校准参数失败！\n");
        return -1;
    }


    /*
     * 温度校准参数
     */

    calib.dig_T1 =
        (uint16_t)(calib_buf[1] << 8 |
                   calib_buf[0]);

    calib.dig_T2 =
        (int16_t)(calib_buf[3] << 8 |
                  calib_buf[2]);

    calib.dig_T3 =
        (int16_t)(calib_buf[5] << 8 |
                  calib_buf[4]);



    /*
     * 压力校准参数
     */

    calib.dig_P1 =
        (uint16_t)(calib_buf[7] << 8 |
                   calib_buf[6]);

    calib.dig_P2 =
        (int16_t)(calib_buf[9] << 8 |
                  calib_buf[8]);

    calib.dig_P3 =
        (int16_t)(calib_buf[11] << 8 |
                  calib_buf[10]);

    calib.dig_P4 =
        (int16_t)(calib_buf[13] << 8 |
                  calib_buf[12]);

    calib.dig_P5 =
        (int16_t)(calib_buf[15] << 8 |
                  calib_buf[14]);

    calib.dig_P6 =
        (int16_t)(calib_buf[17] << 8 |
                  calib_buf[16]);

    calib.dig_P7 =
        (int16_t)(calib_buf[19] << 8 |
                  calib_buf[18]);

    calib.dig_P8 =
        (int16_t)(calib_buf[21] << 8 |
                  calib_buf[20]);

    calib.dig_P9 =
        (int16_t)(calib_buf[23] << 8 |
                  calib_buf[22]);



    /*
     * 湿度校准参数
     *
     * dig_H1 在 0xA1
     */

    ret = i2c_read_reg(
            bme280_fd,
            0xA1,
            &calib.dig_H1,
            1);

    if(ret < 0)
    {
        printf("BME280读取H1校准参数失败！\n");
        return -1;
    }



    /*
     * dig_H2 ~ dig_H6
     *
     * 地址:
     * 0xE1 ~ 0xE7
     */

    ret = i2c_read_reg(
            bme280_fd,
            0xE1,
            hum_buf,
            7);

    if(ret < 0)
    {
        printf("BME280读取湿度校准参数失败！\n");
        return -1;
    }



    calib.dig_H2 =
        (int16_t)(hum_buf[1] << 8 |
                  hum_buf[0]);


    calib.dig_H3 =
        hum_buf[2];



    /*
     * H4/H5 是特殊拼接
     */

    calib.dig_H4 =
        (int16_t)((hum_buf[3] << 4)
        | (hum_buf[4] & 0x0F));


    calib.dig_H5 =
        (int16_t)((hum_buf[5] << 4)
        | (hum_buf[4] >> 4));


    calib.dig_H6 =
        (int8_t)hum_buf[6];



    /*
     * 打印检查
     */

    printf("BME280 calibration loaded:\n");

    printf("T1=%u T2=%d T3=%d\n",
            calib.dig_T1,
            calib.dig_T2,
            calib.dig_T3);


    printf("P1=%u\n",
            calib.dig_P1);


    printf("H1=%u H2=%d H3=%u H4=%d H5=%d H6=%d\n",
            calib.dig_H1,
            calib.dig_H2,
            calib.dig_H3,
            calib.dig_H4,
            calib.dig_H5,
            calib.dig_H6);


    return 0;
}