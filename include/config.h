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
 * 2. I2C总线配置；
 * 3. UART串口配置；
 * 4. MQTT配置；
 * 5. SQLite数据库配置；
 * 6. 系统采样周期等配置。
 *
 * 所有配置统一维护，便于后续修改和移植。
 ******************************************************************************/

#ifndef __CONFIG_H__
#define __CONFIG_H__


/* #1.Linux 设备节点 */

/* I2C */
#define I2C_DEV_PATH                "/dev/i2c-1"

/* UART */
#define UART_DEV_PATH               "/dev/ttyS4"

/* ADC路径 */
#define ADC_SOIL_RAW_PATH           ""                          // 土壤湿度传感器 ADC 原始数据路径
#define ADC_RAIN_RAW_PATH           ""                          // 雨滴传感器 ADC 原始数据路径

/* GPIO：先用编号占位，后面按 RK3588B 实际引脚修改 */
#define GPIO_RAIN_PIN               0                           // 雨滴传感器引脚

/* #2.I2C 配置 */
#define I2C_BME280_ADDR             0x76                        // BME280传感器地址
#define I2C_TIMEOUT_MS              1000                        // I2C超时时间

/* #3.UART 配置*/
#define UART_BAUD_RATE              9600                        // UART波特率
#define UART_DATA_BITS              8                           // 数据位
#define UART_STOP_BITS              1                           // 停止位
#define UART_PARITY                 'N'                         // 校验位
#define UART_BUFFER_SIZE            256                         // 缓存区大小


/* #4.MQTT 配置 */
#define MQTT_HOST                   "127.0.0.1"                 // MQTT服务地址
#define MQTT_PORT                   1883                        // MQTT服务端口
#define MQTT_CLIENT_ID              "rk3588_env_monitor_001"    // MQTT客户端ID
#define MQTT_PUB_TOPIC              "rk3588/env_monitor/data"   // MQTT发布主题
#define MQTT_SUB_TOPIC              "rk3588/env_monitor/cmd"    // MQTT订阅主题

/* #5.SQLite 配置 */
#define SQLITE_DB_PATH              "./data/env_monitor.db"     // SQLite数据库文件路径

/* #6.系统配置*/
#define SAMPLE_PERIOD_MS        1000

#endif 