#include <fstream>
#include <sstream>
#include <cstdio>
#include <cstdlib>
#include <map>
#include <string>
#include <time.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <stdarg.h>
#include <signal.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>

#include <simple_log.h>
#include "simple_config.h"
#include "simple_log_internal.h"



const int MAX_SINGLE_LOG_SIZE = 2048;
const int ONE_DAY_SECONDS = 24 * 60 * 60;
const int FILE_PATH_MAX = 512;

int log_level = DEBUG_LEVEL;

std::string g_config_file_path;

bool use_file_appender = false;
static FileAppender g_file_appender;


/**
 * 缩减文件名路径的长度，便于在日志中输出所在文件的名字
 * @param path
 * @param reserved_len
 * @return
 */
char * format_file_path(const char * path, size_t reserved_len){
    size_t len = strlen(path);
    if(len > reserved_len){
        return (char *)path + (len - reserved_len);
    }else{
        return (char *)path;
    }
}

/**
 * 递归创建目录
 * @param sPathName
 * @return
 */
int create_dir(const char * sPathName){
    char dirName[FILE_PATH_MAX];
    strncpy(dirName,sPathName,FILE_PATH_MAX);
    int i = 0, len = strlen(dirName);
    if(dirName[len-1] != '/'){
        strcat(dirName,"/");
    }
    len = strlen(dirName);
    for(i=1; i<len; ++i){
        if(dirName[i]=='/'){
            dirName[i] = 0;
            if(access(dirName, NULL) != 0){
                if(mkdir(dirName, 0755)){
                    printf("mkdir error which dir:%s err:%s\n", dirName, strerror(errno));
                    return -1;
                }
            }
            dirName[i] = '/';
        }
    }
    return 0;
}


/**
 * 查找目录下的文件
 * @param dir
 * @param files
 * @param tail
 */
void scandir(const char *dir, std::vector<std::string> &files, const std::string &tail) {
    //打开目录指针
    DIR *Dp;
    //文件目录结构体
    struct dirent *enty;
    //打开指定的目录，获得目录指针
    if (NULL == (Dp = opendir(dir))) {
        printf("can not open dir : %s\n", dir);
        return;
    }
    //遍历这个目录下的所有文件
    while (NULL != (enty = readdir(Dp))) {
        //判断是不是目录
        if (!(enty->d_type == 0x8)) {
            //当前目录和上一目录过滤掉
            if (0 == strcmp(".", enty->d_name) ||
                0 == strcmp("..", enty->d_name)) {
                continue;
            }
            //继续递归调用
            std::string dirname = std::string(dir) + "/" + enty->d_name;
            scandir(dirname.c_str(), files, tail);
        } else {
            std::string fullname = std::string(dir) + "/" + enty->d_name;
            if(tail == ""){
                files.push_back(fullname);
            }else{
                bool endwith = fullname.compare(fullname.size() - tail.size(), tail.size(), tail) == 0;
                if (endwith) {
                    files.push_back(fullname);
                }
            }
        }
    }
    //关闭文件指针
    closedir(Dp);
}

/**
 * 初始化日志配置
 * @return
 */
static int _check_config_file() {
    printf("Log config file : %s\n", g_config_file_path.c_str());
	std::map<std::string, std::string> configs;
	get_config_map(g_config_file_path, configs);
	if (configs.empty()) {
        return 0;
    }
    // read log level
    std::string log_level_str = configs["log_level"];
    set_log_level(log_level_str.c_str());
    
    std::string rd = configs["retain_day"];
    if (!rd.empty()) {
        g_file_appender.set_retain_day(atoi(rd.c_str()));
    }
    // read log file
    std::string dir = configs["log_dir"];
    std::string log_file = configs["log_file"];
    int ret = 0;
    if (!log_file.empty()) {
        use_file_appender = true;
        if (!g_file_appender.is_inited()) {
            ret = g_file_appender.init(dir, log_file);
        }
    }
    return ret;
}

void sigreload(int sig) {
    //printf("receive sig:%d \n", sig);
    _check_config_file();
}

/**
 * 返回日期的格式串，%04d-%02d-%02d %02d:%02d:%02d.%03d
 * @param tv
 * @return
 */
