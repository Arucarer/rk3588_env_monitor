/******************************************************************************
 * @file    sqlite_manager.c
 * @brief   SQLite 数据库管理模块实现
 *
 * @author  李宇坤
 * @date    2026-07
 * @version V1.0
 ******************************************************************************/
#include "sqlite_manager.h"
#include <stdio.h>
#include <sqlite3.h>

static sqlite3 *db = NULL; // 数据库句柄

/* ==================== 数据库基本管理 ==================== */
// 打开数据库、启用 WAL、创建所有数据表
sqlite_mgr_result_t sqlite_manager_init(const char *db_path)
{
    int ret;
    char *err_msg = NULL;//这个是错误信息，如果创建表失败，会返回错误信息

    const char *create_sensor_table_sql =
    "CREATE TABLE IF NOT EXISTS sensor_data ("
    "id INTEGER PRIMARY KEY AUTOINCREMENT,"
    "timestamp INTEGER,"
    "bme_humidity REAL,"
    "bme_temperature REAL,"
    "humidity REAL,"
    "temperature REAL,"
    "pressure REAL,"
    "rain_detected INTEGER,"
    "rainfall REAL,"
    "soil_humidity REAL,"
    "valid INTEGER,"
    "is_abnormal INTEGER"
    ");";

    const char *create_config_table_sql =
    "CREATE TABLE IF NOT EXISTS device_config ("
    "id INTEGER PRIMARY KEY AUTOINCREMENT,"
    "device_id TEXT,"
    "device_name TEXT,"
    "sample_period_ms INTEGER,"
    "sqlite_save_period_ms INTEGER,"
    "mqtt_upload_period_ms INTEGER,"
    "temperature_high REAL,"
    "temperature_low REAL,"
    "humidity_high REAL,"
    "humidity_low REAL,"
    "pressure_high REAL,"
    "pressure_low REAL,"
    "rs485_temperature_high REAL,"
    "rs485_temperature_low REAL,"
    "rs485_humidity_high REAL,"
    "rs485_humidity_low REAL,"
    "soil_humidity_low REAL,"
    "sqlite_enabled INTEGER,"
    "mqtt_enabled INTEGER,"
    "updated_at INTEGER"
    ");";

    const char *create_alarm_table_sql =
    "CREATE TABLE IF NOT EXISTS alarm_record ("
    "id INTEGER PRIMARY KEY AUTOINCREMENT,"
    "timestamp INTEGER,"
    "sensor_name TEXT,"
    "parameter TEXT,"
    "alarm_level TEXT,"
    "current_value REAL,"
    "threshold_value REAL,"
    "message TEXT,"
    "handled INTEGER,"
    "handled_time INTEGER"
    ");";

    const char *create_log_table_sql =
    "CREATE TABLE IF NOT EXISTS system_log ("
    "id INTEGER PRIMARY KEY AUTOINCREMENT,"
    "timestamp INTEGER,"
    "log_level TEXT,"
    "module TEXT,"
    "message TEXT"
    ");";

    if (db_path == NULL) {
        return SQLITE_MGR_ERR_PARAM;
    }
    ret = sqlite3_open(db_path, &db);
    if (ret != SQLITE_OK) {
        printf("Failed to open database: %s\n", sqlite3_errmsg(db));
        if (db != NULL) {
            sqlite3_close(db);
            db = NULL;
        }
        return SQLITE_MGR_ERR_DB;
    }
    /* 启用 WAL */
    ret = sqlite3_exec(db, "PRAGMA journal_mode=WAL;", NULL, NULL, &err_msg);
    if (ret != SQLITE_OK) {
        fprintf(stderr, "启用 WAL 模式失败: %s\n", err_msg);

        sqlite3_free(err_msg);
        sqlite3_close(db);
        db = NULL;

        return SQLITE_MGR_ERR_SQL;
    }

    /* 1.创建历史环境数据表 */
    ret = sqlite3_exec(db, create_sensor_table_sql, NULL, NULL, &err_msg);
    if (ret != SQLITE_OK) {
        fprintf(stderr, "sensor_data 表创建失败: %s\n", err_msg);

        sqlite3_free(err_msg);
        sqlite3_close(db);
        db = NULL;

        return SQLITE_MGR_ERR_SQL;
    }

    /* 2.创建设备参数表 */
    ret = sqlite3_exec(db, create_config_table_sql, NULL, NULL, &err_msg);
    if (ret != SQLITE_OK) {
        fprintf(stderr, "device_config 表创建失败: %s\n", err_msg);

        sqlite3_free(err_msg);
        sqlite3_close(db);
        db = NULL;

        return SQLITE_MGR_ERR_SQL;
    }

    /* 3.创建告警记录表 */
    ret = sqlite3_exec(db, create_alarm_table_sql, NULL, NULL, &err_msg);
    if (ret != SQLITE_OK) {
        fprintf(stderr, "alarm_record 表创建失败: %s\n", err_msg);

        sqlite3_free(err_msg);
        sqlite3_close(db);
        db = NULL;

        return SQLITE_MGR_ERR_SQL;
    }

    /* 4.创建系统日志表 */
    ret = sqlite3_exec(db, create_log_table_sql, NULL, NULL, &err_msg);
    if (ret != SQLITE_OK) {
        fprintf(stderr, "system_log 表创建失败: %s\n", err_msg);

        sqlite3_free(err_msg);
        sqlite3_close(db);
        db = NULL;

        return SQLITE_MGR_ERR_SQL;
    }

    return SQLITE_MGR_OK;
}
 
// 判断数据库是否已经打开
bool sqlite_manager_is_open(void)
{
    return (db != NULL);
}

