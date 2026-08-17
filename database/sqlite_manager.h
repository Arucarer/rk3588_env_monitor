/******************************************************************************
 * @file    sqlite_manager.h
 * @brief   SQLite 数据库管理模块接口定义
 *
 * @author  李宇坤
 * @date    2026-07
 * @version V1.0
 *
 * @details
 * 本文件定义环境监测系统 SQLite 数据库管理模块的对外接口，
 * 负责数据库初始化、环境数据写入、历史数据查询及资源释放。
 *
 * 主要功能包括：
 * 1. 打开或创建 SQLite 数据库；
 * 2. 创建环境监测数据表；
 * 3. 将传感器采集数据写入数据库；
 * 4. 查询最近一条环境监测数据；
 * 5. 关闭数据库并释放相关资源。
 *
 ******************************************************************************/

 #ifndef __SQLITE_MANAGER_H__
 #define __SQLITE_MANAGER_H__

 #include <stdint.h>
 #include <stdbool.h>

 #include "sensor.h"
 #include "device_config_t.h"
 #include "alarm_record_t.h"
 #include "system_log.h"
 
 typedef enum
 {
     SQLITE_MGR_OK = 0,          // 操作成功
 
     SQLITE_MGR_ERR_PARAM = -1,  // 参数错误
     SQLITE_MGR_ERR_DB = -2,     // 数据库未打开或数据库操作失败
     SQLITE_MGR_ERR_SQL = -3,    // SQL 准备或执行失败
     SQLITE_MGR_NO_DATA = -4     // 查询成功，但没有数据
 } sqlite_mgr_result_t;

/* ==================== 数据库基本管理 ==================== */

/* 打开数据库、启用 WAL、创建所有数据表 */
sqlite_mgr_result_t sqlite_manager_init(const char *db_path);

/* 判断数据库是否已经打开 */
bool sqlite_manager_is_open(void);

/* 关闭数据库 */
void sqlite_manager_deinit(void);

/* ==================== 历史环境数据 ==================== */

/* 插入一条传感器数据 */
sqlite_mgr_result_t sqlite_manager_insert(
    const sensor_data_t *data);

/* 查询最新一条传感器数据 */
sqlite_mgr_result_t sqlite_manager_query_latest(
    sensor_data_t *data);

/* 按时间范围查询传感器数据 */
sqlite_mgr_result_t sqlite_manager_query_range(
    uint64_t start_time,
    uint64_t end_time,
    sensor_data_t *data_array,
    int max_count,
    int *actual_count);

/* 查询传感器历史数据总条数 */
sqlite_mgr_result_t sqlite_manager_count_sensor(
    int *count);

/* 删除指定时间以前的传感器数据 */
sqlite_mgr_result_t sqlite_manager_delete_sensor_before(
    uint64_t timestamp);


/* ==================== 设备参数 ==================== */

/* 保存设备配置 */
sqlite_mgr_result_t sqlite_manager_save_config(
    const device_config_t *config);

/* 加载设备配置 */
sqlite_mgr_result_t sqlite_manager_load_config(
    device_config_t *config);

/* ==================== 告警记录 ==================== */

/* 插入一条告警记录 */
sqlite_mgr_result_t sqlite_manager_insert_alarm(
    const alarm_record_t *alarm);

/* 按时间范围查询告警 */
sqlite_mgr_result_t sqlite_manager_query_alarm_range(
    uint64_t start_time,
    uint64_t end_time,
    alarm_record_t *alarm_array,
    int max_count,
    int *actual_count);

/* 将指定告警标记为已处理 */
sqlite_mgr_result_t sqlite_manager_mark_alarm_handled(
    int alarm_id,
    uint64_t handled_time);

/* 删除指定时间以前的告警记录 */
sqlite_mgr_result_t sqlite_manager_delete_alarm_before(
    uint64_t timestamp);

/* ==================== 系统日志 ==================== */

/* 插入系统日志 */
sqlite_mgr_result_t sqlite_manager_insert_log(
    const system_log_t *log);

/* 按时间范围查询系统日志 */
sqlite_mgr_result_t sqlite_manager_query_log_range(
    uint64_t start_time,
    uint64_t end_time,
    system_log_t *log_array,
    int max_count,
    int *actual_count);

/* 删除指定时间以前的系统日志 */
sqlite_mgr_result_t sqlite_manager_delete_log_before(
    uint64_t timestamp);


/* ==================== 数据库维护 ==================== */

/* 获取数据库文件相关统计信息/记录数量等，先保留基础检查 */
sqlite_mgr_result_t sqlite_manager_checkpoint(void);

/* 清理数据库空闲页，压缩数据库文件 */
sqlite_mgr_result_t sqlite_manager_vacuum(void);


 #endif /* __SQLITE_MANAGER_H__ */