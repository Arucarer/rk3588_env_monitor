/******************************************************************************
 * @file    modbus_master.c
 * @brief   Modbus RTU 主机协议实现
 *
 * @author  李宇坤
 * @date    2026-07
 * @version V1.0
 *
 * @details
 * 本文件实现 Linux 应用层 Modbus RTU 主机通信功能，基于串口完成
 * Modbus 请求帧发送、响应帧接收、CRC 校验及寄存器数据解析。
 *
 * 主要功能包括：
 * 1. 构造 Modbus RTU 请求帧；
 * 2. 调用 UART 接口发送请求数据；
 * 3. 接收并校验从机响应帧；
 * 4. 校验从机地址、功能码、数据长度及 CRC16；
 * 5. 解析保持寄存器和输入寄存器数据；
 * 6. 为上层 RS485 传感器模块提供统一的 Modbus 访问接口。
 *
 ******************************************************************************/

 #include "modbus_master.h"
 #include "crc16.h"
 #include "uart.h"
 
 #include <errno.h>
 #include <stddef.h>
 #include <stdint.h>
 #include <stdio.h>
 #include <string.h>
 #include <unistd.h>
 
 /* Modbus RTU 读取寄存器请求帧长度 */
 #define MODBUS_READ_REQUEST_SIZE          8
 
 /* Modbus RTU 最小响应帧长度 */
 #define MODBUS_MIN_RESPONSE_SIZE          5
 
 /* Modbus RTU 异常响应帧长度 */
 #define MODBUS_EXCEPTION_RESPONSE_SIZE    5
 
 /* 单次最大读取寄存器数量 */
 #define MODBUS_MAX_READ_REGISTERS         125
 
 /* Modbus RTU 最大帧长度 */
 #define MODBUS_RTU_MAX_FRAME_SIZE         256

 /*Modbus RTU 写入寄存器请求帧长度*/
 #define MODBUS_WRITE_REQUEST_SIZE         8  

 /* Modbus RTU 写单个寄存器正常响应帧长度 */
 #define MODBUS_WRITE_RESPONSE_SIZE        8


 /*读取寄存器通用接口*/
static int modbus_master_read_registers(int fd,//串口文件描述符
                                        uint8_t slave_addr,//从站地址
                                        uint8_t function_code,//功能码
                                        uint16_t start_addr,//寄存器起始地址
                                        uint16_t reg_count,//寄存器个数
                                        uint16_t *reg_data)//寄存器数据
{
    int ret;
    uint16_t crc;
    uint16_t recv_crc;//接收的CRC16
    uint16_t calc_crc;//计算出的CRC16
    
    size_t i;
    size_t expect_len;

    uint8_t request[MODBUS_READ_REQUEST_SIZE];
    uint8_t response[MODBUS_RTU_MAX_FRAME_SIZE];
    /* 1. 参数检查 */
    if (fd < 0 ||
        reg_data == NULL ||
        slave_addr == 0 ||
        reg_count == 0 ||
        reg_count > MODBUS_MAX_READ_REGISTERS ||
        (function_code != MODBUS_FUNC_READ_HOLDING_REGISTERS &&
         function_code != MODBUS_FUNC_READ_INPUT_REGISTERS))  {
            return MODBUS_ERR_PARAM;//参数错误
        }

    /* 2. 构造请求帧 */
    request[0] = slave_addr;
    request[1] = function_code;

    request[2] = (uint8_t)(start_addr >> 8);//起始地址最高位
    request[3] = (uint8_t)(start_addr & 0xFF);

    request[4] = (uint8_t)(reg_count >> 8);//寄存器数量高位置
    request[5] = (uint8_t)(reg_count & 0xFF);

    /* 3. 添加CRC */
    crc = crc16_modbus(request, 6);//计算CRC16,7\8位是CRC本身

    /* Modbus RTU规定CRC低字节在前 */
    request[6] = (uint8_t)(crc & 0xFF);//CRC低字节
    request[7] = (uint8_t)((crc >> 8) & 0xFF);
    
    /* 4. 发送请求帧 */
    ret = uart_write(fd, request, MODBUS_READ_REQUEST_SIZE);
    if (ret < 0) {
        return MODBUS_ERR_SEND;
    }
    if (ret != MODBUS_READ_REQUEST_SIZE) {
        return MODBUS_ERR_SEND;
    }
    /* 5. 接收响应帧 */
    expect_len = MODBUS_MIN_RESPONSE_SIZE + reg_count * 2;

    ret = uart_read(fd, response, expect_len);
    if (ret < 0) {
        return MODBUS_ERR_RECV;
    }
    if ((size_t)ret != expect_len) {
        return MODBUS_ERR_RECV;
    }
        
    /* 6. 校验响应帧 从机地址 + 异常功能码 + 异常码 + CRC低 + CRC高 */
    if (response[0] != slave_addr) {
        return MODBUS_ERR_SLAVE_ADDR;
    }
    if (response[1] == (uint8_t)(function_code | 0x80)) {
        /* 此时response[2]才是异常码 */
        return MODBUS_ERR_RESPONSE;
    }
    if (response[1] != function_code)
    {
        return MODBUS_ERR_FUNCTION;
    }
    if (response[2] != (uint8_t)(reg_count * 2)) 
    {
        return MODBUS_ERR_RESPONSE;
    }
    recv_crc = (uint16_t)response[ret - 2] | ((uint16_t)response[ret - 1] << 8);
    calc_crc = crc16_modbus(response, ret - 2);
    if (calc_crc != recv_crc)
    {
        return MODBUS_ERR_CRC;
    }

    /* 7. 解析寄存器数据 */

    for (i = 0; i < reg_count; i++) 
    {
        reg_data[i] = ((uint16_t)response[3 + i * 2] << 8) | (uint16_t)response[4 + i * 2];
    }
    return MODBUS_OK;
}


 /*读取保持寄存器*/