std::string _get_show_time(timeval tv) {
	char show_time[40];
	memset(show_time, 0, 40);

	struct tm *tm;
	tm = localtime(&tv.tv_sec);

	sprintf(show_time, "%04d-%02d-%02d %02d:%02d:%02d.%03d",
			tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday,
			tm->tm_hour, tm->tm_min, tm->tm_sec, (int)(tv.tv_usec/1000));
	return std::string(show_time);
}

/**
 * 把字符串形式的日志级别转换为数字值
 * @param level_str
 * @return
 */
int _get_log_level(const char *level_str) {
	if(strcasecmp(level_str, "ERROR") == 0) {
		return ERROR_LEVEL;
	}
	if(strcasecmp(level_str, "WARN") == 0) {
		return WARN_LEVEL;
	}
	if(strcasecmp(level_str, "INFO") == 0) {
		return INFO_LEVEL;
	}
	if(strcasecmp(level_str, "DEBUG") == 0) {
		return DEBUG_LEVEL;
	}
	return DEBUG_LEVEL;
}

/**
 * 设置日志级别
 * @param level
 */
void set_log_level(const char *level) {
    log_level = _get_log_level(level);
}


/**
 * 初始化日志系统
 * @param configFilePath
 * @return
 */
int log_init(const char * configFilePath){
    g_config_file_path = configFilePath;
    //signal(SIGUSR1, sigreload);
    return _check_config_file();
}

/**
 * 写日志
 * @param format
 * @param ap
 */
void _log(const char *format, va_list ap) {
    struct timeval now;
    struct timezone tz;
    gettimeofday(&now, &tz);
    std::string fin_format = _get_show_time(now) + " " + format;

    if (!use_file_appender) { // if no config, send log to stdout
        vfprintf(stderr, fin_format.c_str(), ap);
        fprintf(stderr,"\n");
        fflush(stderr);
        return;
    }
    //判断是否需要切换文件
    g_file_appender.shift_file_if_need(now, tz);
    //写日志
    g_file_appender.write_log(fin_format.c_str(), ap);
}


void log_error(const char *format, ...) {
	if (log_level < ERROR_LEVEL) {
		return;
	}

	va_list ap;
	va_start(ap, format);

	_log(format, ap);

	va_end(ap);
}

void log_warn(const char *format, ...) {
	if (log_level < WARN_LEVEL) {
		return;
	}

	va_list ap;
	va_start(ap, format);

	_log(format, ap);

	va_end(ap);
}

void log_info(const char *format, ...) {
	if (log_level < INFO_LEVEL) {
		return;
	}

	va_list ap;
	va_start(ap, format);

	_log(format, ap);

	va_end(ap);
}

void log_debug(const char *format, ...) {
	if (log_level < DEBUG_LEVEL) {
		return;
	}

	va_list ap;
	va_start(ap, format);

	_log(format, ap);

	va_end(ap);
}


FileAppender::FileAppender() {
    _is_inited = false;
    _retain_day = -1;
    pthread_mutex_init(&writelock, NULL);
}

FileAppender::~FileAppender() {
    if (_fs.is_open()) {
        _fs.close();
    }
    pthread_mutex_destroy(&writelock);
}

/**
 * 初始化日志输出文件
 * @param dir
 * @param log_file
 * @return
 */
int FileAppender::init(const std::string& dir, const std::string& log_file) {
    if (!dir.empty() && dir != ".") {
        if(create_dir(dir.c_str()) < 0){
            _is_inited = false;
            return -1;
        }
    }
    _log_dir = dir;
    _log_file = log_file;
    _log_file_path = dir + "/" + log_file;

    //启动时清除历史文件
    this->clear_old_logs();

    _fs.open(_log_file_path.c_str(), std::fstream::out | std::fstream::app);
    _is_inited = true;

    return 0;
}

/**
 * 写日志
 * @param format
 * @param ap
 * @return
 */
int FileAppender::write_log(const char *format, va_list ap) {
    char single_log[MAX_SINGLE_LOG_SIZE];
    bzero(single_log, MAX_SINGLE_LOG_SIZE);
    pthread_mutex_lock(&writelock);
    if (_fs.is_open()) {
        vsnprintf(single_log, MAX_SINGLE_LOG_SIZE-1, format, ap);
        _fs << single_log << "\n";
        _fs.flush();
    }
    pthread_mutex_unlock(&writelock);
    return 0;
}

