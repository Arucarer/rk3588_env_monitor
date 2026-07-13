/******************************************************************************
 * @file    modbus_master.h
 * @brief   Modbus RTU 主机协议接口定义
 *
 * @author  李宇坤
 * @date    2026-07
 * @version V1.0
 ******************************************************************************/

 #ifndef __MODBUS_MASTER_H__
 #define __MODBUS_MASTER_H__
 
 #include <stdint.h>
 
 /* 常用功能码 */
 #define MODBUS_FUNC_READ_HOLDING_REGISTERS    0x03
 #define MODBUS_FUNC_READ_INPUT_REGISTERS      0x04
 #define MODBUS_FUNC_WRITE_SINGLE_REGISTER     0x06
 
 /* 返回值 */
 #define MODBUS_OK                              0       //成功
 #define MODBUS_ERR_PARAM                      -1       //参数错误
 #define MODBUS_ERR_SEND                       -2       //发送错误
 #define MODBUS_ERR_RECV                       -3       //接收错误
 #define MODBUS_ERR_TIMEOUT                    -4       //超时
 #define MODBUS_ERR_CRC                        -5       //CRC校验错误
 #define MODBUS_ERR_SLAVE_ADDR                 -6       //从站地址错误
 #define MODBUS_ERR_FUNCTION                   -7       //功能码错误
 #define MODBUS_ERR_RESPONSE                   -8       //响应错误
 
 /*读取保持寄存器*/
 int modbus_master_read_holding_registers(int fd,
                                          uint8_t slave_addr,
                                          uint16_t start_addr,
                                          uint16_t reg_count,
                                          uint16_t *reg_data);
 
 /*读取输入寄存器*/
 int modbus_master_read_input_registers(int fd,
                                        uint8_t slave_addr,
                                        uint16_t start_addr,
                                        uint16_t reg_count,
                                        uint16_t *reg_data);
 
 /*写单个保持寄存器*/
 int modbus_master_write_single_register(int fd,
                                         uint8_t slave_addr,
                                         uint16_t reg_addr,
                                         uint16_t reg_value);
 
 #endif