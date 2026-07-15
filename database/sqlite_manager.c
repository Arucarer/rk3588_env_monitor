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

int sqlite_manager_init(const char *db_path)
{
    int ret;
    char *err_msg = NULL;//这个是错误信息，如果创建表失败，会返回错误信息

    const char *create_table_sql =
    "CREATE TABLE IF NOT EXISTS sensor_data (" // 创建表
    "id INTEGER PRIMARY KEY AUTOINCREMENT,"       // id
    "timestamp INTEGER,"                          // 时间戳
    "bme_humidity REAL,"                          // BME 湿度
    "bme_temperature REAL,"                       // BME 温度 
    "humidity REAL,"                              // RS485湿度
    "temperature REAL,"                           // RS485温度
    "pressure REAL,"                              // 压力
    "rain_detected INTEGER,"                      // 是否下雨
    "rainfall REAL,"                              // 雨量
    "soil_humidity REAL,"                         // 土壤湿度
    "valid INTEGER"                               // 有效性
    ");";

    if (db_path == NULL) {
        return -1;
    }
    ret = sqlite3_open(db_path, &db);
    if (ret != SQLITE_OK) {
        printf("Failed to open database: %s\n", sqlite3_errmsg(db));
        if (db != NULL) {
            sqlite3_close(db);
            db = NULL;
        }
        return -1;
    }
    ret = sqlite3_exec(db, create_table_sql, NULL, NULL, &err_msg);//创建表格
    if (ret != SQLITE_OK)
    {
        fprintf(stderr,"表格创建失败: %s\n", err_msg);
        sqlite3_free(err_msg);
        sqlite3_close(db);
        db = NULL;
        return -1;
    }
    return 0;
}
 
int sqlite_manager_insert(const sensor_data_t *data)
{
    int ret;
    sqlite3_stmt *stmt = NULL;

    if (db == NULL || data == NULL) {
        return -1;
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
    "valid"
    ") VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?);";

    /* 创建预处理语句 */
    ret = sqlite3_prepare_v2(db, insert_sql, -1, &stmt, NULL);
    if (ret != SQLITE_OK) {
        fprintf(stderr, "准备语句失败: %s\n", sqlite3_errmsg(db));
        return -1;
    }
    /* 绑定参数 */
    ret = sqlite3_bind_parameter_count(stmt);
    if (ret != 10) {
        fprintf(stderr, "参数数量无效: %d\n", ret);
        sqlite3_finalize(stmt);
        return -1;
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
    /* 执行SQL语句 */
    ret = sqlite3_step(stmt);
    if (ret != SQLITE_DONE) {
        fprintf(stderr, "执行 SQL 语句失败: %s\n", sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        return -1;
    }
    /* 释放预处理语句 */
    sqlite3_finalize(stmt);
    return 0;
}

int sqlite_manager_query_latest(sensor_data_t *data)
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
    "valid "
    "FROM sensor_data "//查询 sensor_data 表
    "ORDER BY id DESC "//按 id 降序排序
    "LIMIT 1;";//只返回一条数据
    
    if(data == NULL || db == NULL)
    {
        return -1;
    }

    /* 创建预处理语句 */
    ret = sqlite3_prepare_v2(db, query_sql, -1, &stmt, NULL);
    if (ret != SQLITE_OK) {
        fprintf(stderr, "准备语句失败: %s\n", sqlite3_errmsg(db));
        return -1;
    }
    /* 执行SQL语句 */
    ret = sqlite3_step(stmt);
    if (ret != SQLITE_ROW) { 
        fprintf(stderr, "没有数据\n");
        sqlite3_finalize(stmt);
        return -1;
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
    sqlite3_finalize(stmt);
    return 0;
}

void sqlite_manager_deinit(void)
{
    if (db != NULL) {
        sqlite3_close(db);
        db = NULL;
    }
}