int modbus_master_read_holding_registers(int fd,
                                        uint8_t slave_addr,//从站地址
                                        uint16_t start_addr,//寄存器起始地址
                                        uint16_t reg_count,//寄存器个数
                                        uint16_t *reg_data)//寄存器数据
{
    return modbus_master_read_registers(fd, slave_addr, MODBUS_FUNC_READ_HOLDING_REGISTERS, start_addr, reg_count, reg_data);
}

/*读取输入寄存器*/
int modbus_master_read_input_registers(int fd,
                                       uint8_t slave_addr,
                                       uint16_t start_addr,
                                       uint16_t reg_count,
                                       uint16_t *reg_data)
{
    return modbus_master_read_registers(fd, slave_addr, MODBUS_FUNC_READ_INPUT_REGISTERS, start_addr, reg_count, reg_data);
}

/*写单个保持寄存器*/
int modbus_master_write_single_register(int fd,
                                        uint8_t slave_addr,
                                        uint16_t reg_addr,
                                        uint16_t reg_value)
{
    int ret;
    size_t i;

    uint16_t crc;
    uint16_t recv_crc;
    uint16_t calc_crc;

    uint8_t request[MODBUS_WRITE_REQUEST_SIZE];
    uint8_t response[MODBUS_WRITE_RESPONSE_SIZE];
    
    /* 1. 参数检查 */
    if(fd < 0 || slave_addr == 0)
    {
        return MODBUS_ERR_PARAM;
    }
    /* 2. 构造请求帧 */
    request[0] = slave_addr;
    request[1] = MODBUS_FUNC_WRITE_SINGLE_REGISTER;

    request[2] = (uint8_t)(reg_addr >> 8);
    request[3] = (uint8_t)(reg_addr & 0xFF);

    request[4] = (uint8_t)(reg_value >> 8);
    request[5] = (uint8_t)(reg_value & 0xFF);

    /* 3. 添加CRC */
    crc = crc16_modbus(request, 6);
    request[6] = (uint8_t)(crc & 0xFF);
    request[7] = (uint8_t)((crc >> 8) & 0xFF);

    /* 4. 发送请求帧 */
    ret = uart_write(fd, request, MODBUS_WRITE_REQUEST_SIZE);
    if (ret < 0) {
        return MODBUS_ERR_SEND;
    }
    if (ret != MODBUS_WRITE_REQUEST_SIZE) {
        return MODBUS_ERR_SEND;
    }
    /* 5. 读取响应帧 */
    ret = uart_read(fd, response, MODBUS_WRITE_RESPONSE_SIZE);
    if (ret < 0) {
        return MODBUS_ERR_RECV;
    }
    if ((size_t)ret != MODBUS_WRITE_RESPONSE_SIZE) {
        return MODBUS_ERR_RECV;
    }
    /* 6. 校验响应帧 */
    if (response[0] != slave_addr) {
        return MODBUS_ERR_SLAVE_ADDR;
    }
    if (response[1] == (uint8_t)(MODBUS_FUNC_WRITE_SINGLE_REGISTER | 0x80)) {
        /* 此时response[2]才是异常码 */
        return MODBUS_ERR_RESPONSE;
    }
    if (response[1] != MODBUS_FUNC_WRITE_SINGLE_REGISTER) {
        return MODBUS_ERR_FUNCTION;
    }
    recv_crc = (uint16_t)response[ret - 2] | ((uint16_t)response[ret - 1] << 8);
    calc_crc = crc16_modbus(response, ret - 2);
    if (calc_crc != recv_crc) {
        return MODBUS_ERR_CRC;
    }
    /* 7. 校验响应帧回显的寄存器地址和写入值 */
    for (i = 2; i < 6; i++) {
        if (response[i] != request[i]) {
            return MODBUS_ERR_RESPONSE;
        }
    }
    return MODBUS_OK;

}