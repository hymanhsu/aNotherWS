/**
 * @brief From https://github.com/hongliuliao/simple-log
 * @note Modified by Kevin.XU
 */

#ifndef SIMPLE_LOG_H
#define SIMPLE_LOG_H

#include <string.h>
#include <dirent.h>
#include <sys/time.h>
#include <unistd.h>
#include <pthread.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief 日志级别
 */
extern int log_level;

/**
 * @brief 日志级别定义
 */
enum LOGLEVEL
{
    ERROR_LEVEL = 1,
    WARN_LEVEL,
    INFO_LEVEL, 
    DEBUG_LEVEL
};

/**
 * @brief 日志中显示的文件名最大长度
 */
#define RESERVED_FILE_LEN  28

void log_error(const char *format, ...);
void log_warn(const char *format, ...);
void log_info(const char *format, ...);
void log_debug(const char *format, ...);

/**
 * 缩减文件名路径的长度，便于在日志中输出所在文件的名字
 * @param path
 * @param reserved_len
 * @return
 */
char * format_file_path(const char * path, size_t reserved_len);

/**
 * 初始化日志系统
 * @param configFilePath
 * @return
 */
int log_init(const char * configFilePath);

/**
 * 设置日志级别
 * @param level
 */
void set_log_level(const char *level);

/**
 * @brief 各种日志输出宏
 */
#define LOG_ERROR(format, args...) \
    do{ \
    if(log_level >= ERROR_LEVEL) { \
		log_error("%u-%u %s %s(%d): " format, getpid(), pthread_self(), "ERROR", format_file_path(__FILE__,RESERVED_FILE_LEN), __LINE__, ##args); \
    } \
    }while(0)

#define LOG_WARN(format, args...) \
    do{ \
    if(log_level >= WARN_LEVEL) { \
		log_warn("%u-%u %s %s(%d): " format, getpid(), pthread_self(), "WARN", format_file_path(__FILE__,RESERVED_FILE_LEN), __LINE__, ##args); \
    } \
    }while(0)

#define LOG_INFO(format, args...) \
    do{ \
    if(log_level >= INFO_LEVEL) { \
		log_info("%u-%u %s %s(%d): " format, getpid(), pthread_self(), "INFO", format_file_path(__FILE__,RESERVED_FILE_LEN), __LINE__, ##args); \
    } \
    }while(0)

#define LOG_DEBUG(format, args...) \
    do{ \
    if(log_level >= DEBUG_LEVEL) { \
		log_debug("%u-%u %s %s(%d): " format, getpid(), pthread_self(), "DEBUG", format_file_path(__FILE__,RESERVED_FILE_LEN), __LINE__, ##args); \
    } \
    }while(0)

#ifdef __cplusplus
}
#endif

#endif
