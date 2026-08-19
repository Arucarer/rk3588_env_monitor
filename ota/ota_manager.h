/******************************************************************************
 * @file    ota_manager.h
 * @brief   OTA远程升级统一管理模块接口
 *
 * @author  李宇坤
 * @date    2026-08
 * @version V1.0
 *
 * @details
 * 本模块作为OTA系统的统一业务管理层，对外提供版本检查、
 * 后台升级、状态查询、升级取消以及异常恢复等接口。
 *
 * OTA Manager内部负责协调：
 *
 * ota_http
 *     → HTTP下载、断点续传
 *
 * ota_internal
 *     → 版本管理、MD5校验、升级锁、差分合成、
 *       程序安装、健康检查及自动回滚
 *
 * 外部模块无需了解OTA内部实现细节。
 ******************************************************************************/

#ifndef __OTA_MANAGER_H__
#define __OTA_MANAGER_H__

#include "ota_types.h"

/* 初始化OTA管理模块 */
int ota_manager_init(void);

/* 检查服务器是否存在可用新版本 */
int ota_manager_check_update(ota_update_info_t *update_info);

/* 启动后台静默OTA升级 */
int ota_manager_start_upgrade(const ota_update_info_t *update_info);

/* 获取当前OTA运行状态 */
int ota_manager_get_status(ota_status_t *status);

/* 取消当前OTA任务 */
int ota_manager_cancel(void);

/* 检测异常升级状态并尝试恢复 */
int ota_manager_recover(void);

/* 释放OTA管理模块资源 */
void ota_manager_deinit(void);

#endif