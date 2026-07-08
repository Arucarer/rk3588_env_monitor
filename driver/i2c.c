/******************************************************************************
 * @file    i2c.c
 * @brief   Linux I2C 应用层接口封装实现
 *
 * @author  李宇坤
 * @date    2026-07
 * @version V1.0
 ******************************************************************************/

 #include "i2c.h"
 #include <stdio.h>
 #include <string.h>
 #include <unistd.h>
 #include <fcntl.h>
 #include <sys/ioctl.h>
 #include <linux/i2c-dev.h>
 #include <errno.h>
 
 int i2c_open(const char *dev_path)
 {
    int ret = open(dev_path, O_RDWR);
    if(ret < 0)
    {
        perror("open i2c device failed");
        return -1;
    }
    return ret;
 }      
 
 int i2c_set_addr(int fd, uint8_t addr)
 {
    int ret = ioctl(fd, I2C_SLAVE, addr);//I2C_SLAVE是一个宏定义，表示设置I2C从设备地址的操作
    if(ret < 0)
    {
        perror("set i2c slave address failed");
        return -1;
    }
    return 0;
 }  

 int i2c_read_reg(int fd, uint8_t reg, uint8_t *data, size_t len)
 {
    int ret = write(fd, &reg, 1);//告诉I2C设备要读取的寄存器地址
    if(ret != 1)
    {
        perror("write i2c register failed");
        return -1;
    }
    ret = read(fd, data, len);//从I2C设备读取数据
    if(ret != len)
    {
        perror("read i2c data failed");
        return -1;
    }
    return 0;
 } 

 int i2c_write_reg(int fd, uint8_t reg, const uint8_t *data, size_t len)
 {
    uint8_t buf[len + 1];//创建一个缓冲区，用于存储寄存器地址和数据
    int ret = 0;
    
    if (fd < 0 || data == NULL || len == 0) {
        return -1;
    }
    buf[0] = reg; //寄存器地址
    memcpy(buf + 1, data, len); //这个函数将数据复制到缓冲区中，偏移1字节，因为第一个字节是寄存器地址
    ret = write(fd, buf, len + 1); //写入fd设备，包含寄存器和数据
    if (ret != len + 1) {
        perror("write i2c data failed");
        return -1;
    }
    return 0;
 }

 void i2c_close(int fd)
 {
    close(fd);
    return;
 }                
