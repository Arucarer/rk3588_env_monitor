/******************************************************************************
 * @file    ota_internal.c
 * @brief   OTA远程升级内部功能实现
 *
 * @author  李宇坤
 * @date    2026-08
 * @version V1.0
 ******************************************************************************/

#include "ota_internal.h"

#include <stdio.h>
#include <string.h>

/* 解析服务器version.json */
int ota_internal_parse_version_file(const char *file_path,
                                    ota_update_info_t *update_info)
{
    /* TODO:
     * 1. 打开version.json
     * 2. 解析device
     * 3. 解析version
     * 4. 解析base_version
     * 5. 解析type
     * 6. 解析level
     * 7. 解析url
     * 8. 解析md5
     * 9. 解析size
     */

    (void)file_path;
    (void)update_info;

    return 0;
}

/* 获取本地当前版本 */
int ota_internal_get_local_version(char *version, size_t size)
{
    /* TODO:
     * 从本地版本文件读取当前程序版本
     */

    (void)version;
    (void)size;

    return 0;
}

/* 保存本地版本 */
int ota_internal_save_local_version(const char *version)
{
    /* TODO:
     * 将升级成功后的版本号写入本地版本文件
     */

    (void)version;

    return 0;
}

/* 比较版本 */
int ota_internal_compare_version(const char *current_version,
                                 const char *new_version)
{
    /* TODO:
     * 分别解析major.minor.patch
     *
     * 返回值建议：
     * < 0 ：当前版本低于服务器版本
     * = 0 ：版本相同
     * > 0 ：当前版本高于服务器版本
     */

    (void)current_version;
    (void)new_version;

    return 0;
}

/* 检查设备型号 */
int ota_internal_check_device(const ota_update_info_t *update_info)
{
    /* TODO:
     * 检查服务器升级信息中的device是否属于当前设备
     */

    (void)update_info;

    return 0;
}

/* 检查差分升级基础版本 */
int ota_internal_check_base_version(const ota_update_info_t *update_info,
                                    const char *current_version)
{
    /* TODO:
     * full升级无需检查base_version
     *
     * delta升级要求：
     * current_version == base_version
     */

    (void)update_info;
    (void)current_version;

    return 0;
}

/* 获取升级锁 */
int ota_internal_lock_acquire(void)
{
    /* TODO:
     * 创建OTA锁文件
     *
     * 防止两个OTA任务同时执行
     */

    return 0;
}

/* 释放升级锁 */
int ota_internal_lock_release(void)
{
    /* TODO:
     * 删除OTA锁文件
     */

    return 0;
}

/* 防重复升级 */
int ota_internal_check_duplicate_version(const char *version)
{
    /* TODO:
     * 检查当前版本/升级记录
     * 防止重复安装同一个版本
     */

    (void)version;

    return 0;
}

/* 校验文件大小 */
int ota_internal_verify_file_size(const char *file_path,
                                  size_t expected_size)
{
    /* TODO:
     * 获取本地升级包大小
     * 与version.json中的size比较
     */

    (void)file_path;
    (void)expected_size;

    return 0;
}

/* MD5校验 */
int ota_internal_verify_md5(const char *file_path,
                            const char *expected_md5)
{
    /* TODO:
     * 1. 计算升级包MD5
     * 2. 与version.json中的MD5比较
     */

    (void)file_path;
    (void)expected_md5;

    return 0;
}

/* 应用差分补丁 */
int ota_internal_apply_patch(const char *old_file,
                             const char *patch_file,
                             const char *new_file)
{
    /* TODO:
     *
     * old_file
     *    +
     * patch_file
     *    ↓
     * bspatch
     *    ↓
     * new_file
     */

    (void)old_file;
    (void)patch_file;
    (void)new_file;

    return 0;
}

/* 备份当前程序 */
int ota_internal_backup_current(void)
{
    /* TODO:
     * 将当前env_monitor备份
     * 为后续rollback做准备
     */

    return 0;
}

/* 安装新程序 */
int ota_internal_install_new(const char *new_file)
{
    /* TODO:
     * 1. 检查新程序
     * 2. 设置执行权限
     * 3. 安全替换当前程序
     */

    (void)new_file;

    return 0;
}

/* 健康检查 */
int ota_internal_health_check(void)
{
    /* TODO:
     * 检查新版本是否能够正常启动和运行
     */

    return 0;
}

/* 提交升级 */
int ota_internal_commit_upgrade(const char *version)
{
    /* TODO:
     * 1. 保存新版本号
     * 2. 标记OTA成功
     * 3. 删除不再需要的备份
     */

    (void)version;

    return 0;
}

/* 自动回滚 */
int ota_internal_rollback(void)
{
    /* TODO:
     * 使用备份程序恢复升级前版本
     */

    return 0;
}

/* 清理临时文件 */
int ota_internal_cleanup(void)
{
    /* TODO:
     * 清理：
     * version.json
     * .part下载文件
     * patch
     * new程序等临时文件
     */

    return 0;
}