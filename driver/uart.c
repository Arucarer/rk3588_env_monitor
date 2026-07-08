/******************************************************************************
 * @file    uart.c
 * @brief   Linux UART 应用层接口实现
 *
 * @author  李宇坤
 * @date    2026-07-08
 * @version V1.0
 *
 * @details
 * 本文件基于 Linux Termios 串口接口实现 UART 通信，
 * 提供串口设备打开、参数配置、数据收发及关闭等功能。
 *
 * 主要用于 RS485、Modbus RTU 及其他 UART 外设通信。
 ******************************************************************************/

 #include "uart.h"

#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <termios.h>

int uart_open(const char *dev_path)
{
    int fd = open(dev_path, O_RDWR | O_NOCTTY | O_NDELAY);//O_RDWR | O_NOCTTY | O_NDELAY是串口设备打开方式
    if (fd < 0) {
        perror("open uart device failed");
        return -1;
    }
    fcntl(fd, F_SETFL, 0); // 设置为阻塞模式
    return fd;
}
int uart_set_config(int fd, const uart_config_t *config)// 设置串口参数
{
    speed_t speed;
    struct termios options;// linux保存串口的结构体，也就是获取当前串口的配置
    int ret;
    if (fd < 0 || config == NULL)
        return -1;
    ret = tcgetattr(fd, &options);//获取当前串口配置
    if (ret != 0) 
    {
        perror("tcgetattr");
        return -1;
    }

    /* 设置波特率 */
    speed = get_baudrate(config->baudrate);

    cfsetispeed(&options, speed);//设置输入波特率
    cfsetospeed(&options, speed);//设置输出波特率

    /* 设置数据位 */
    options.c_cflag &= ~CSIZE; // 清除数据位设置
    switch (config->data_bits) 
    {
        case 5: options.c_cflag |= CS5; break;
        case 6: options.c_cflag |= CS6; break;
        case 7: options.c_cflag |= CS7; break;
        case 8: options.c_cflag |= CS8; break;
        default: options.c_cflag |= CS8; break;// 默认8位数据位
    }
    
    /* 设置停止位 */
    options.c_cflag &= ~CSTOPB;// 清除停止位设置
    switch (config->stop_bits) 
    {
        case 1: options.c_cflag &= ~CSTOPB; break;// CSTOPB表示1位停止位
        case 2: options.c_cflag |= CSTOPB; break;
        default: options.c_cflag &= ~CSTOPB; break;
    }

    /* 设置校验位 */
    options.c_cflag &= ~PARENB; // 清除校验位设置
    switch (config->parity)
    {
    case 'E':
    case 'e':
        options.c_cflag |= PARENB;//PARENB表示启用校验位 |= 表示启用校验位 &~ 表示禁用校验位
        options.c_cflag &= ~PARODD;// PARODD表示奇校验
        break;

    case 'O':
    case 'o':
        options.c_cflag |= PARENB;
        options.c_cflag |= PARODD;
        break;

    case 'N':
    case 'n':
    default:
        options.c_cflag &= ~PARENB;
        options.c_cflag &= ~PARODD;
        break;// 默认无校验位
    }
    options.c_cflag |= CLOCAL;//忽略 Modem 信号，直接使用串口
    options.c_cflag |= CREAD;// 允许读取数据

    tcflush(fd, TCIFLUSH);//清空输入缓冲区
    ret = tcsetattr(fd, TCSANOW, &options);//TCSANOW表示立即执行，TCSAFLUSH表示等待数据传输完毕再执行
    if (ret != 0)
    {
        perror("tcsetattr");
        return -1;
    }
    return 0;
}
int uart_read(int fd, uint8_t *data, size_t len)
{
    int ret;
    if (fd < 0 || data == NULL)
        return -1;

    ret = read(fd, data, len);
    if (ret < 0)
    {
        perror("uart read");
        return -1;
    }
    return ret;
}
int uart_write(int fd, const uint8_t *data, size_t len)
{
    int ret;
    if (fd < 0 || data == NULL)
        return -1;

    ret = write(fd, data, len);
    if (ret < 0)
    {
        perror("uart write");
        return -1;
    }
    return ret;
}
void uart_close(int fd)
{
    if (fd >= 0)
    {
        close(fd);
    }
}

static speed_t get_baudrate(uint32_t baudrate)
{
    switch (baudrate) {
        case 9600: return B9600;
        case 19200: return B19200;
        case 38400: return B38400;
        case 57600: return B57600;
        case 115200: return B115200;
        default: return B9600; // 默认波特率
    }
}