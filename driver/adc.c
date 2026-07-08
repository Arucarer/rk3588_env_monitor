/******************************************************************************
 * @file    adc.c
 * @brief   ADC 模块应用层实现
 *
 * @author  李宇坤
 * @date    2026-07
 * @version V1.0
 *
 * @details
 * 本文件实现 Linux IIO（Industrial I/O）ADC 设备的应用层访问接口，
 * 通过读取 IIO sysfs 节点获取 ADC 原始采样值及电压值，并提供
 * ADC 数据转换等功能，为上层传感器模块提供统一的 ADC 数据访问接口。
 *
 * 主要功能包括：
 * 1. 读取 ADC 原始采样值（Raw）；
 * 2. 读取 ADC 电压值（Voltage）；
 * 3. 提供 ADC 数值转换接口；
 * 4. 为土壤湿度、雨滴等模拟量传感器提供统一访问接口。
 ******************************************************************************/
 #include "adc.h"

 int adc_open(const char *dev_path)
 {
    int fd;
    fd = open(dev_path, O_RDONLY);
    return fd;
 }
 int adc_read_raw(const char *path, int *raw)
 {
    int fd;
    int ret;
    char buf[32];

    if (path == NULL || raw == NULL)
        return -1;

    fd = adc_open(path);
    if (fd < 0)
    {
        close(fd);
        return -1;
    }
    ret = read(fd, buf, sizeof(buf) - 1);
    if (ret <= 0)
    {
        close(fd);
        return -1;
    }

    buf[ret] = '\0';
    *raw = atoi(buf);
    close(fd);
    return 0;
 }

 int adc_read_voltage(const char *raw_path,
    const char *scale_path,
    float *voltage);// 读取 ADC 电压值

int adc_raw_to_percent(int raw,
      int dry_raw,
      int wet_raw);// 将 ADC 原始采样值转换为百分比

