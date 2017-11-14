#ifndef SIMPLE_LOG_INTERNAL_H
#define SIMPLE_LOG_INTERNAL_H

#include <string>
#include <fstream>
#include <utility>
#include <vector>

/**
 * 递归创建目录
 * @param sPathName
 * @return
 */
int create_dir(const char * sPathName);

/**
 * 查找目录下的文件
 * @param dir
 * @param files
 * @param tail
 */
void scandir(const char *dir, std::vector<std::string> &files, const std::string &tail = "");


/**
 * 返回日期的格式串，%04d-%02d-%02d %02d:%02d:%02d.%03d
 * @param tv
 * @return
 */
std::string _get_show_time(timeval tv);

/**
 * @brief 日志输出类
 */
class FileAppender {
public:
    FileAppender();
    ~FileAppender();
    /**
     * 初始化日志输出文件
     * @param dir
     * @param log_file
     * @return
     */
    int init(const std::string& dir, const std::string& log_file);
    /**
     * 是否被初始化
     * @return
     */
    bool is_inited();
    /**
     * 写日志
     * @param format
     * @param ap
     * @return
     */
    int write_log(const char *format, va_list ap);
    /**
     * 日志切换
     * @param tv
     * @param tz
     * @return
     */
    int shift_file_if_need(struct timeval tv, struct timezone tz);
    /**
     * 删除老日志文件
     * @param tv
     * @return
     */
    int delete_old_log(timeval tv);
    /**
     * 设置保留天数
     * @param rd
     */
    void set_retain_day(int rd);
    /**
     * 构造文件名
     * @param buffer
     * @param len
     * @param tm
     */
    std::string format_file_name(struct tm *tm);
    /**
     * 清除历史上的老文件
     */
    void clear_old_logs();
private:
    std::fstream _fs;
    std::string _log_file;
    std::string _log_dir;
    std::string _log_file_path;
    long _last_sec;
    bool _is_inited;
    int _retain_day;
    pthread_mutex_t writelock;
};


#endif