/**
 * 构造文件名
 * @param buffer
 * @param len
 * @param tm
 */
std::string FileAppender::format_file_name(struct tm *tm){
    char buffer[FILE_PATH_MAX];
    bzero(buffer, FILE_PATH_MAX);
//    snprintf(buffer, FILE_PATH_MAX, "%s.%04d-%02d-%02d",
//             _log_file.c_str(),
//             tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday
//    );
    snprintf(buffer, FILE_PATH_MAX, "%s.%04d-%02d-%02d-%02d-%02d-%02d",
             _log_file.c_str(),
             tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday,
             tm->tm_hour, tm->tm_min, tm->tm_sec
    );
    std::string file_path = _log_dir + "/" + buffer;
    return file_path;
}

/**
 * 日志切换
 * @param tv
 * @param tz
 * @return
 */
int FileAppender::shift_file_if_need(struct timeval tv, struct timezone tz) {
    if (_last_sec == 0) {
        _last_sec = tv.tv_sec;
        return 0;
    }
    long fix_now_sec = tv.tv_sec - tz.tz_minuteswest * 60;
    long fix_last_sec = _last_sec - tz.tz_minuteswest * 60;

    long diff = fix_now_sec - fix_last_sec;

    //如果自从上次切换到现在，是否又超过了一天
    if ( diff/ONE_DAY_SECONDS > 0) {
        pthread_mutex_lock(&writelock);

        struct tm *tm;
        time_t y_sec = tv.tv_sec - ONE_DAY_SECONDS;
        tm = localtime(&y_sec); ///这里是有一个问题，就是把旧文件设置为昨天的日志文件名不会太准确，但也不影响使用
        std::string prev_file_path = format_file_name(tm);

        if (access(prev_file_path.c_str(), F_OK) != 0) {  ///如果prev_file不存在
            rename(_log_file_path.c_str(), prev_file_path.c_str()); ///把当前文件改名为prev_file
            //重新打开当前文件
            _fs.close();
            _fs.open(_log_file_path.c_str(), std::fstream::out | std::fstream::app);
            _last_sec = tv.tv_sec;
        }

        pthread_mutex_unlock(&writelock);

        delete_old_log(tv); ///清理旧文件
    }

    return 0;
}


/**
 * 删除老日志文件
 * @param tv
 * @return
 */
int FileAppender::delete_old_log(timeval tv) {
    if (_retain_day <= 0) {
        return 0;
    }
    struct timeval old_tv;
    old_tv.tv_sec = tv.tv_sec - _retain_day * ONE_DAY_SECONDS;
    old_tv.tv_usec = tv.tv_usec;
    char old_file[FILE_PATH_MAX];
    memset(old_file, 0, FILE_PATH_MAX);
    struct tm *tm;
    tm = localtime(&old_tv.tv_sec);
    std::string old_file_path = format_file_name(tm);
    return remove(old_file_path.c_str());
}


/**
 * 清除历史上的老文件
 */
void FileAppender::clear_old_logs(){
    struct timeval now;
    struct timezone tz;
    gettimeofday(&now, &tz);
    long fix_now_sec = now.tv_sec - tz.tz_minuteswest * 60;
    std::vector<std::string> filesNames;
    scandir(_log_dir.c_str(),filesNames);
    if(!filesNames.empty()){
        std::vector<std::string>::iterator iter = filesNames.begin();
        while(iter != filesNames.end()){
            std::string & filename = *iter;
            if(filename != this->_log_file_path){
                struct stat buf;
                int fd = open(filename.c_str(),O_RDONLY);
                fstat(fd, &buf);
                long modify_time = buf.st_mtime;
                close(fd);
                if(fix_now_sec - modify_time > _retain_day * ONE_DAY_SECONDS ){
                    printf("remove old log file : %s\n", filename.c_str());
                    remove(filename.c_str());
                }
            }
            iter++;
        }
    }
}


/**
 * 是否被初始化
 * @return
 */
bool FileAppender::is_inited() {
    return _is_inited;
}

/**
 * 设置保留天数
 * @param rd
 */
void FileAppender::set_retain_day(int rd) {
    _retain_day = rd;
}



