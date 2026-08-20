/******************************************************************************
 * @file    ota_internal.c
 * @brief   OTA远程升级内部功能实现
 *
 * @author  李宇坤
 * @date    2026-08
 * @version V1.0
 ******************************************************************************/

#include "ota_internal.h"
#include <openssl/evp.h>

#include <stdio.h>
#include <string.h>

/* 解析服务器version.json */
int ota_internal_parse_version_file(const char *file_path, ota_update_info_t *update_info)
{
    FILE *fp;//这个是文件指针
    /* #1.参数检查*/
    if (file_path == NULL || update_info == NULL) {
        return -1;
    }
    /* #2.打开version.json */
    fp = fopen(file_path, "r");
    if (fp == NULL) {
        return -1;
    }
    /* #3.读取version.json */

    /* #4.解析version.json*/

    /* #5.保存版本信息*/

    fclose(fp);
    return 0;
}

/* 获取本地当前版本 */
int ota_internal_get_local_version(char *version, size_t size)
{
    (void)version;
    (void)size;

    return 0;
}

/* 保存本地版本 */
int ota_internal_save_local_version(const char *version)
{
    FILE *fp;

    /* #1. 参数检查 */
    if (version == NULL || version[0] == '\0') {
        return -1;
    }
    /* #2.创建文件 */
    fp = fopen(OTA_LOCAL_VERSION_FILE, "w");
    if (fp == NULL)
    {
        printf("open local version file failed: %s\n", OTA_LOCAL_VERSION_FILE);
        return -1;
    }    

    /* #3. 写入版本号 */
    if (fprintf(fp, "%s\n", version) < 0)
    {
        printf("write local version failed\n");
        fclose(fp);
        return -1;
    }

    /* #4. 关闭文件 */
    fclose(fp);
    return 0;
}

/* 比较版本 */
int ota_internal_compare_version(const char *current_version, const char *new_version)
{
    int cur_major, cur_minor, cur_patch;
    int new_major, new_minor, new_patch;
    
    /* #1.参数检查 */
    if (current_version == NULL || new_version == NULL) {
        return -1;
    }
    /* #2. 解析当前版本 */
    if (sscanf(current_version, "%d.%d.%d", &cur_major, &cur_minor, &cur_patch) != 3) {
        printf("invalid current version: %s\n", current_version);
        return 0;
    }    

    /* #3. 解析服务器版本 */
    if (sscanf(new_version, "%d.%d.%d",
        &new_major, &new_minor, &new_patch) != 3) {
    printf("invalid new version: %s\n", new_version);
    return 0;
    }

    /* #4. 比较主版本号 */
    if (cur_major < new_major) return -1;
    if (cur_major > new_major) return 1;

    /* #5. 比较次版本号 */
    if (cur_minor < new_minor) return -1;
    if (cur_minor > new_minor) return 1;

    /* #6. 比较修订版本号 */
    if (cur_patch < new_patch) return -1;
    if (cur_patch > new_patch) return 1;

    /* #7. 版本完全相同 */
    return 0;
}

/* 检查设备型号 */
int ota_internal_check_device(const ota_update_info_t *update_info)
{
    /*检查参数*/
    if (update_info == NULL) {
        return -1;
    }
    /* #2. 检查设备型号 */
    if (strcmp(update_info->device, OTA_DEVICE_NAME) != 0) {
        printf("device mismatch: %s\n", update_info->device);
        return -1;
    }
    return 0;
}

/* 检查差分升级基础版本 */
int ota_internal_check_base_version(const ota_update_info_t *update_info, const char *current_version)
{
    /* #1. 参数检查 */
    if (update_info == NULL || current_version == NULL) {
        return -1;
    }
    /* #2. 全量升级无需检查基础版本*/
    if (update_info->type == OTA_TYPE_FULL) {
        return 0;
    } 
    /* #3. 检查基础版本 */
    if (update_info->type != OTA_TYPE_DELTA) {
        printf("invalid OTA update type\n");
        return -1;
    }
    /* #4. 差分升级必须匹配基础版本 */
    if (strcmp(update_info->base_version, current_version) == 0) {
        return 0;
    }
    printf("OTA base version mismatch: current=%s, required=%s\n", current_version, update_info->base_version);
    return -1;
}

