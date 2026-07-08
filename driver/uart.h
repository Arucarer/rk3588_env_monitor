/******************************************************************************
 * @file    uart.h
 * @brief   Linux UART 应用层接口封装
 *
 * @author  李宇坤
 * @date    2026-07-08
 * @version V1.0
 *
 * @details
 * 本文件用于封装 Linux 应用层 UART 通信接口，包括：
 * 1. 串口设备打开、关闭
 * 2. 串口参数配置（波特率、数据位、停止位、校验位等）
 * 3. 串口数据读取、写入
 *
 * 本模块基于 Linux 标准 Termios 串口接口实现，
 * 为 RS485、Modbus RTU 及其他 UART 外设提供统一访问接口。
 ******************************************************************************/
#ifndef __UART_H__
#define __UART_H__ 

#include <stdint.h>

/* 串口配置结构体 */

typedef struct {
    uint32_t baudrate;        // 波特率
    uint8_t data_bits;         // 数据位
    uint8_t stop_bits;         // 停止位
    char parity;               // 校验位 ('N', 'E', 'O')
} uart_config_t;

int uart_open(const char *dev_path);
int uart_set_config(int fd, const uart_config_t *config);// 设置串口参数
int uart_read(int fd, uint8_t *data, size_t len);
int uart_write(int fd, const uint8_t *data, size_t len);
void uart_close(int fd);
#endif