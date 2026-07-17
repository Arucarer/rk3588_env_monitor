/******************************************************************************
 * @file    rain_test.c
 * @brief   雨滴传感器测试程序
 *
 * @author  李宇坤
 * @date    2026-07
 * @version V1.0
 *
 * @details
 * 测试内容：
 * 1. ADC模拟量读取；
 * 2. 雨滴湿润程度计算；
 * 3. GPIO数字量检测。
 *
 ******************************************************************************/

 #include <stdio.h>
 #include <unistd.h>
 #include <stdbool.h>
 
 #include "rain.h"
 
 
 int main(void)
 {
     int ret;
 
     uint16_t adc_value;
     float moisture;
 
     bool rain_flag;
 
 
     printf("Rain sensor test start...\n");
 
 
     /*
      * 初始化雨滴传感器
      */
     ret = rain_init();
 
     if(ret < 0)
     {
         printf("rain init failed!\n");
         return -1;
     }
 
 
     while(1)
     {
 
         /*
          * 读取ADC值
          */
         ret = rain_read_adc(&adc_value);
 
         if(ret < 0)
         {
             printf("rain adc read failed!\n");
             sleep(1);
             continue;
         }
 
 
         printf("----------------------\n");
 
         printf("ADC value: %d\n",
                 adc_value);
 
 
 
         /*
          * ADC转换湿润程度
          */
         ret = rain_get_moisture(
                 adc_value,
                 &moisture);
 
         if(ret == 0)
         {
             printf("Moisture: %.2f %%\n",
                     moisture);
         }
 
 
         /*
          * 判断是否下雨
          */
         rain_flag = rain_detect(adc_value);
 
 
         if(rain_flag)
         {
             printf("Rain detected!\n");
         }
         else
         {
             printf("No rain\n");
         }
 
 
         sleep(1);
     }
 
 
 
     rain_deinit();
 
 
     return 0;
 }