/* 获取升级锁 */
int ota_internal_lock_acquire(void)
{
    FILE *fp;
    /* #1. 检查锁文件是否已经存在 */
    fp = fopen(OTA_LOCK_FILE, "r");//r是只读模式，尝试打开锁文件
    if (fp != NULL) {
        fclose(fp);
        printf("OTA lock already exists\n");
        return -1;
    }
    /* #2. 创建OTA锁文件 */
    fp = fopen(OTA_LOCK_FILE, "w");//创建文件
    if (fp == NULL) {
        printf("create OTA lock failed: %s\n", OTA_LOCK_FILE);
        return -1;
    }
    /* #3. 写入简单标记 */
    fprintf(fp, "locked\n");
    /* #4. 关闭文件 */
    fclose(fp);
    return 0;
}

/* 释放升级锁 */
int ota_internal_lock_release(void)
{
    /* #1. 删除OTA锁文件 */
    if (remove(OTA_LOCK_FILE) != 0) {
        printf("remove OTA lock failed: %s\n", OTA_LOCK_FILE);
        return -1;
    }

    return 0;
}

/* 防重复升级 */
int ota_internal_check_duplicate_version(const char *version)
{
    char current_version[OTA_VERSION_MAX_LEN];
    /* #1. 参数检查 */
    if (version == NULL || version[0] == '\0') {
        return -1;
    }
    /* #2. 检查OTA失败版本 */
    if(ota_internal_get_local_version(current_version, sizeof(current_version)) < 0) {
        printf("get local version failed\n");
        return -1;
    }
    /* #3. 检查是否重复升级 */
    if (strcmp(current_version, version) == 0) {
        printf("duplicate OTA version: %s\n", version);
        return -1;
    }
    return 0;
}

/* 校验文件大小 */
int ota_internal_verify_file_size(const char *file_path, size_t expected_size)
{
    struct stat st;
    /* #1. 检查参数*/
    if (file_path == NULL || expected_size == 0) {
        return -1;
    }
    /* #2. 获取本地文件信息 */
    if (ota_internal_get_local_version(current_version, sizeof(current_version)) < 0) {
        printf("get local version failed\n");
        return -1;
    }
    /* #3. 比较文件大小 */  
    if ((size_t)st.st_size != expected_size){
        printf("file size mismatch: %s, expected=%zu, actual=%zu\n", file_path, expected_size, (size_t)st.st_size);
        return -1;
    }
    return 0;
}

