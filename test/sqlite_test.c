/******************************************************************************
 * @file    sqlite_test.c
 * @brief   SQLite Manager 模块完整功能测试
 *
 * @author  李宇坤
 * @date    2026-08
 * @version V1.0
 ******************************************************************************/

 #include <stdio.h>
 #include <string.h>
 #include <time.h>
 
 #include "sqlite_manager.h"
 
 /* 测试数据库路径 */
 #define TEST_DB_PATH "data/test_env_monitor.db"
 
 /* ==================== 辅助函数 ==================== */
 
 /* 打印测试结果 */
 static void print_result(const char *test_name, sqlite_mgr_result_t ret)
 {
     if (ret == SQLITE_MGR_OK) {
         printf("[PASS] %s\n", test_name);
     } else if (ret == SQLITE_MGR_NO_DATA) {
         printf("[NO DATA] %s\n", test_name);
     } else {
         printf("[FAIL] %s, ret = %d\n", test_name, ret);
     }
 }
 
 
 /* ==================== 传感器数据测试 ==================== */
 
 static void test_sensor_data(void)
 {
     sqlite_mgr_result_t ret;
     sensor_data_t data;
     sensor_data_t latest;
     sensor_data_t data_array[10];
 
     int count = 0;
     int actual_count = 0;
     uint64_t now = (uint64_t)time(NULL);
 
     printf("\n");
     printf("========================================\n");
     printf("        测试传感器历史数据\n");
     printf("========================================\n");
 
     /* 1. 构造第一条测试数据 */
     memset(&data, 0, sizeof(data));
 
     data.timestamp = now;
     data.bme_temperature = 25.50f;
     data.bme_humidity = 60.20f;
     data.temperature = 26.10f;
     data.humidity = 61.30f;
     data.pressure = 1008.50f;
     data.soil_humidity = 42.50f;
     data.rain_detected = false;
     data.rainfall = 0.00f;
     data.valid = true;
     data.is_abnormal = false;
 
     ret = sqlite_manager_insert(&data);
     print_result("插入第1条传感器数据", ret);
 
     /* 2. 构造第二条测试数据 */
     data.timestamp = now + 1;
     data.bme_temperature = 26.20f;
     data.bme_humidity = 62.10f;
     data.temperature = 27.00f;
     data.humidity = 63.50f;
     data.pressure = 1007.80f;
     data.soil_humidity = 35.20f;
     data.rain_detected = true;
     data.rainfall = 1.50f;
     data.valid = true;
     data.is_abnormal = true;
 
     ret = sqlite_manager_insert(&data);
     print_result("插入第2条传感器数据", ret);
 
     /* 3. 查询最新一条数据 */
     memset(&latest, 0, sizeof(latest));
 
     ret = sqlite_manager_query_latest(&latest);
     print_result("查询最新传感器数据", ret);
 
     if (ret == SQLITE_MGR_OK) {
         printf("最新数据:\n");
         printf("  timestamp       = %llu\n", (unsigned long long)latest.timestamp);
         printf("  BME temperature = %.2f C\n", latest.bme_temperature);
         printf("  BME humidity    = %.2f %%\n", latest.bme_humidity);
         printf("  RS485 temp      = %.2f C\n", latest.temperature);
         printf("  RS485 humidity  = %.2f %%\n", latest.humidity);
         printf("  pressure        = %.2f hPa\n", latest.pressure);
         printf("  soil humidity   = %.2f %%\n", latest.soil_humidity);
         printf("  rain detected   = %d\n", latest.rain_detected);
         printf("  rainfall        = %.2f\n", latest.rainfall);
         printf("  valid           = %d\n", latest.valid);
         printf("  is_abnormal     = %d\n", latest.is_abnormal);
     }
 
     /* 4. 查询历史数据总条数 */
     ret = sqlite_manager_count_sensor(&count);
     print_result("查询传感器数据总条数", ret);
 
     if (ret == SQLITE_MGR_OK) {
         printf("当前传感器数据总数 = %d\n", count);
     }
 
     /* 5. 按时间范围查询 */
     memset(data_array, 0, sizeof(data_array));
 
     ret = sqlite_manager_query_range(
         now,
         now + 10,
         data_array,
         10,
         &actual_count
     );
 
     print_result("按时间范围查询传感器数据", ret);
 
     if (ret == SQLITE_MGR_OK) {
         printf("共查询到 %d 条数据:\n", actual_count);
 
         for (int i = 0; i < actual_count; i++) {
             printf("  [%d] time=%llu, BME_T=%.2f, BME_H=%.2f, abnormal=%d\n",
                    i,
                    (unsigned long long)data_array[i].timestamp,
                    data_array[i].bme_temperature,
                    data_array[i].bme_humidity,
                    data_array[i].is_abnormal);
         }
     }
 
     /* 6. 删除当前测试时间以前的数据 */
     ret = sqlite_manager_delete_sensor_before(now + 2);
     print_result("删除指定时间以前的传感器数据", ret);
 
     /* 7. 再次统计数量 */
     ret = sqlite_manager_count_sensor(&count);
     print_result("删除后重新统计传感器数据", ret);
 
     if (ret == SQLITE_MGR_OK) {
         printf("删除后传感器数据总数 = %d\n", count);
     }
 }
 
 
 /* ==================== 设备配置测试 ==================== */
 
 static void test_device_config(void)
 {
     sqlite_mgr_result_t ret;
     device_config_t config;
     device_config_t loaded_config;
 
     printf("\n");
     printf("========================================\n");
     printf("          测试设备参数\n");
     printf("========================================\n");
 
     /* 1. 构建设备配置 */
     memset(&config, 0, sizeof(config));
 
     snprintf(config.device_id, sizeof(config.device_id), "%s", "RK3588-ENV-001");
     snprintf(config.device_name, sizeof(config.device_name), "%s", "RK3588环境监测设备");
 
     config.sample_period_ms = 1000;
     config.sqlite_save_period_ms = 5000;
     config.mqtt_upload_period_ms = 10000;
 
     config.temperature_high = 35.0f;
     config.temperature_low = 0.0f;
 
     config.humidity_high = 90.0f;
     config.humidity_low = 20.0f;
 
     config.pressure_high = 1100.0f;
     config.pressure_low = 900.0f;
 
     config.rs485_temperature_high = 35.0f;
     config.rs485_temperature_low = 0.0f;
 
     config.rs485_humidity_high = 90.0f;
     config.rs485_humidity_low = 20.0f;
 
     config.soil_humidity_low = 25.0f;
 
     config.sqlite_enabled = true;
     config.mqtt_enabled = true;
 
     config.updated_at = (uint64_t)time(NULL);
 
     /* 2. 保存配置 */
     ret = sqlite_manager_save_config(&config);
     print_result("保存设备配置", ret);
 
     /* 3. 加载最新配置 */
     memset(&loaded_config, 0, sizeof(loaded_config));
 
     ret = sqlite_manager_load_config(&loaded_config);
     print_result("加载最新设备配置", ret);
 
     if (ret == SQLITE_MGR_OK) {
         printf("加载到的设备配置:\n");
         printf("  device_id              = %s\n", loaded_config.device_id);
         printf("  device_name            = %s\n", loaded_config.device_name);
         printf("  sample_period_ms       = %d\n", loaded_config.sample_period_ms);
         printf("  sqlite_save_period_ms  = %d\n", loaded_config.sqlite_save_period_ms);
         printf("  mqtt_upload_period_ms  = %d\n", loaded_config.mqtt_upload_period_ms);
 
         printf("  temperature_high       = %.2f\n", loaded_config.temperature_high);
         printf("  temperature_low        = %.2f\n", loaded_config.temperature_low);
 
         printf("  humidity_high          = %.2f\n", loaded_config.humidity_high);
         printf("  humidity_low           = %.2f\n", loaded_config.humidity_low);
 
         printf("  pressure_high          = %.2f\n", loaded_config.pressure_high);
         printf("  pressure_low           = %.2f\n", loaded_config.pressure_low);
 
         printf("  RS485 temperature high = %.2f\n", loaded_config.rs485_temperature_high);
         printf("  RS485 temperature low  = %.2f\n", loaded_config.rs485_temperature_low);
 
         printf("  RS485 humidity high    = %.2f\n", loaded_config.rs485_humidity_high);
         printf("  RS485 humidity low     = %.2f\n", loaded_config.rs485_humidity_low);
 
         printf("  soil_humidity_low      = %.2f\n", loaded_config.soil_humidity_low);
 
         printf("  sqlite_enabled         = %d\n", loaded_config.sqlite_enabled);
         printf("  mqtt_enabled           = %d\n", loaded_config.mqtt_enabled);
 
         printf("  updated_at             = %llu\n",
                (unsigned long long)loaded_config.updated_at);
     }
 }
 
 
 /* ==================== 告警测试 ==================== */
 
 static void test_alarm(void)
 {
     sqlite_mgr_result_t ret;
     alarm_record_t alarm;
     alarm_record_t alarm_array[10];
 
     int actual_count = 0;
     int alarm_id = 0;
 
     uint64_t now = (uint64_t)time(NULL);
 
     printf("\n");
     printf("========================================\n");
     printf("          测试告警记录\n");
     printf("========================================\n");
 
     /* 1. 构造第一条告警 */
     memset(&alarm, 0, sizeof(alarm));
 
     alarm.timestamp = now;
 
     snprintf(alarm.sensor_name, sizeof(alarm.sensor_name), "%s", "BME280");
     snprintf(alarm.parameter, sizeof(alarm.parameter), "%s", "temperature");
     snprintf(alarm.alarm_level, sizeof(alarm.alarm_level), "%s", "WARNING");
     snprintf(alarm.message, sizeof(alarm.message), "%s", "温度超过上限");
 
     alarm.current_value = 38.50f;
     alarm.threshold_value = 35.00f;
     alarm.handled = false;
     alarm.handled_time = 0;
 
     ret = sqlite_manager_insert_alarm(&alarm);
     print_result("插入第1条告警记录", ret);
 
     /* 2. 构造第二条告警 */
     alarm.timestamp = now + 1;
 
     snprintf(alarm.sensor_name, sizeof(alarm.sensor_name), "%s", "SOIL");
     snprintf(alarm.parameter, sizeof(alarm.parameter), "%s", "soil_humidity");
     snprintf(alarm.alarm_level, sizeof(alarm.alarm_level), "%s", "ERROR");
     snprintf(alarm.message, sizeof(alarm.message), "%s", "土壤湿度过低");
 
     alarm.current_value = 15.0f;
     alarm.threshold_value = 25.0f;
     alarm.handled = false;
     alarm.handled_time = 0;
 
     ret = sqlite_manager_insert_alarm(&alarm);
     print_result("插入第2条告警记录", ret);
 
     /* 3. 查询告警 */
     memset(alarm_array, 0, sizeof(alarm_array));
 
     ret = sqlite_manager_query_alarm_range(
         now,
         now + 10,
         alarm_array,
         10,
         &actual_count
     );
 
     print_result("按时间范围查询告警", ret);
 
     if (ret == SQLITE_MGR_OK) {
         printf("共查询到 %d 条告警:\n", actual_count);
 
         for (int i = 0; i < actual_count; i++) {
             printf("  [%d] id=%d, sensor=%s, parameter=%s\n",
                    i,
                    alarm_array[i].id,
                    alarm_array[i].sensor_name,
                    alarm_array[i].parameter);
 
             printf("      level=%s, current=%.2f, threshold=%.2f\n",
                    alarm_array[i].alarm_level,
                    alarm_array[i].current_value,
                    alarm_array[i].threshold_value);
 
             printf("      message=%s, handled=%d, handled_time=%llu\n",
                    alarm_array[i].message,
                    alarm_array[i].handled,
                    (unsigned long long)alarm_array[i].handled_time);
         }
 
         /* 保存第一条告警 ID */
         if (actual_count > 0) {
             alarm_id = alarm_array[0].id;
         }
     }
 
     /* 4. 标记第一条告警为已处理 */
     if (alarm_id > 0) {
         ret = sqlite_manager_mark_alarm_handled(
             alarm_id,
             (uint64_t)time(NULL)
         );
 
         print_result("标记告警为已处理", ret);
     }
 
     /* 5. 再查询一次，检查 handled 是否变成 1 */
     memset(alarm_array, 0, sizeof(alarm_array));
     actual_count = 0;
 
     ret = sqlite_manager_query_alarm_range(
         now,
         now + 10,
         alarm_array,
         10,
         &actual_count
     );
 
     print_result("重新查询告警处理状态", ret);
 
     if (ret == SQLITE_MGR_OK) {
         for (int i = 0; i < actual_count; i++) {
             printf("  id=%d, handled=%d, handled_time=%llu\n",
                    alarm_array[i].id,
                    alarm_array[i].handled,
                    (unsigned long long)alarm_array[i].handled_time);
         }
     }
 
     /* 6. 删除测试告警 */
     ret = sqlite_manager_delete_alarm_before(now + 2);
     print_result("删除指定时间以前的告警记录", ret);
 }
 
 
 /* ==================== 系统日志测试 ==================== */
 
 static void test_system_log(void)
 {
     sqlite_mgr_result_t ret;
     system_log_t log;
     system_log_t log_array[10];
 
     int actual_count = 0;
     uint64_t now = (uint64_t)time(NULL);
 
     printf("\n");
     printf("========================================\n");
     printf("          测试系统日志\n");
     printf("========================================\n");
 
     /* 1. 插入 INFO 日志 */
     memset(&log, 0, sizeof(log));
 
     log.timestamp = now;
 
     snprintf(log.log_level, sizeof(log.log_level), "%s", "INFO");
     snprintf(log.module, sizeof(log.module), "%s", "SYSTEM");
     snprintf(log.message, sizeof(log.message), "%s", "系统启动成功");
 
     ret = sqlite_manager_insert_log(&log);
     print_result("插入 INFO 日志", ret);
 
     /* 2. 插入 WARNING 日志 */
     log.timestamp = now + 1;
 
     snprintf(log.log_level, sizeof(log.log_level), "%s", "WARNING");
     snprintf(log.module, sizeof(log.module), "%s", "MQTT");
     snprintf(log.message, sizeof(log.message), "%s", "MQTT连接状态异常");
 
     ret = sqlite_manager_insert_log(&log);
     print_result("插入 WARNING 日志", ret);
 
     /* 3. 插入 ERROR 日志 */
     log.timestamp = now + 2;
 
     snprintf(log.log_level, sizeof(log.log_level), "%s", "ERROR");
     snprintf(log.module, sizeof(log.module), "%s", "SQLITE");
     snprintf(log.message, sizeof(log.message), "%s", "数据库测试错误日志");
 
     ret = sqlite_manager_insert_log(&log);
     print_result("插入 ERROR 日志", ret);
 
     /* 4. 按时间查询 */
     memset(log_array, 0, sizeof(log_array));
 
     ret = sqlite_manager_query_log_range(
         now,
         now + 10,
         log_array,
         10,
         &actual_count
     );
 
     print_result("按时间范围查询系统日志", ret);
 
     if (ret == SQLITE_MGR_OK) {
         printf("共查询到 %d 条日志:\n", actual_count);
 
         for (int i = 0; i < actual_count; i++) {
             printf("  [%d] id=%d, time=%llu, level=%s, module=%s\n",
                    i,
                    log_array[i].id,
                    (unsigned long long)log_array[i].timestamp,
                    log_array[i].log_level,
                    log_array[i].module);
 
             printf("      message=%s\n", log_array[i].message);
         }
     }
 
     /* 5. 删除测试日志 */
     ret = sqlite_manager_delete_log_before(now + 3);
     print_result("删除指定时间以前的系统日志", ret);
 }
 
 
 /* ==================== 数据库维护测试 ==================== */
 
 static void test_maintenance(void)
 {
     sqlite_mgr_result_t ret;
 
     printf("\n");
     printf("========================================\n");
     printf("          测试数据库维护\n");
     printf("========================================\n");
 
     /* 1. WAL checkpoint */
     ret = sqlite_manager_checkpoint();
     print_result("执行 WAL checkpoint", ret);
 
     /* 2. VACUUM */
     ret = sqlite_manager_vacuum();
     print_result("执行 VACUUM", ret);
 }
 
 
 /* ==================== 主函数 ==================== */
 
 int main(void)
 {
     sqlite_mgr_result_t ret;
 
     printf("\n");
     printf("========================================\n");
     printf("      RK3588 SQLite Manager Test\n");
     printf("========================================\n");
 
     /*
      * 删除上一次测试生成的数据库。
      * 保证每次测试都从一个全新的数据库开始。
      */
     remove(TEST_DB_PATH);
     remove("data/test_env_monitor.db-wal");
     remove("data/test_env_monitor.db-shm");
 
     /* 1. 初始化数据库 */
     ret = sqlite_manager_init(TEST_DB_PATH);
     print_result("SQLite数据库初始化", ret);
 
     if (ret != SQLITE_MGR_OK) {
         printf("数据库初始化失败，终止测试。\n");
         return -1;
     }
 
     /* 2. 检查数据库状态 */
     if (sqlite_manager_is_open()) {
         printf("[PASS] 数据库当前处于打开状态\n");
     } else {
         printf("[FAIL] 数据库没有打开\n");
         sqlite_manager_deinit();
         return -1;
     }
 
     /* 3. 传感器数据测试 */
     test_sensor_data();
 
     /* 4. 设备配置测试 */
     test_device_config();
 
     /* 5. 告警记录测试 */
     test_alarm();
 
     /* 6. 系统日志测试 */
     test_system_log();
 
     /* 7. 数据库维护测试 */
     test_maintenance();
 
     /* 8. 关闭数据库 */
     sqlite_manager_deinit();
 
     if (!sqlite_manager_is_open()) {
         printf("\n[PASS] 数据库已正常关闭\n");
     } else {
         printf("\n[FAIL] 数据库关闭失败\n");
     }
 
     printf("\n");
     printf("========================================\n");
     printf("       SQLite Manager Test Finished\n");
     printf("========================================\n");
 
     return 0;
 }