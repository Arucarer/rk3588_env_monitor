/******************************************************************************
 * @file    ota_manager.c
 * @brief   OTA远程升级统一管理模块实现
 *
 * @author  李宇坤
 * @date    2026-08
 * @version V1.0
 ******************************************************************************/

#include "ota_manager.h"
#include "ota_http.h"
#include "ota_internal.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>

static ota_status_t ota_status;                  // 当前OTA运行状态
static pthread_t ota_thread;                     // OTA工作线程
static int ota_thread_running = 0;               // OTA线程运行标志：0未运行，1正在运行
static int ota_cancel_flag = 0;                  // OTA取消标志：0正常，1请求取消
static int ota_manager_initialized = 0;          // OTA Manager初始化状态
static pthread_mutex_t ota_manager_mutex = PTHREAD_MUTEX_INITIALIZER; // Manager状态互斥锁


/* 下载服务器version.json */
static int ota_download_version_file(void);
/* 执行完整OTA升级流程 */
static int ota_execute_upgrade(const ota_update_info_t *update_info);
/* OTA后台工作线程 */
static void *ota_worker_thread(void *arg);


static int ota_is_cancel_requested(void);

/******************************************************************************
 * 对外接口
 ******************************************************************************/

/* OTA模块初始化 */
int ota_manager_init(void)
{

    /* #1.防止重复初始化*/
    if(ota_manager_initialized){
        return 0;
    }
    /* #2.初始化HTTP模块 */
    if(ota_http_init() < 0){
        printf("初始化HTTP模块失败\n");
        return -1;
    }
    ota_manager_initialized = 1;
    printf("OTA Manager初始化成功\n");
    return 0;
}

/* 检查服务器新版本 */
int ota_manager_check_update(ota_update_info_t *update_info)
{
    /* TODO:
     *
     * ota_download_version_file()
     *          ↓
     * ota_internal_parse_version_file()
     *          ↓
     * ota_internal_get_local_version()
     *          ↓
     * ota_internal_check_device()
     *          ↓
     * ota_internal_compare_version()
     *          ↓
     * 判断是否存在新版本
     */

    char current_version[OTA_VERSION_MAX_LEN];
    int ret;
    /* #1.检查参数*/
    if(update_info == NULL){
        printf("参数错误\n");
        return -1;
    }
    if(!ota_manager_initialized){
        printf("OTA Manager未初始化\n");
        return -1;
    }
    /* #2.下载version.json */
    if(ota_download_version_file() < 0){
        printf("下载version.json失败\n");
        return -1;
    }
    /* #3.解析version.json */
    if(ota_internal_parse_version_file(OTA_VERSION_FILE, update_info) < 0){
        printf("解析version.json失败\n");
        return -1;
    }
    /* #4.检查设备 */
    if(ota_internal_check_device(update_info) < 0){
        printf("检查设备失败\n");
        return -1;
    }
    /* #5.检查当前版本 */
    if(ota_internal_get_local_version(current_version, sizeof(current_version)) < 0){
        printf("获取本地版本失败\n");
        return -1;
    }
    /* #6. 比较当前版本和服务器版本 */
    ret = ota_internal_compare_version(current_version, update_info->version);

    if (ret == -2) {
        printf("OTA version compare failed\n");
        return -1;
    }
    
    if (ret == 0) {
        printf("OTA already at latest version: %s\n", current_version);
        return 0;
    }
    
    if (ret > 0) {
        printf("OTA server version is older: current=%s, server=%s\n", current_version, update_info->version);
        return 0;
    }

    /* #8. 差分升级检查基础版本 */
    if (ota_internal_check_base_version(update_info, current_version) < 0) {
        printf("OTA 基础版本检查失败\n");
        return -1;
    }

    /* #9. 当前存在可执行的新版本 */
    printf("OTA 存在可执行版本: %s -> %s\n", current_version, update_info->version);
    return 1;

}

