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
#include "delay.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>

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

int bme280_fd = -1;

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
int bme280_read_id(uint8_t *id)
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
int bme280_config(void)
{
    int ret;
    uint8_t data;

    /* 1. 配置湿度过采样 */
    data = BME280_OSRS_H_1;
    ret = i2c_write_reg(bme280_fd, BME280_REG_CTRL_HUM, &data, 1);
    if (ret < 0)
    {
        printf("BME280 配置湿度过采样失败！\n");
        return -1;
    }

    /* 2. 配置温度过采样 */
    data = BME280_OSRS_T_1;
    ret = i2c_write_reg(bme280_fd, BME280_REG_CTRL_MEAS, &data, 1);
    if (ret < 0)
    {
        printf("BME280 配置温度过采样失败！\n");
        return -1;
    }
    
    /* 3. 配置气压过采样 */
    data = BME280_OSRS_P_1;
    ret = i2c_write_reg(bme280_fd, BME280_REG_CONFIG, &data, 1);
    if (ret < 0)
    {
        printf("BME280 配置气压过采样失败！\n");
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

static float bme280_compensate_temperature(int32_t temp_raw)
{ 
    float ret;
    ret = temp_raw / 16384.0 - 25.0;
    return ret;
}

static float bme280_compensate_pressure(int32_t press_raw)
{ 
    float ret;
    ret = press_raw / 1048576.0 - 1.0;
    ret = ret * 25.0 / 16384.0;
    return ret;
}
static float bme280_compensate_humidity(int32_t hum_raw)
{ 
    float ret;
    ret = hum_raw / 16384.0 - 1.0;
    ret = ret * 100.0 / 1024.0;
    return ret;
}