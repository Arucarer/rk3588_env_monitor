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
 #include "sensor.h"
 
 int sqlite_manager_init(const char *db_path);
 
 int sqlite_manager_insert(const sensor_data_t *data);
 
 int sqlite_manager_query_latest(sensor_data_t *data);
 
 void sqlite_manager_deinit(void);
 
 
 #endif /* __SQLITE_MANAGER_H__ */