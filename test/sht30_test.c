/******************************************************************************
 * @file    sht30_test.c
 * @brief   RS485 SHT30 温湿度传感器测试程序
 *
 * @author  李宇坤
 * @date    2026-07
 * @version V1.0
 *
 * @details
 * 测试内容：
 * 1. UART通信；
 * 2. Modbus RTU通信；
 * 3. SHT30温湿度读取。
 *
 ******************************************************************************/

 #include <stdio.h>
 #include <unistd.h>
 
 #include "sht30_rs485.h"
 
 
 int main(void)
 {
     int ret;
 
     rs485_sht30_data_t data;
 
 
     printf("RS485 SHT30 test start...\n");
 
 
     /*
      * 初始化SHT30
      */
     ret = sht30_rs485_init();
 
     if(ret < 0)
     {
         printf("SHT30 init failed!\n");
         return -1;
     }
 
 
     while(1)
     {
 
         /*
          * 读取温湿度
          */
         ret = sht30_rs485_read_data(&data);
 
 
         if(ret < 0)
         {
             printf("SHT30 read failed!\n");
 
             sleep(1);
             continue;
         }
 
 
         printf("----------------------\n");
 
 
         printf("Temperature: %.1f C\n",
                 data.temperature);
 
 
         printf("Humidity: %.1f %%RH\n",
                 data.humidity);
 
 
         printf("----------------------\n");
 
 
         sleep(1);
     }
 
 
 
     sht30_rs485_deinit();
 
 
     return 0;
 }