/* 启动后台升级 */
int ota_manager_start_upgrade(const ota_update_info_t *update_info)
{
    ota_update_info_t *thread_info;
    int ret;

    /* #1. 参数和初始化状态检查 */
    if (update_info == NULL) {
        printf("参数错误\n");
        return -1;
    }

    if (!ota_manager_initialized) {
        printf("OTA Manager未初始化\n");
        return -1;
    }

    /* #2. 检查Manager内部是否已经存在OTA线程 */
    pthread_mutex_lock(&ota_manager_mutex);

    if (ota_thread_running) {
        pthread_mutex_unlock(&ota_manager_mutex);
        printf("OTA worker already running\n");
        return -1;
    }

    ota_thread_running = 1;
    ota_cancel_flag = 0;

    pthread_mutex_unlock(&ota_manager_mutex);

    /* #3. 获取OTA升级锁 */
    if (ota_internal_lock_acquire() < 0) {
        printf("OTA upgrade lock acquire failed\n");

        pthread_mutex_lock(&ota_manager_mutex);
        ota_thread_running = 0;
        pthread_mutex_unlock(&ota_manager_mutex);

        return -1;
    }

    /* #4. 创建线程独立升级信息 */
    thread_info = malloc(sizeof(ota_update_info_t));
    if (thread_info == NULL) {
        printf("allocate OTA thread info failed\n");
        ota_internal_lock_release();

        pthread_mutex_lock(&ota_manager_mutex);
        ota_thread_running = 0;
        pthread_mutex_unlock(&ota_manager_mutex);

        return -1;
    }

    memcpy(thread_info, update_info, sizeof(ota_update_info_t));

    /* #5. 创建OTA后台线程 */
    ret = pthread_create(&ota_thread, NULL, ota_worker_thread, thread_info);
    if (ret != 0) {
        printf("create OTA worker thread failed: %d\n", ret);

        free(thread_info);
        ota_internal_lock_release();

        pthread_mutex_lock(&ota_manager_mutex);
        ota_thread_running = 0;
        pthread_mutex_unlock(&ota_manager_mutex);

        return -1;
    }

    /* #6. 设置线程分离状态 */
    ret = pthread_detach(ota_thread);
    if (ret != 0) {
        printf("detach OTA worker thread failed: %d\n", ret);
    }

    printf("OTA upgrade started: %s\n", update_info->version);
    return 0;
}

/* 获取OTA状态 */
int ota_manager_get_status(ota_status_t *status)
{
    /* #1. 参数检查 */
    if (status == NULL) {
        printf("参数错误\n");
        return -1;
    }

    /* #2. 检查Manager初始化状态 */
    if (!ota_manager_initialized) {
        printf("OTA Manager未初始化\n");
        return -1;
    }

    /* #3. 加锁读取当前OTA状态 */
    pthread_mutex_lock(&ota_manager_mutex);
    memcpy(status, &ota_status, sizeof(ota_status_t));
    pthread_mutex_unlock(&ota_manager_mutex);

    return 0;
}
/* 取消OTA */
int ota_manager_cancel(void)
{
    /* #1. 检查Manager初始化状态 */
    if (!ota_manager_initialized) {
        printf("OTA Manager未初始化\n");
        return -1;
    }

    /* #2. 检查是否存在正在运行的OTA */
    pthread_mutex_lock(&ota_manager_mutex);

    if (!ota_thread_running) {
        pthread_mutex_unlock(&ota_manager_mutex);
        printf("OTA worker is not running\n");
        return -1;
    }

    /* #3. 设置统一取消标志 */
    ota_cancel_flag = 1;

    pthread_mutex_unlock(&ota_manager_mutex);

    /* #4. 通知HTTP层终止正在进行的下载 */
    ota_http_cancel();

    printf("OTA cancel requested\n");
    return 0;
}

