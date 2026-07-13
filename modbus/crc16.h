/******************************************************************************
 * @file    crc16.h
 * @brief   Modbus RTU CRC16校验接口定义
 *
 * @author  李宇坤
 * @date    2026-07
 * @version V1.0
 ******************************************************************************/

 #ifndef __CRC16_H__
#define __CRC16_H__

#include <stddef.h>
#include <stdint.h>

/**
 * @brief 计算Modbus RTU CRC16校验值
 *
 * @param data 待校验数据缓冲区
 * @param len  数据长度
 *
 * @return CRC16校验值
 */
uint16_t crc16_modbus(const uint8_t *data, size_t len);

#endif