// 关闭数据库
void sqlite_manager_deinit(void)
{
    if (db != NULL) {
        sqlite3_close(db);
        db = NULL;
    }
}


/* ==================== 历史环境数据 ==================== */
/* 插入一条传感器数据 */
sqlite_mgr_result_t sqlite_manager_insert(const sensor_data_t *data)
{
    int ret;
    sqlite3_stmt *stmt = NULL;

    if (data == NULL) {
        return SQLITE_MGR_ERR_PARAM;
    }
    
    if (db == NULL) {
        return SQLITE_MGR_ERR_DB;
    }

    /* 插入数据SQL语句*/
    const char *insert_sql =
    "INSERT INTO sensor_data ("
    "timestamp,"
    "bme_temperature,"
    "bme_humidity,"
    "temperature,"
    "humidity,"
    "pressure,"
    "soil_humidity,"
    "rain_detected,"
    "rainfall,"
    "valid,"
    "is_abnormal "
    ") VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?);";

    /* 创建预处理语句 */
    ret = sqlite3_prepare_v2(db, insert_sql, -1, &stmt, NULL);
    if (ret != SQLITE_OK) {
        fprintf(stderr, "准备语句失败: %s\n", sqlite3_errmsg(db));
        return SQLITE_MGR_ERR_SQL;
    }
    /* 绑定参数 */
    ret = sqlite3_bind_parameter_count(stmt);
    if (ret != 11) {
        fprintf(stderr, "参数数量无效: %d\n", ret);
        sqlite3_finalize(stmt);
        return SQLITE_MGR_ERR_SQL;
    }

    /* 预处理SQL语句 */
    sqlite3_bind_int64(stmt, 1, data->timestamp);
    sqlite3_bind_double(stmt, 2, data->bme_temperature);
    sqlite3_bind_double(stmt, 3, data->bme_humidity);
    sqlite3_bind_double(stmt, 4, data->temperature);
    sqlite3_bind_double(stmt, 5, data->humidity);
    sqlite3_bind_double(stmt, 6, data->pressure);
    sqlite3_bind_double(stmt, 7, data->soil_humidity);
    sqlite3_bind_int(stmt, 8, data->rain_detected);
    sqlite3_bind_double(stmt, 9, data->rainfall);
    sqlite3_bind_int(stmt, 10, data->valid);
    sqlite3_bind_int(stmt, 11, data->is_abnormal);
    /* 执行SQL语句 */
    ret = sqlite3_step(stmt);
    if (ret != SQLITE_DONE) {
        fprintf(stderr, "执行 SQL 语句失败: %s\n", sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        return SQLITE_MGR_ERR_SQL;
    }
    /* 释放预处理语句 */
    sqlite3_finalize(stmt);
    return SQLITE_MGR_OK;
}

/* 查询最新一条传感器数据 */
sqlite_mgr_result_t sqlite_manager_query_latest(sensor_data_t *data)
{
    int ret;
    sqlite3_stmt *stmt = NULL;

    /* 查询最新数据SQL语句 */
    const char *query_sql =
    "SELECT "
    "timestamp,"
    "bme_temperature,"
    "bme_humidity,"
    "temperature,"
    "humidity,"
    "pressure,"
    "soil_humidity,"
    "rain_detected,"
    "rainfall,"
    "valid,"
    "is_abnormal "
    "FROM sensor_data "//查询 sensor_data 表
    "ORDER BY id DESC "//按 id 降序排序
    "LIMIT 1;";//只返回一条数据
    
    /* 参数检查 */
    if (data == NULL) {
        return SQLITE_MGR_ERR_PARAM;
    }

    /* 数据库状态检查 */
    if (db == NULL) {
        return SQLITE_MGR_ERR_DB;
    }

    /* 创建预处理语句 */
    ret = sqlite3_prepare_v2(db, query_sql, -1, &stmt, NULL);
    if (ret != SQLITE_OK) {
        fprintf(stderr, "准备语句失败: %s\n", sqlite3_errmsg(db));
        return SQLITE_MGR_ERR_SQL;
    }
    /* 执行SQL语句 */
    ret = sqlite3_step(stmt);
    if (ret == SQLITE_DONE) {//没有数据
        sqlite3_finalize(stmt);
        return SQLITE_MGR_NO_DATA;
    }

    if (ret != SQLITE_ROW) {
        fprintf(stderr, "查询数据失败: %s\n", sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        return SQLITE_MGR_ERR_SQL;
    }
    /* 获取数据 */
    data->timestamp = sqlite3_column_int64(stmt, 0);//获取数据
    data->bme_temperature = sqlite3_column_double(stmt, 1);
    data->bme_humidity = sqlite3_column_double(stmt, 2);
    data->temperature = sqlite3_column_double(stmt, 3);
    data->humidity = sqlite3_column_double(stmt, 4);
    data->pressure = sqlite3_column_double(stmt, 5);
    data->soil_humidity = sqlite3_column_double(stmt, 6);
    data->rain_detected = sqlite3_column_int(stmt, 7);
    data->rainfall = sqlite3_column_double(stmt, 8);
    data->valid = sqlite3_column_int(stmt, 9);
    data->is_abnormal = sqlite3_column_int(stmt, 10);
    sqlite3_finalize(stmt);
    return SQLITE_MGR_OK;
}


/* 按时间范围查询传感器数据 */
sqlite_mgr_result_t sqlite_manager_query_range(uint64_t start_time,
    uint64_t end_time,
    sensor_data_t *data_array,
    int max_count,
    int *actual_count)
{
    int ret;
    int count = 0;
    sqlite3_stmt *stmt = NULL;

    const char *query_sql =
        "SELECT "
        "timestamp,"
        "bme_temperature,"
        "bme_humidity,"
        "temperature,"
        "humidity,"
        "pressure,"
        "soil_humidity,"
        "rain_detected,"
        "rainfall,"
        "valid,"
        "is_abnormal "
        "FROM sensor_data "
        "WHERE timestamp >= ? AND timestamp <= ? "
        "ORDER BY timestamp ASC "
        "LIMIT ?;";
    //"WHERE timestamp >= ? AND timestamp <= ? "是查询条件，表示查询时间范围内的数据
    //"ORDER BY timestamp ASC "是排序条件，表示按时间升序排序
    //"LIMIT ?;";是限制条件，表示最多查询多少条数据

    /*1.参数检查*/
    if(data_array == NULL || actual_count == NULL) {
        return SQLITE_MGR_ERR_PARAM;
    }
    if(start_time > end_time || max_count <= 0) {
        return SQLITE_MGR_ERR_PARAM;
    }
    /*2.数据库状态检查*/
    if(db == NULL) {
        return SQLITE_MGR_ERR_DB;
    }
    *actual_count = 0;//初始化实际查询到的数据条数
    /*3.SQL语句执行*/
    ret = sqlite3_prepare_v2(db, query_sql, -1, &stmt, NULL);//创建预处理语句
    if(ret != SQLITE_OK) {
        fprintf(stderr, "准备时间范围查询语句失败: %s\n", sqlite3_errmsg(db));
        return SQLITE_MGR_ERR_SQL;
    }
    /* 4. 绑定查询参数 */
    ret = sqlite3_bind_int64(stmt, 1, (sqlite3_int64)start_time);
    if (ret != SQLITE_OK) {
        sqlite3_finalize(stmt);
        return SQLITE_MGR_ERR_SQL;
    }

    ret = sqlite3_bind_int64(stmt, 2, (sqlite3_int64)end_time);
    if (ret != SQLITE_OK) {
        sqlite3_finalize(stmt);
        return SQLITE_MGR_ERR_SQL;
    }

    ret = sqlite3_bind_int(stmt, 3, max_count);
    if (ret != SQLITE_OK) {
        sqlite3_finalize(stmt);
        return SQLITE_MGR_ERR_SQL;
    }

    /* 5. 执行查询并获取数据 */
    while ((ret = sqlite3_step(stmt)) == SQLITE_ROW) //while ((ret = sqlite3_step(stmt)) == SQLITE_ROW）是循环，每次循环都会获取一条数据
    {
        data_array[count].timestamp = (uint64_t)sqlite3_column_int64(stmt, 0);

        data_array[count].bme_temperature = sqlite3_column_double(stmt, 1);

        data_array[count].bme_humidity = sqlite3_column_double(stmt, 2);

        data_array[count].temperature = sqlite3_column_double(stmt, 3);

        data_array[count].humidity = sqlite3_column_double(stmt, 4);

        data_array[count].pressure = sqlite3_column_double(stmt, 5);

        data_array[count].soil_humidity = sqlite3_column_double(stmt, 6);

        data_array[count].rain_detected = sqlite3_column_int(stmt, 7);

        data_array[count].rainfall = sqlite3_column_double(stmt, 8);

        data_array[count].valid = sqlite3_column_int(stmt, 9);

        data_array[count].is_abnormal = sqlite3_column_int(stmt, 10);

        count++;
    }
    /* 6. 判断查询是否正常结束 */
    if (ret != SQLITE_DONE) {
        fprintf(stderr, "时间范围查询失败: %s\n",
                sqlite3_errmsg(db));

        sqlite3_finalize(stmt);
        return SQLITE_MGR_ERR_SQL;
    }

    sqlite3_finalize(stmt);

    /* 7. 返回实际查询数量 */
    *actual_count = count;

    if (count == 0) {
        return SQLITE_MGR_NO_DATA;
    }

    return SQLITE_MGR_OK;
}


/* 查询传感器历史数据总条数*/
sqlite_mgr_result_t sqlite_manager_count_sensor(int *count)
{
    int ret;
    sqlite3_stmt *stmt = NULL;
    const char *query_sql = "SELECT COUNT(*) FROM sensor_data;";//查询传感器数据总条数的SQL语句

    /* 1.参数检查*/
    if (count == NULL) {
        return SQLITE_MGR_ERR_PARAM;
    }
    if (db == NULL) {
        return SQLITE_MGR_ERR_DB;
    }
    *count = 0;

    /* 3. 创建预处理语句 */
    ret = sqlite3_prepare_v2(db, query_sql, -1, &stmt, NULL);
    if (ret != SQLITE_OK) {
        fprintf(stderr, "准备统计传感器数据语句失败: %s\n",
                sqlite3_errmsg(db));
        return SQLITE_MGR_ERR_SQL;
    }
    /* 4. 执行查询 */
    ret = sqlite3_step(stmt);//执行查询语句
    if (ret != SQLITE_ROW) {
        fprintf(stderr, "查询传感器数据数量失败: %s\n", sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        return SQLITE_MGR_ERR_SQL;
    }

    /* 5. 获取查询结果 */
    *count = sqlite3_column_int(stmt, 0);
    /* 6. 释放查询语句 */
    sqlite3_finalize(stmt);
    return SQLITE_MGR_OK;
}

/* 查询传感器数据数量 */
sqlite_mgr_result_t sqlite_manager_delete_sensor_before(uint64_t timestamp)
{
    int ret;
    sqlite3_stmt *stmt = NULL;

    // 删除指定时间以前的传感器数据SQL语句
    const char *delete_sql =
        "DELETE FROM sensor_data "
        "WHERE timestamp < ?;";

    if (timestamp == 0) {
        return SQLITE_MGR_ERR_PARAM;
    }

    /* 1. 数据库状态检查 */
    if (db == NULL) {
        return SQLITE_MGR_ERR_DB;
    }

    /* 2. 创建预处理语句 */
    ret = sqlite3_prepare_v2(db, delete_sql, -1, &stmt, NULL);
    if (ret != SQLITE_OK) {
        fprintf(stderr, "准备删除历史数据语句失败: %s\n",
                sqlite3_errmsg(db));

        return SQLITE_MGR_ERR_SQL;
    }

    /* 3. 绑定时间戳 */
    ret = sqlite3_bind_int64(
        stmt,
        1,
        (sqlite3_int64)timestamp
    );

    if (ret != SQLITE_OK) {
        fprintf(stderr, "绑定删除时间参数失败: %s\n",
                sqlite3_errmsg(db));

        sqlite3_finalize(stmt);
        return SQLITE_MGR_ERR_SQL;
    }

    /* 4. 执行删除 */
    ret = sqlite3_step(stmt);

    if (ret != SQLITE_DONE) {
        fprintf(stderr, "删除历史数据失败: %s\n",
                sqlite3_errmsg(db));

        sqlite3_finalize(stmt);
        return SQLITE_MGR_ERR_SQL;
    }
    /* 5. 释放预处理语句 */
    sqlite3_finalize(stmt);

    return SQLITE_MGR_OK;
}

/* ==================== 设备参数 ==================== */
/* 保存设备配置 */
sqlite_mgr_result_t sqlite_manager_save_config(const device_config_t *config)
{
    int ret;
    sqlite3_stmt *stmt = NULL;

    const char *save_sql =
    "INSERT INTO device_config ("
    "device_id,"
    "device_name,"
    "sample_period_ms,"
    "sqlite_save_period_ms,"
    "mqtt_upload_period_ms,"
    "temperature_high,"
    "temperature_low,"
    "humidity_high,"
    "humidity_low,"
    "pressure_high,"
    "pressure_low,"
    "rs485_temperature_high,"
    "rs485_temperature_low,"
    "rs485_humidity_high,"
    "rs485_humidity_low,"
    "soil_humidity_low,"
    "sqlite_enabled,"
    "mqtt_enabled,"
    "updated_at"
    ") VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?);";

    /*1. 参数检查*/
    if(config == NULL) {
        return SQLITE_MGR_ERR_PARAM;
    }
    /*2. 数据库状态检查*/
    if(db == NULL) {
        return SQLITE_MGR_ERR_DB;
    }
    /*3. 创建预处理语句*/
    ret = sqlite3_prepare_v2(db, save_sql, -1, &stmt, NULL);
    if (ret != SQLITE_OK) {
        fprintf(stderr, "准备保存设备配置语句失败: %s\n", sqlite3_errmsg(db));
        return SQLITE_MGR_ERR_SQL;
    }
    /*4. 绑定设备基本信息*/
    sqlite3_bind_text(stmt, 1, config->device_id, -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, config->device_name, -1, SQLITE_TRANSIENT);
    
    sqlite3_bind_int(stmt, 3, config->sample_period_ms);
    sqlite3_bind_int(stmt, 4, config->sqlite_save_period_ms);
    sqlite3_bind_int(stmt, 5, config->mqtt_upload_period_ms);
    
    sqlite3_bind_double(stmt, 6, config->temperature_high);
    sqlite3_bind_double(stmt, 7, config->temperature_low);
    
    sqlite3_bind_double(stmt, 8, config->humidity_high);
    sqlite3_bind_double(stmt, 9, config->humidity_low);
    
    sqlite3_bind_double(stmt, 10, config->pressure_high);
    sqlite3_bind_double(stmt, 11, config->pressure_low);
    
    sqlite3_bind_double(stmt, 12, config->rs485_temperature_high);
    sqlite3_bind_double(stmt, 13, config->rs485_temperature_low);
    
    sqlite3_bind_double(stmt, 14, config->rs485_humidity_high);
    sqlite3_bind_double(stmt, 15, config->rs485_humidity_low);
    
    sqlite3_bind_double(stmt, 16, config->soil_humidity_low);
    
    sqlite3_bind_int(stmt, 17, config->sqlite_enabled);
    sqlite3_bind_int(stmt, 18, config->mqtt_enabled);
    
    sqlite3_bind_int64(stmt, 19, (sqlite3_int64)config->updated_at);

    /*执行SQL语句 */
    ret = sqlite3_step(stmt);
    if (ret != SQLITE_DONE) {
        fprintf(stderr, "保存设备配置失败: %s\n", sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        return SQLITE_MGR_ERR_SQL;
    }
    /* 12. 释放预处理语句 */
    sqlite3_finalize(stmt);

    return SQLITE_MGR_OK;

}

/* 加载设备配置 */
sqlite_mgr_result_t sqlite_manager_load_config(device_config_t *config)
{
    int ret;
    sqlite3_stmt *stmt = NULL;
    
    const char *query_sql =
    "SELECT "
    "device_id,"
    "device_name,"
    "sample_period_ms,"
    "sqlite_save_period_ms,"
    "mqtt_upload_period_ms,"
    "temperature_high,"
    "temperature_low,"
    "humidity_high,"
    "humidity_low,"
    "pressure_high,"
    "pressure_low,"
    "rs485_temperature_high,"
    "rs485_temperature_low,"
    "rs485_humidity_high,"
    "rs485_humidity_low,"
    "soil_humidity_low,"
    "sqlite_enabled,"
    "mqtt_enabled,"
    "updated_at "
    "FROM device_config "
    "ORDER BY id DESC "
    "LIMIT 1;";

    /* 1. 参数检查 */
    if(config == NULL) {
        return SQLITE_MGR_ERR_PARAM;
    }
    /* 2. 数据库状态检查 */
    if(db == NULL) {
        return SQLITE_MGR_ERR_DB;
    }

    /* 3. 创建预处理语句 */
    ret = sqlite3_prepare_v2(db, query_sql, -1, &stmt, NULL);
    if (ret != SQLITE_OK) {
        fprintf(stderr, "准备读取设备配置语句失败: %s\n", sqlite3_errmsg(db));
        return SQLITE_MGR_ERR_SQL;
    }
    
    /* 4. 执行查询 */
    ret = sqlite3_step(stmt);

    if (ret == SQLITE_DONE) {
        sqlite3_finalize(stmt);//释放预处理语句
        return SQLITE_MGR_NO_DATA;
    }//SQLITE_DONE是表示查询成功，但没有数据返回

    if (ret != SQLITE_ROW) {
        fprintf(stderr, "读取设备配置失败: %s\n", sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        return SQLITE_MGR_ERR_SQL;
    }//SQLITE_ROW表示查询成功，并且有数据返回

    /* 5. 读取字符串字段 */
    const unsigned char *device_id =
        sqlite3_column_text(stmt, 0);

    const unsigned char *device_name =
        sqlite3_column_text(stmt, 1);

    if (device_id != NULL) {
        snprintf(config->device_id, sizeof(config->device_id), "%s", (const char *)device_id);
    } else {
        config->device_id[0] = '\0';
    }

    if (device_name != NULL) {
        snprintf(config->device_name, sizeof(config->device_name), "%s", (const char *)device_name);
    } else {
        config->device_name[0] = '\0';
    }

    /* 6. 读取周期参数 */
    config->sample_period_ms = sqlite3_column_int(stmt, 2);

    config->sqlite_save_period_ms = sqlite3_column_int(stmt, 3);

    config->mqtt_upload_period_ms = sqlite3_column_int(stmt, 4);

    /* 7. 读取环境阈值 */
    config->temperature_high = sqlite3_column_double(stmt, 5);

    config->temperature_low = sqlite3_column_double(stmt, 6);

    config->humidity_high = sqlite3_column_double(stmt, 7);

    config->humidity_low = sqlite3_column_double(stmt, 8);

    config->pressure_high = sqlite3_column_double(stmt, 9);

    config->pressure_low = sqlite3_column_double(stmt, 10);

    /* 8. 读取 RS485 阈值 */
    config->rs485_temperature_high = sqlite3_column_double(stmt, 11);

    config->rs485_temperature_low = sqlite3_column_double(stmt, 12);

    config->rs485_humidity_high = sqlite3_column_double(stmt, 13);

    config->rs485_humidity_low = sqlite3_column_double(stmt, 14);

    /* 9. 读取土壤湿度阈值 */
    config->soil_humidity_low = sqlite3_column_double(stmt, 15);

    /* 10. 读取功能开关 */
    config->sqlite_enabled = sqlite3_column_int(stmt, 16);

    config->mqtt_enabled = sqlite3_column_int(stmt, 17);

    /* 11. 读取更新时间 */
    config->updated_at = (uint64_t)sqlite3_column_int64(stmt, 18);

    sqlite3_finalize(stmt);

    return SQLITE_MGR_OK;

}


/* ==================== 告警记录 ==================== */

/* 插入一条告警记录 */
sqlite_mgr_result_t sqlite_manager_insert_alarm(const alarm_record_t *alarm)
{
    int ret;
    sqlite3_stmt *stmt = NULL;
    const char *insert_sql =
        "INSERT INTO alarm_record ("
        "timestamp,"
        "sensor_name,"
        "parameter,"
        "alarm_level,"
        "current_value,"
        "threshold_value,"
        "message,"
        "handled,"
        "handled_time"
        ") VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?);";

    /* 1. 检查参数 */
    if (alarm == NULL){
        return SQLITE_MGR_ERR_PARAM;
    }
    /* 2.数据库状态检查*/
    if(db == NULL) {
        return SQLITE_MGR_ERR_DB;
    }
    /* 3. 创建SQL语句 */
    ret = sqlite3_prepare_v2(db, insert_sql, -1, &stmt, NULL);
    if (ret != SQLITE_OK) {
        fprintf(stderr, "准备插入告警记录语句失败: %s\n", sqlite3_errmsg(db));
        return SQLITE_MGR_ERR_SQL;
    }
    /* 4. 绑定参数 */
    sqlite3_bind_int64(stmt, 1, (sqlite3_int64)alarm->timestamp);//绑定时间戳
    sqlite3_bind_text(stmt, 2, alarm->sensor_name, -1, SQLITE_TRANSIENT);//绑定传感器名称，SQLITE_TRANSIENT表示SQLite会在内部复制字符串数据，确保在执行SQL语句时数据不会被修改或释放
    sqlite3_bind_text(stmt, 3, alarm->parameter, -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 4, alarm->alarm_level, -1, SQLITE_TRANSIENT);
    sqlite3_bind_double(stmt, 5, alarm->current_value);
    sqlite3_bind_double(stmt, 6, alarm->threshold_value);
    sqlite3_bind_text(stmt, 7, alarm->message, -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 8, alarm->handled);
    sqlite3_bind_int64(stmt, 9, (sqlite3_int64)alarm->handled_time);
    /* 5. 执行 SQL */
    ret = sqlite3_step(stmt);
    if (ret != SQLITE_DONE) {
        fprintf(stderr, "插入告警记录失败: %s\n", sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        return SQLITE_MGR_ERR_SQL;
    }
    sqlite3_finalize(stmt);
    return SQLITE_MGR_OK;
}

/* 按时间范围查询告警 */
sqlite_mgr_result_t sqlite_manager_query_alarm_range(
    uint64_t start_time,
    uint64_t end_time,
    alarm_record_t *alarm_array,
    int max_count,
    int *actual_count)
{
    int ret;
    int count = 0;
    sqlite3_stmt *stmt = NULL;
    const char *query_sql =
    "SELECT "
    "id,"
    "timestamp,"
    "sensor_name,"
    "parameter,"
    "alarm_level,"
    "current_value,"
    "threshold_value,"
    "message,"
    "handled,"
    "handled_time "
    "FROM alarm_record "
    "WHERE timestamp >= ? AND timestamp <= ? "
    "ORDER BY timestamp ASC "
    "LIMIT ?;";

    /* 检查参数 */
    if (alarm_array == NULL || actual_count == NULL) {
        return SQLITE_MGR_ERR_PARAM;
    }
    if(start_time > end_time || max_count <= 0) {
        return SQLITE_MGR_ERR_PARAM;
    }
    /* 2.数据库状态检查*/
    if(db == NULL) {
        return SQLITE_MGR_ERR_DB;
    }
    *actual_count = 0;

    /* 3.创建SQL语句 */
    ret = sqlite3_prepare_v2(db, query_sql, -1, &stmt, NULL);
    if (ret != SQLITE_OK) {
        fprintf(stderr,
            "准备告警时间范围查询语句失败: %s\n",
            sqlite3_errmsg(db));
        return SQLITE_MGR_ERR_SQL;
    }
    /* 4. 绑定查询条件 */
    ret = sqlite3_bind_int64(stmt, 1, (sqlite3_int64)start_time);
    if (ret != SQLITE_OK) {
        sqlite3_finalize(stmt);
        return SQLITE_MGR_ERR_SQL;
    }

    ret = sqlite3_bind_int64(stmt, 2, (sqlite3_int64)end_time);
    if (ret != SQLITE_OK) {
        sqlite3_finalize(stmt);
        return SQLITE_MGR_ERR_SQL;
    }

    ret = sqlite3_bind_int(stmt, 3, max_count);
    if (ret != SQLITE_OK) {
        sqlite3_finalize(stmt);
        return SQLITE_MGR_ERR_SQL;
    }

     /* 5. 循环读取查询到的告警数据 */
    while ((ret = sqlite3_step(stmt)) == SQLITE_ROW) {
        alarm_array[count].id = sqlite3_column_int(stmt, 0);
        alarm_array[count].timestamp = (uint64_t)sqlite3_column_int64(stmt, 1);

        const unsigned char *sensor_name = sqlite3_column_text(stmt, 2);
        const unsigned char *parameter = sqlite3_column_text(stmt, 3);
        const unsigned char *alarm_level = sqlite3_column_text(stmt, 4);
        const unsigned char *message = sqlite3_column_text(stmt, 7);

        alarm_array[count].current_value = sqlite3_column_double(stmt, 5);
        alarm_array[count].threshold_value = sqlite3_column_double(stmt, 6);
        alarm_array[count].handled = sqlite3_column_int(stmt, 8);
        alarm_array[count].handled_time = (uint64_t)sqlite3_column_int64(stmt, 9);

        /* 将 SQLite TEXT 数据复制到结构体字符数组 */
        snprintf(alarm_array[count].sensor_name, sizeof(alarm_array[count].sensor_name), "%s", sensor_name != NULL ? (const char *)sensor_name : "");
        snprintf(alarm_array[count].parameter, sizeof(alarm_array[count].parameter), "%s", parameter != NULL ? (const char *)parameter : "");
        snprintf(alarm_array[count].alarm_level, sizeof(alarm_array[count].alarm_level), "%s", alarm_level != NULL ? (const char *)alarm_level : "");
        snprintf(alarm_array[count].message, sizeof(alarm_array[count].message), "%s", message != NULL ? (const char *)message : "");

        count++;
    }

    if (ret != SQLITE_DONE) {
        fprintf(stderr, "告警时间范围查询失败: %s\n", sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        return SQLITE_MGR_ERR_SQL;
    }

    sqlite3_finalize(stmt);
    *actual_count = count;

    if (count == 0) {
        return SQLITE_MGR_NO_DATA;
    }

    return SQLITE_MGR_OK;    
}

/* 将指定告警标记为已处理 */
sqlite_mgr_result_t sqlite_manager_mark_alarm_handled(int alarm_id, uint64_t handled_time)
{
    int ret;
    sqlite3_stmt *stmt = NULL;//预处理语句对象

    /*更新告警记录状态*/
    const char *update_sql =
        "UPDATE alarm_record "//更新告警记录表
        "SET handled = 1, handled_time = ? "//这一句表示将告警标记为已处理，并记录处理时间
        "WHERE id = ?;";//这一句表示更新id为alarm_id的告警记录
    
    /* 1. 参数检查 */
    if (alarm_id <= 0) {
        return SQLITE_MGR_ERR_PARAM;
    }

    /* 2. 数据库状态检查 */
    if (db == NULL) {
        return SQLITE_MGR_ERR_DB;
    }

    /* 3. 创建预处理语句 */
    ret = sqlite3_prepare_v2(db, update_sql, -1, &stmt, NULL);
    if (ret != SQLITE_OK) {
        fprintf(stderr, "准备更新告警状态语句失败: %s\n", sqlite3_errmsg(db));
        return SQLITE_MGR_ERR_SQL;
    }

    /* 4. 绑定参数 */
    sqlite3_bind_int64(stmt, 1, (sqlite3_int64)handled_time);
    sqlite3_bind_int(stmt, 2, alarm_id);

    /* 5. 执行SQL */
    ret = sqlite3_step(stmt);
    if (ret != SQLITE_DONE) {
        fprintf(stderr, "更新告警状态失败: %s\n", sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        return SQLITE_MGR_ERR_SQL;
    }
    /* 6. 判断是否真的更新到了记录*/
    if (sqlite3_changes(db) == 0) {
        sqlite3_finalize(stmt);
        return SQLITE_MGR_NO_DATA;
    }
    sqlite3_finalize(stmt);
    return SQLITE_MGR_OK;
}

/* 删除指定时间以前的告警记录 */
sqlite_mgr_result_t sqlite_manager_delete_alarm_before(uint64_t timestamp)
{
    int ret;
    sqlite3_stmt *stmt = NULL;

    const char *delete_sql =
        "DELETE FROM alarm_record "
        "WHERE timestamp < ?;";
    
    /* 1. 参数检查 */
    if (timestamp == 0) {
        return SQLITE_MGR_ERR_PARAM;
    }

    /* 2. 数据库状态检查 */
    if (db == NULL) {
        return SQLITE_MGR_ERR_DB;
    }
    /* 3. 创建预处理语句 */
    ret = sqlite3_prepare_v2(db, delete_sql, -1, &stmt, NULL);
    if (ret != SQLITE_OK) {
        fprintf(stderr, "准备删除告警记录语句失败: %s\n", sqlite3_errmsg(db));
        return SQLITE_MGR_ERR_SQL;
    }
    /* 4. 绑定参数 */
    ret = sqlite3_bind_int64(stmt, 1, (sqlite3_int64)timestamp);
    if (ret != SQLITE_OK) {
        sqlite3_finalize(stmt);
        return SQLITE_MGR_ERR_SQL;
    }
    
    /* 5. 执行SQL语句 */
    ret = sqlite3_step(stmt);
    if (ret != SQLITE_DONE) {
        fprintf(stderr, "执行删除告警记录语句失败: %s\n", sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        return SQLITE_MGR_ERR_SQL;
    }
    /* 6. 释放资源 */
    sqlite3_finalize(stmt);
    return SQLITE_MGR_OK;
}


/* ==================== 系统日志 ==================== */
/* 插入系统日志 */
sqlite_mgr_result_t sqlite_manager_insert_log(const system_log_t *log)
{
    int ret;
    sqlite3_stmt *stmt = NULL;

    /* 插入系统日志*/
    const char *insert_sql =
        "INSERT INTO system_log ("
        "timestamp,"
        "log_level,"
        "module,"
        "message"
        ") VALUES (?, ?, ?, ?);";

    /* 1. 参数检查 */
    if(log == NULL) {
        return SQLITE_MGR_ERR_PARAM;
    }
    /* 2. 数据库状态检查 */
    if(db == NULL) {
        return SQLITE_MGR_ERR_DB;
    }
    /* 3. 创建SQL语句 */
    ret = sqlite3_prepare_v2(db, insert_sql, -1, &stmt, NULL);
    if(ret != SQLITE_OK) {
        fprintf(stderr, "准备插入系统日志语句失败: %s\n", sqlite3_errmsg(db));
        return SQLITE_MGR_ERR_SQL;
    }
    /* 4. 绑定参数 */
    sqlite3_bind_int64(stmt, 1, (sqlite3_int64)log->timestamp);
    sqlite3_bind_text(stmt, 2, log->log_level, -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 3, log->module, -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 4, log->message, -1, SQLITE_TRANSIENT);

    /* 5. 执行SQL语句 */
    ret = sqlite3_step(stmt);
    if(ret != SQLITE_DONE) {
        fprintf(stderr, "插入系统日志失败: %s\n", sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        return SQLITE_MGR_ERR_SQL;
    }
    /* 6. 释放资源 */
    sqlite3_finalize(stmt);
    return SQLITE_MGR_OK;
}

/* 按时间范围查询系统日志 */
sqlite_mgr_result_t sqlite_manager_query_log_range(
    uint64_t start_time,
    uint64_t end_time,
    system_log_t *log_array,
    int max_count,
    int *actual_count)
{
    int ret;
    int count = 0;
    sqlite3_stmt *stmt = NULL;

    /* 按照时间范围查询系统日志SQL语句 */
    const char *query_sql =
    "SELECT "
    "id,"
    "timestamp,"
    "log_level,"
    "module,"
    "message "
    "FROM system_log "
    "WHERE timestamp >= ? AND timestamp <= ? "
    "ORDER BY timestamp ASC "
    "LIMIT ?;";

    /* 1.检查参数 */
    if(log_array == NULL || actual_count == NULL) {
        return SQLITE_MGR_ERR_PARAM;
    }
    if(start_time > end_time || max_count <= 0) {
        return SQLITE_MGR_ERR_PARAM;
    }
    /*2.检查数据库状态*/
    if(db == NULL) {
        return SQLITE_MGR_ERR_DB;
    }
    *actual_count = 0;
    /*3.创建SQL语句*/
    ret = sqlite3_prepare_v2(db, query_sql, -1, &stmt, NULL);
    if (ret != SQLITE_OK) {
        fprintf(stderr, "准备系统日志查询语句失败: %s\n", sqlite3_errmsg(db));
        return SQLITE_MGR_ERR_SQL;
    }
    /*4.绑定查询条件*/
    ret = sqlite3_bind_int64(stmt, 1, (sqlite3_int64)start_time);
    if (ret != SQLITE_OK) {
        sqlite3_finalize(stmt);
        return SQLITE_MGR_ERR_SQL;
    }

    ret = sqlite3_bind_int64(stmt, 2, (sqlite3_int64)end_time);
    if (ret != SQLITE_OK) {
        sqlite3_finalize(stmt);
        return SQLITE_MGR_ERR_SQL;
    }

    ret = sqlite3_bind_int(stmt, 3, max_count);
    if (ret != SQLITE_OK) {
        sqlite3_finalize(stmt);
        return SQLITE_MGR_ERR_SQL;
    }

    /* 5. 循环读取系统日志 */
    while ((ret = sqlite3_step(stmt)) == SQLITE_ROW) {
        log_array[count].id = sqlite3_column_int(stmt, 0);
        log_array[count].timestamp = (uint64_t)sqlite3_column_int64(stmt, 1);
        const unsigned char *log_level = sqlite3_column_text(stmt, 2);
        const unsigned char *module = sqlite3_column_text(stmt, 3);
        const unsigned char *message = sqlite3_column_text(stmt, 4);
        //日志级别\日志来自哪个模块\具体发生了什么,const 不是说 log_level 这个指针永远不能变，而是说你不能通过 log_level 去修改它指向的 SQLite 文本内容
        snprintf(log_array[count].log_level, sizeof(log_array[count].log_level), "%s", log_level != NULL ? (const char *)log_level : "");
        snprintf(log_array[count].module, sizeof(log_array[count].module), "%s", module != NULL ? (const char *)module : "");
        snprintf(log_array[count].message, sizeof(log_array[count].message), "%s", message != NULL ? (const char *)message : "");
        //把刚刚从 SQLite 里读出来的三个临时字符串，安全地复制到你自己的 system_log_t 结构体里
        count++;
    }

    /* 6. 判断查询是否正常结束 */
    if (ret != SQLITE_DONE) {
        fprintf(stderr, "系统日志时间范围查询失败: %s\n", sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        return SQLITE_MGR_ERR_SQL;
    }

    sqlite3_finalize(stmt);

    /* 7. 返回实际查询数量 */
    *actual_count = count;

    if (count == 0) {
        return SQLITE_MGR_NO_DATA;
    }

    return SQLITE_MGR_OK;

}

/* 删除指定时间以前的系统日志 */
sqlite_mgr_result_t sqlite_manager_delete_log_before(uint64_t timestamp)
{
    int ret;
    sqlite3_stmt *stmt = NULL;

    /* 删除指定时间之前的系统日志 */
    const char *delete_sql =
        "DELETE FROM system_log "
        "WHERE timestamp < ?;";

    /* 1. 参数检查 */
    if (timestamp == 0) {
        return SQLITE_MGR_ERR_PARAM;
    }

    /* 2. 数据库状态检查 */
    if (db == NULL) {
        return SQLITE_MGR_ERR_DB;
    }

    /* 3. 创建预处理语句 */
    ret = sqlite3_prepare_v2(db, delete_sql, -1, &stmt, NULL);
    if (ret != SQLITE_OK) {
        fprintf(stderr, "准备删除系统日志语句失败: %s\n", sqlite3_errmsg(db));
        return SQLITE_MGR_ERR_SQL;
    }

    /* 4. 绑定时间戳 */
    ret = sqlite3_bind_int64(stmt, 1, (sqlite3_int64)timestamp);
    if (ret != SQLITE_OK) {
        sqlite3_finalize(stmt);
        return SQLITE_MGR_ERR_SQL;
    }

    /* 5. 执行删除 */
    ret = sqlite3_step(stmt);
    if (ret != SQLITE_DONE) {
        fprintf(stderr, "删除系统日志失败: %s\n", sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        return SQLITE_MGR_ERR_SQL;
    }

    /* 6. 释放预处理语句 */
    sqlite3_finalize(stmt);

    return SQLITE_MGR_OK;
}

/* ==================== 数据库维护 ==================== */

/* 获取数据库文件相关统计信息/记录数量等，先保留基础检查 */
sqlite_mgr_result_t sqlite_manager_checkpoint(void)
{
    int ret;
    char *err_msg = NULL;
    /* 1. 数据库状态检查 */
    if(db == NULL) {
        return SQLITE_MGR_ERR_DB;
    }
    /* 2. 执行数据库维护命令 */
    ret = sqlite3_exec(db, "PRAGMA wal_checkpoint;", NULL, NULL, &err_msg);
    if (ret != SQLITE_OK) {
        fprintf(stderr, "执行数据库维护命令失败: %s\n", err_msg);
        sqlite3_free(err_msg);
        return SQLITE_MGR_ERR_SQL;
    }
    return SQLITE_MGR_OK;
}

/* 清理数据库空闲页，压缩数据库文件 */
sqlite_mgr_result_t sqlite_manager_vacuum(void)
{
    int ret;
    char *err_msg = NULL;
    /* 1. 数据库状态检查 */
    if (db == NULL) {
        return SQLITE_MGR_ERR_DB;
    }

    /* 2. 执行 VACUUM */
    ret = sqlite3_exec(db, "VACUUM;", NULL, NULL, &err_msg);
    if (ret != SQLITE_OK) {
        fprintf(stderr, "VACUUM 执行失败: %s\n", err_msg);
        sqlite3_free(err_msg);
        return SQLITE_MGR_ERR_SQL;
    }

    return SQLITE_MGR_OK;
}