/* 异常恢复 */
int ota_manager_recover(void)
{
    int lock_exists;
    int backup_exists;
    int success_exists;

    /* #1. 检查Manager初始化状态 */
    if (!ota_manager_initialized) {
        printf("OTA Manager未初始化\n");
        return -1;
    }

    /* #2. 检查遗留状态文件 */
    lock_exists = (access(OTA_LOCK_FILE, F_OK) == 0);
    backup_exists = (access(OTA_BACKUP_FILE, F_OK) == 0);
    success_exists = (access(OTA_SUCCESS_FILE, F_OK) == 0);

    /* #3. 没有发现任何异常状态 */
    if (!lock_exists && !backup_exists) {
        return 0;
    }

    printf("检测到上一次OTA遗留状态\n");

    /* #4. backup存在且success不存在，说明升级尚未正式提交，需要回滚 */
    if (backup_exists && !success_exists) {
        printf("检测到未提交OTA升级，开始回滚\n");

        if (ota_internal_rollback() < 0) {
            printf("OTA异常恢复回滚失败\n");
            return -1;
        }

        if (ota_internal_cleanup() < 0) {
            printf("OTA异常恢复清理失败\n");
            return -1;
        }
    }

    /* #5. backup和success同时存在，说明升级已经提交，只是backup没有删除成功 */
    if (backup_exists && success_exists) {
        printf("检测到已提交OTA遗留备份文件，清理旧备份\n");

        if (remove(OTA_BACKUP_FILE) != 0) {
            printf("清理OTA遗留备份失败: %s\n", OTA_BACKUP_FILE);
            return -1;
        }
    }

    /* #6. 只有锁文件，没有backup，说明大概率在安装前异常中断，保留断点文件 */
    if (lock_exists && !backup_exists) {
        printf("OTA在安装前异常中断，保留下载文件用于断点续传\n");
    }

    /* #7. 释放上一次遗留的升级锁 */
    if (lock_exists) {
        if (ota_internal_lock_release() < 0) {
            printf("OTA异常恢复释放升级锁失败\n");
            return -1;
        }
    }

    printf("OTA异常恢复完成\n");
    return 0;
}

/* OTA释放 */
void ota_manager_deinit(void)
{
    int thread_running;

    /* #1. 防止重复释放 */
    if (!ota_manager_initialized) {
        return;
    }

    /* #2. 检查OTA后台线程是否仍在运行 */
    pthread_mutex_lock(&ota_manager_mutex);
    thread_running = ota_thread_running;
    pthread_mutex_unlock(&ota_manager_mutex);

    if (thread_running) {
        printf("OTA worker is still running, manager deinit rejected\n");
        return;
    }

    /* #3. 释放HTTP模块 */
    ota_http_deinit();

    /* #4. 恢复Manager状态 */
    pthread_mutex_lock(&ota_manager_mutex);
    ota_manager_initialized = 0;
    pthread_mutex_unlock(&ota_manager_mutex);

    printf("OTA manager deinitialized\n");
}


/******************************************************************************
 * 内部流程函数
 ******************************************************************************/

/* 下载version.json */
static int ota_download_version_file(void)
{
    int ret;
    /*#1. 下载version.json*/
    ret = ota_http_download(OTA_VERSION_URL, OTA_VERSION_FILE);
    if (ret != 0) {
        printf("下载 version.json失败\n");
        return -1;
    }
    //#2. 下载成功
    printf("OTA下载 version.json成功\n");
    return 0;
}

