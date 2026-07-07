/******************************************************************************
 * @file    common.h
 * @brief   公共头文件
 *
 * @author  李宇坤
 * @date    2026-07
 * @version V1.0
 *
 * @details
 * 本文件定义整个项目的公共基础内容，包括：
 * 1. 通用返回值定义；
 * 2. 错误码定义；
 * 3. 日志输出宏；
 * 4. 常用宏定义；
 * 5. 公共头文件引用。
 *
 * 所有模块均可包含本文件，作为项目统一基础头文件。
 ******************************************************************************/

 #ifndef __COMMON_H__
 #define __COMMON_H__ 

/* C 标准库 */

 #include "config.h"

 #include <stdio.h>
 #include <stdlib.h>
 #include <stdint.h>
 #include <stdbool.h>

/* 常用字符串 */
 #include <string.h>

 /* Linux 基础 */
 #include <unistd.h>// POSIX标准函数
 #include <errno.h> // 错误码
 
 #endif