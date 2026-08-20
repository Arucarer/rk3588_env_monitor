/******************************************************************************
 * @file    ota_internal.h
 * @brief   OTA远程升级内部功能接口
 *
 * @author  李宇坤
 * @date    2026-08
 * @version V1.0
 *
 * @details
 * 本文件定义OTA管理模块内部使用的功能接口，包括：
 * 1. version.json解析；
 * 2. 本地版本读取与保存；
 * 3. 版本号比较；
 * 4. 设备型号及差分基础版本校验；
 * 5. 升级锁与防重复升级；
 * 6. 升级包大小及MD5校验；
 * 7. 差分补丁应用；
 * 8. 当前程序备份、新版本安装；
 * 9. 健康检查、升级提交及失败回滚；
 * 10. OTA临时文件清理。
 *
 * 本文件属于OTA模块内部接口，原则上只供ota_manager.c调用，
 * main、Qt、MQTT等外部模块不应直接调用这些接口。
 ******************************************************************************/

#ifndef __OTA_INTERNAL_H__
#define __OTA_INTERNAL_H__

#include "ota_types.h"
#include "ota_config.h"
#include <stddef.h>

/* 解析服务器version.json */
int ota_internal_parse_version_file(const char *file_path, ota_update_info_t *update_info);

/* 获取本地当前版本 */
int ota_internal_get_local_version(char *version, size_t size);

/* 保存本地当前版本 */
int ota_internal_save_local_version(const char *version);

/* 比较两个版本号 */
int ota_internal_compare_version(const char *current_version, const char *new_version);

/* 检查升级信息中的设备型号 */
int ota_internal_check_device(const ota_update_info_t *update_info);

/* 检查差分升级基础版本 */
int ota_internal_check_base_version(const ota_update_info_t *update_info, const char *current_version);

/* 获取OTA升级锁 */
int ota_internal_lock_acquire(void);

/* 释放OTA升级锁 */
int ota_internal_lock_release(void);

/* 判断是否为重复升级 */
int ota_internal_check_duplicate_version(const char *version);

/* 校验升级包文件大小 */
int ota_internal_verify_file_size(const char *file_path, size_t expected_size);

/* 校验升级包MD5 */
int ota_internal_verify_md5(const char *file_path, const char *expected_md5);

/* 应用差分升级补丁 */
int ota_internal_apply_patch(const char *old_file, const char *patch_file, const char *new_file);

/* 备份当前运行程序 */
int ota_internal_backup_current(void);

/* 安装新的程序文件 */
int ota_internal_install_new(const char *new_file);

/* 执行新版本健康检查 */
int ota_internal_health_check(void);

/* 提交升级成功结果 */
int ota_internal_commit_upgrade(const char *version);

/* 升级失败回滚 */
int ota_internal_rollback(void);

/* 清理OTA临时文件 */
int ota_internal_cleanup(void);

#endif