/******************************************************************************
 * @file    config.h
 * @brief   系统配置参数
 *
 * @author  李宇坤
 * @date    2026-07
 * @version V1.0
 *
 * @details
 * 本文件用于集中管理系统运行时配置参数，包括：
 * 1. Linux设备节点路径；
 * 2. GPIO与ADC硬件资源；
 * 3. I2C总线配置；
 * 4. UART串口配置；
 * 5. MQTT配置；
 * 6. SQLite数据库配置；
 * 7. 系统采样周期等配置。
 *
 * 所有配置统一维护，便于后续修改和移植。
 ******************************************************************************/

 #ifndef __CONFIG_H__
 #define __CONFIG_H__
 
 
 /******************************************************************************
  * #1 Linux设备节点
  ******************************************************************************/
 
 /* I2C设备节点 */
 #define I2C_DEV_PATH                   "/dev/i2c-1"
 
 /* UART设备节点 */
 #define UART_DEV_PATH                  "/dev/ttyS4"
 
 /* ADC原始数据路径
  * 当前为空字符串，实际接入传感器并确认IIO通道后再修改。
  *
  * 示例：
  * /sys/bus/iio/devices/iio:device0/in_voltage4_raw
  */
 #define ADC_SOIL_RAW_PATH              ""
 #define ADC_RAIN_RAW_PATH              ""
 
 
 /******************************************************************************
  * #2 GPIO配置
  ******************************************************************************/
 
 /*
  * GPIO编号目前使用-1作为无效占位值。
  * 实际接线后，根据RK3588B GPIO名称计算Linux GPIO编号并修改。
  *
  * 不建议使用0作为占位值，因为GPIO0本身可能是有效GPIO。
  */
 
 /* 雨滴传感器DO数字输出引脚 */
 #define GPIO_RAIN_DO_PIN               (-1)
 

 /* 雨滴传感器 AO 对应的 ADC raw 文件路径 */
 #define RAIN_ADC_AO_PATH    "/sys/bus/iio/devices/iio:device0/in_voltage4_raw"
 /* 土壤传感器 AO 对应的 ADC raw 文件路径 */
 #define SOIL_ADC_AO_PIN        "/sys/bus/iio/devices/iio:device0/in_voltageX_raw"

 /* RS485收发方向控制引脚
  * 自动流向TTL转RS485模块不需要使用该引脚。
  */
 #define GPIO_RS485_DIR_PIN             (-1)
 
 /* 雨滴模块常见为低电平表示检测到雨滴，后续以实测为准 */
 #define RAIN_DETECTED_LEVEL            0
 #define RAIN_NOT_DETECTED_LEVEL        1
 
 /* RS485方向控制电平，自动流向模块可忽略 */
 #define RS485_TX_ENABLE_LEVEL          1
 #define RS485_RX_ENABLE_LEVEL          0
 
 
 /******************************************************************************
  * #3 ADC配置
  ******************************************************************************/
 
 /* RK3588 SARADC通常为12位 */
 #define ADC_RESOLUTION_BITS            12
 #define ADC_MAX_RAW_VALUE              4095
 
 /* SARADC参考电压，单位：V
  * 具体值应根据实际ADC输入电压域确认。
  */
 #define ADC_REFERENCE_VOLTAGE          1.8f
 
 /* 土壤湿度传感器校准值
  * 后续分别测量完全干燥和充分湿润时的ADC原始值后修改。
  */
 #define SOIL_ADC_DRY_VALUE             3500
 #define SOIL_ADC_WET_VALUE             1000
 
 /* 雨滴传感器模拟量校准值
  * 后续根据无水和有水状态实测修改。
  */
 #define RAIN_ADC_DRY_VALUE             3300
 #define RAIN_ADC_WET_VALUE             800
 #define RAIN_DETECT_THRESHOLD          2500
 
 
 /******************************************************************************
  * #4 I2C配置
  ******************************************************************************/
 
 /* BME280 I2C地址
  * SDO接GND时通常为0x76；
  * SDO接VCC时通常为0x77。
  */
 #define I2C_BME280_ADDR                0x76
 
 /* I2C操作超时时间，单位：ms */
 #define I2C_TIMEOUT_MS                 1000
 
 /* I2C操作失败重试次数 */
 #define I2C_RETRY_COUNT                3
 
 
 /******************************************************************************
  * #5 UART配置
  ******************************************************************************/
 
 /* UART通信参数 */
 #define UART_BAUD_RATE                 9600
 #define UART_DATA_BITS                 8
 #define UART_STOP_BITS                 1
 #define UART_PARITY                    'N'
 #define UART_BUFFER_SIZE               256
 
 /* UART读写超时时间，单位：ms */
 #define UART_TIMEOUT_MS                1000
 
 /* UART操作失败重试次数 */
 #define UART_RETRY_COUNT               3
 
 
 /******************************************************************************
  * #6 Modbus RTU配置
  ******************************************************************************/
 
 /* Modbus从机地址 */
 #define MODBUS_TEMP_HUMI_SLAVE_ADDR    0x01
 
 /* Modbus响应超时时间，单位：ms */
 #define MODBUS_TIMEOUT_MS              500
 
 /* Modbus通信失败重试次数 */
 #define MODBUS_RETRY_COUNT             3
 
 /* Modbus RTU最大帧长度 */
 #define MODBUS_BUFFER_SIZE             256
 
 
 /******************************************************************************
  * #7 MQTT配置
  ******************************************************************************/
 
 #define MQTT_HOST                      "127.0.0.1"
 #define MQTT_PORT                      1883
 #define MQTT_CLIENT_ID                 "rk3588_env_monitor_001"
 #define MQTT_PUB_TOPIC                 "rk3588/env_monitor/data"
 #define MQTT_SUB_TOPIC                 "rk3588/env_monitor/cmd"
 
 /* MQTT连接参数 */
 #define MQTT_KEEPALIVE_SEC             60
 #define MQTT_QOS                       1
 #define MQTT_RETAIN                    0
 #define MQTT_RECONNECT_INTERVAL_SEC    5
 
 
 /******************************************************************************
  * #8 SQLite配置
  ******************************************************************************/
 
 #define SQLITE_DB_PATH                 "./data/env_monitor.db"
 
 /* 数据库存储周期，单位：ms */
 #define SQLITE_SAVE_PERIOD_MS          5000
 
 
 /******************************************************************************
  * #9 日志配置
  ******************************************************************************/
 
 #define LOG_FILE_PATH                  "./log/env_monitor.log"
 #define LOG_BUFFER_SIZE                512
 
 
 /******************************************************************************
  * #10 系统配置
  ******************************************************************************/
 
 /* 传感器采样周期，单位：ms */
 #define SAMPLE_PERIOD_MS               1000
 
 /* 通用传感器读取重试次数 */
 #define SENSOR_RETRY_COUNT             3
 
 /* 传感器离线判定次数 */
 #define SENSOR_OFFLINE_THRESHOLD       5
 
 
 #endif /* __CONFIG_H__ */