/* MD5校验 */
int ota_internal_verify_md5(const char *file_path,
    const char *expected_md5)
{
    FILE *fp;
    EVP_MD_CTX *ctx;
    unsigned char buf[4096];
    unsigned char digest[EVP_MAX_MD_SIZE];
    unsigned int digest_len;
    size_t read_len;
    char md5_str[33];
    int i;

    /* #1. 参数检查 */
    if (file_path == NULL || expected_md5 == NULL) {
    return -1;
    }

    /* #2. 打开升级包 */
    fp = fopen(file_path, "rb");
    if (fp == NULL) {
    printf("open OTA file failed: %s\n", file_path);
    return -1;
    }

    /* #3. 创建MD5计算上下文 */
    ctx = EVP_MD_CTX_new();
    if (ctx == NULL) {
    fclose(fp);
    return -1;
    }

    /* #4. 初始化MD5计算 */
    if (EVP_DigestInit_ex(ctx, EVP_md5(), NULL) != 1) {
    EVP_MD_CTX_free(ctx);
    fclose(fp);
    return -1;
    }

    /* #5. 分块读取文件并计算MD5 */
    while ((read_len = fread(buf, 1, sizeof(buf), fp)) > 0) {
    if (EVP_DigestUpdate(ctx, buf, read_len) != 1) {
    EVP_MD_CTX_free(ctx);
    fclose(fp);
    return -1;
    }
    }

    /* #6. 获取最终MD5 */
    if (EVP_DigestFinal_ex(ctx, digest, &digest_len) != 1) {
    EVP_MD_CTX_free(ctx);
    fclose(fp);
    return -1;
    }

    EVP_MD_CTX_free(ctx);
    fclose(fp);

    /* #7. 将MD5二进制数据转换成32位字符串 */
    for (i = 0; i < 16; i++) {
    sprintf(&md5_str[i * 2], "%02x", digest[i]);
    }

    md5_str[32] = '\0';

    /* #8. 与服务器MD5比较 */
    if (strcmp(md5_str, expected_md5) != 0) {
    printf("OTA MD5 mismatch: actual=%s, expected=%s\n",
    md5_str, expected_md5);
    return -1;
    }

    return 0;
}

/* 应用差分补丁 */
int ota_internal_apply_patch(const char *old_file,
    const char *patch_file,
    const char *new_file)
{
char cmd[1024];
int ret;

/* #1. 参数检查 */
if (old_file == NULL || patch_file == NULL || new_file == NULL) {
return -1;
}

/* #2. 检查旧程序是否存在 */
if (access(old_file, F_OK) != 0) {
printf("old OTA file not found: %s\n", old_file);
return -1;
}

/* #3. 检查差分补丁是否存在 */
if (access(patch_file, F_OK) != 0) {
printf("OTA patch file not found: %s\n", patch_file);
return -1;
}

/* #4. 生成bspatch命令 */
snprintf(cmd, sizeof(cmd), "bspatch \"%s\" \"%s\" \"%s\"",
old_file, new_file, patch_file);

/* #5. 应用差分补丁 */
ret = system(cmd);
if (ret != 0) {
printf("apply OTA patch failed\n");
return -1;
}

/* #6. 检查新程序是否生成成功 */
if (access(new_file, F_OK) != 0) {
printf("new OTA file not generated: %s\n", new_file);
return -1;
}

return 0;
}

/* 备份当前程序 */
int ota_internal_backup_current(void)
{
    FILE *src;
    FILE *dst;
    char buf[4096];
    size_t read_len;

    /* #1. 打开当前程序 */
    src = fopen(OTA_CURRENT_PROGRAM, "rb");
    if (src == NULL) {
        printf("open current program failed: %s\n", OTA_CURRENT_PROGRAM);
        return -1;
    }

    /* #2. 创建备份文件 */
    dst = fopen(OTA_BACKUP_FILE, "wb");
    if (dst == NULL) {
        printf("create backup file failed: %s\n", OTA_BACKUP_FILE);
        fclose(src);
        return -1;
    }

    /* #3. 复制当前程序到备份文件 */
    while ((read_len = fread(buf, 1, sizeof(buf), src)) > 0) {
        if (fwrite(buf, 1, read_len, dst) != read_len) {
            printf("write OTA backup file failed\n");
            fclose(src);
            fclose(dst);
            remove(OTA_BACKUP_FILE);
            return -1;
        }
    }

    /* #4. 检查读取过程是否发生错误 */
    if (ferror(src)) {
        printf("read current program failed\n");
        fclose(src);
        fclose(dst);
        remove(OTA_BACKUP_FILE);
        return -1;
    }

    /* #5. 关闭文件 */
    fclose(src);
    fclose(dst);

    /* #6. 恢复备份文件执行权限 */
    if (chmod(OTA_BACKUP_FILE, OTA_PROGRAM_MODE) != 0) {
        printf("set backup file permission failed: %s\n", OTA_BACKUP_FILE);
        remove(OTA_BACKUP_FILE);
        return -1;
    }

    return 0;
}

