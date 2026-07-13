/******************************************************************************
 * @file    crc16.c
 * @brief   Modbus RTU CRC16校验实现
 *
 * @author  李宇坤
 * @date    2026-07
 * @version V1.0
 ******************************************************************************/

 #include "crc16.h"

 uint16_t crc16_modbus(const uint8_t *data, size_t len)
 {
    uint16_t crc;
    size_t i;
    int bit;
    if(data == NULL || len == 0)
    {
        return 0;
    }
    crc = 0xFFFF;//这是Modbus RTU 规定的初始值
    for(i = 0; i < len; i++)
    {
        crc ^= data[i];
        for(bit = 0; bit < 8; bit++)
        {
            if(crc & 0x0001)//判断当前 crc 的最低位是否为 1
            {
                crc >>= 1;
                crc ^= 0xA001;
            }
            else
            {
                crc >>= 1;
            }
        }
    }
 }