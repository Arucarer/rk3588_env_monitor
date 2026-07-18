/******************************************************************************
 * @file    i2c.h
 * @brief   Linux I2C 应用层接口封装
 *
 * @author  李宇坤
 * @date    2026-07
 * @version V1.0
 *
 * @details
 * 本文件用于封装 Linux 应用层 I2C 访问接口，包括：
 * 1. 打开 I2C 设备节点；
 * 2. 设置 I2C 从设备地址；
 * 3. 读取 I2C 设备寄存器；
 * 4. 写入 I2C 设备寄存器；
 * 5. 关闭 I2C 设备。
 *
 * 主要用于 BME280 等 I2C 传感器的数据采集。
 ******************************************************************************/
#ifndef __I2C_H__
#define __I2C_H__

#include <stdint.h>
#include <stddef.h>

int i2c_open(const char *dev_path);                                     // 打开 I2C 设备节点
int i2c_set_addr(int fd, uint8_t addr);                                 // 设置 I2C 从设备地址
int i2c_read_reg(int fd, uint8_t reg, uint8_t *data, size_t len);       // 读取 I2C 设备寄存器
int i2c_write_reg(int fd, uint8_t reg, const uint8_t *data, size_t len);// 写入 I2C 设备寄存器
void i2c_close(int fd);                                                 // 关闭 I2C 设备

#endif /* __I2C_H__ */