/* 安装新程序 */
int ota_internal_install_new(const char *new_file)
{
    /* #1. 参数检查 */
    if (new_file == NULL) {
        return -1;
    }

    /* #2. 检查新程序是否存在 */
    if (access(new_file, F_OK) != 0) {
        printf("new OTA program not found: %s\n", new_file);
        return -1;
    }

    /* #3. 设置新程序执行权限 */
    if (chmod(new_file, OTA_PROGRAM_MODE) != 0) {
        printf("set new OTA program permission failed: %s\n", new_file);
        return -1;
    }

    /* #4. 删除当前旧程序 */
    if (remove(OTA_CURRENT_PROGRAM) != 0) {
        printf("remove current program failed: %s\n", OTA_CURRENT_PROGRAM);
        return -1;
    }

    /* #5. 将新程序移动为正式程序 */
    if (rename(new_file, OTA_CURRENT_PROGRAM) != 0) {
        printf("install new OTA program failed\n");
        return -1;
    }

    /* #6. 再次确保正式程序具有执行权限 */
    if (chmod(OTA_CURRENT_PROGRAM, OTA_PROGRAM_MODE) != 0) {
        printf("set installed program permission failed\n");
        return -1;
    }

    return 0;
}

/* 健康检查 */
int ota_internal_health_check(void)
{
    char cmd[512];
    int ret;

    /* #1. 检查新程序是否存在 */
    if (access(OTA_CURRENT_PROGRAM, F_OK) != 0) {
        printf("OTA health check failed: program not found\n");
        return -1;
    }

    /* #2. 检查程序是否具有执行权限 */
    if (access(OTA_CURRENT_PROGRAM, X_OK) != 0) {
        printf("OTA health check failed: program is not executable\n");
        return -1;
    }

    /* #3. 执行程序自检 */
    snprintf(cmd, sizeof(cmd), "%s --health-check", OTA_CURRENT_PROGRAM);

    ret = system(cmd);
    if (ret != 0) {
        printf("OTA health check failed\n");
        return -1;
    }

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
    /* #1. 检查备份文件是否存在 */
    if (access(OTA_BACKUP_FILE, F_OK) != 0) {
        printf("OTA backup file not found: %s\n", OTA_BACKUP_FILE);
        return -1;
    }

    /* #2. 删除当前失败的新程序 */
    if (access(OTA_CURRENT_PROGRAM, F_OK) == 0) {
        if (remove(OTA_CURRENT_PROGRAM) != 0) {
            printf("remove current program failed: %s\n", OTA_CURRENT_PROGRAM);
            return -1;
        }
    }

    /* #3. 恢复旧程序 */
    if (rename(OTA_BACKUP_FILE, OTA_CURRENT_PROGRAM) != 0) {
        printf("restore OTA backup failed\n");
        return -1;
    }

    /* #4. 恢复程序执行权限 */
    if (chmod(OTA_CURRENT_PROGRAM, OTA_PROGRAM_MODE) != 0) {
        printf("set rollback program permission failed\n");
        return -1;
    }

    return 0;
}

/* 清理临时文件 */
int ota_internal_cleanup(void)
{
    /* #1. 删除version.json */
    if (access(OTA_VERSION_FILE, F_OK) == 0) {
        remove(OTA_VERSION_FILE);
    }

    /* #2. 删除下载临时文件 */
    if (access(OTA_DOWNLOAD_FILE, F_OK) == 0) {
        remove(OTA_DOWNLOAD_FILE);
    }

    /* #3. 删除差分补丁文件 */
    if (access(OTA_PATCH_FILE, F_OK) == 0) {
        remove(OTA_PATCH_FILE);
    }

    /* #4. 删除新程序临时文件 */
    if (access(OTA_NEW_FILE, F_OK) == 0) {
        remove(OTA_NEW_FILE);
    }

    return 0;
}