/* 完整OTA执行流程 */
static int ota_execute_upgrade(const ota_update_info_t *update_info)
{
    const char *package_file;
    const char *install_file;

    /*#1.参数检查 */
    if (update_info == NULL || update_info->url[0] == '\0' ||
        update_info->md5[0] == '\0' || update_info->size == 0) {
        printf("invalid OTA update info\n");
        return -1;
    }
    /*#2. 检查升级类型 */
    if (update_info->type == OTA_TYPE_FULL) {
        package_file = OTA_DOWNLOAD_FILE;
    }
    else if (update_info->type == OTA_TYPE_DELTA) {
        package_file = OTA_PATCH_FILE;
    }
    else {
        printf("invalid OTA update type\n");
        return -1;
    }
    /*#3. 创建升级文件 */
    if(ota_http_download_resume(update_info->url, package_file) < 0){
        printf("下载失败\n");
        return -1;
    }
    if(ota_is_cancel_requested()){
        printf("OTA取消\n");
        return -1;
    }
    /*#4. 验证文件大小 */
    if (ota_internal_verify_file_size(package_file, update_info->size) < 0) {
        printf("文件大小验证失败\n");
        remove(package_file);
        return -1;
    }
    /*#5. 验证MD5 */
    if(ota_internal_verify_md5(package_file, update_info->md5) < 0){
        printf("MD5验证失败\n");
        remove(package_file);
        return -1;
    }
    if(ota_is_cancel_requested()){
        printf("OTA取消\n");
        return -1;
    }
    /*#6. 升级 */
    if (update_info->type == OTA_TYPE_DELTA) {
        if (ota_internal_apply_patch(OTA_CURRENT_PROGRAM, package_file, OTA_NEW_FILE) < 0) {
            printf("patch失败\n");
            return -1;
        }
        install_file = OTA_NEW_FILE;
    }
    else {
        install_file = OTA_DOWNLOAD_FILE;
    }

    if(ota_is_cancel_requested()){
        printf("OTA取消\n");
        return -1;
    }
    /*#7. 备份 */
    if (ota_internal_backup_current() < 0){
        printf("备份失败\n");
        return -1;
    }
    /*#8. 安装 */
    if (ota_internal_install_new(install_file) < 0) {
        printf("安装失败\n");
        //回滚
        if (ota_internal_rollback() < 0) {
            printf("OTA rollback failed\n");
        }
    
        return -1;
    }
    /* #9. 健康检查 */
    if(ota_internal_health_check() < 0){
        printf("健康检查失败\n");
        if (ota_internal_rollback() < 0) {
            printf("Ota回滚失败\n");
        }
        return -1;
    }
    /* #10. 提交升级结果 */
    if (ota_internal_commit_upgrade(update_info->version) < 0) {
        printf("OTA commit upgrade failed\n");
        if (ota_internal_rollback() < 0) {
            printf("OTA rollback failed\n");
        }
        return -1;
    }

    /* #11. 清理OTA临时文件 */
    ota_internal_cleanup();

    printf("OTA upgrade success: %s\n", update_info->version);
    return 0;
}

/* OTA后台线程 */
static void *ota_worker_thread(void *arg)
{
    ota_update_info_t *update_info = (ota_update_info_t *)arg;
    int ret;

    /* #1. 参数检查 */
    if (update_info == NULL) {
        printf("OTA worker thread invalid argument\n");

        ota_internal_lock_release();

        pthread_mutex_lock(&ota_manager_mutex);
        ota_thread_running = 0;
        ota_cancel_flag = 0;
        pthread_mutex_unlock(&ota_manager_mutex);

        return NULL;
    }

    /* #2. 执行OTA升级 */
    ret = ota_execute_upgrade(update_info);

    if (ret == 0) {
        printf("OTA worker upgrade success: %s\n", update_info->version);
    }
    else {
        printf("OTA worker upgrade failed: %s\n", update_info->version);
    }

    /* #3. 释放升级锁 */
    if (ota_internal_lock_release() < 0) {
        printf("OTA release lock failed\n");
    }

    /* #4. 释放线程参数 */
    free(update_info);

    /* #5. 恢复Manager运行状态 */
    pthread_mutex_lock(&ota_manager_mutex);
    ota_thread_running = 0;
    ota_cancel_flag = 0;
    pthread_mutex_unlock(&ota_manager_mutex);

    return NULL;
}

static int ota_is_cancel_requested(void)
{
    int cancelled;

    pthread_mutex_lock(&ota_manager_mutex);
    cancelled = ota_cancel_flag;
    pthread_mutex_unlock(&ota_manager_mutex);

    return cancelled;
}