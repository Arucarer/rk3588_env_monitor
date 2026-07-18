/******************************************************************************
 * @file    soil_test.c
 * @brief   土壤湿度传感器测试程序
 *
 * @author  李宇坤
 * @date    2026-07
 * @version V1.0
 *
 * @details
 * 测试内容：
 * 1. ADC是否正常读取；
 * 2. 获取土壤传感器原始值；
 * 3. 输出土壤湿度百分比。
 *
 ******************************************************************************/

 #include <stdio.h>
 #include <unistd.h>
 
 #include "soil.h"
 
 
 int main(void)
 {
     int ret;
 
     uint16_t adc_value;
     printf("Soil sensor test start...\n");
 
 
     /*
      * 1. 初始化土壤传感器
      */
     ret = soil_init();
 
     if(ret < 0)
     {
         printf("soil init failed!\n");
         return -1;
     }
 
 
     /*
      * 2. 循环读取
      */
     while(1)
     {
 
         ret = soil_read_adc(&adc_value);
 
         if(ret < 0)
         {
             printf("soil adc read failed!\n");
             sleep(1);
             continue;
         }
 
 
         printf("----------------------\n");
 
         printf("ADC raw value: %d\n",
                 adc_value);
 
 
 
         /*
          * 如果已经完成标定
          * 再打开这个
          */
         /*
         ret = soil_get_moisture(
                 adc_value,
                 &moisture);
 
         if(ret == 0)
         {
             printf(
             "Soil moisture: %.2f %%\n",
             moisture);
         }
         */
 
 
         sleep(1);
     }
 
 
 
     soil_deinit();
 
 
     return 0;
 }
