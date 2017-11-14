/**
* @file       common.h
* @brief      通用工具
* @details    包括字符串和时间的处理
* @author     Kevin.XU
* @date       2017.4.25
* @version    1.0.0
* @par Copyright (c):
*      ...
* @par History:
*   1.0.0: Kevin.XU, 2017.4.25, Create\n
*/

#ifndef UVTEST_COMMON_H
#define UVTEST_COMMON_H


#include <stddef.h>   //size_t
#include <sys/types.h>

#include <stdint.h>  //uint8_t,etc
#include <utility>
#include <string>

#ifdef __cplusplus
extern "C" {
#endif

#define strcmpi strcasecmp

/**
 * @brief 涉及的基本变量类型
 */
typedef unsigned char       uchar;


#define MAX_SPLIT_COUNT     10
#define DEFAULT_SPLIT_COUNT 3
typedef struct split_result{
    size_t   split_count;                      ///需要分割的长度值的个数
    size_t   split_result[MAX_SPLIT_COUNT];    ///保存分割的多个长度值的数组
} split_result;

/**
 * @brief 通过stderr打印错误信息
 */
#define URGENT_PRINT(msg)                                \
  do {                                                    \
    fprintf(stderr,                                       \
            "Urgent msg in %s on line %d: %s\n",         \
            __FILE__,                                     \
            __LINE__,                                     \
            msg);                                         \
    fflush(stderr);                                       \
  } while (0)


/**
 * 获取随机数，填充指定的缓冲区，可以设定长度，
 * 该方法可以比较方便的获取随机数,
 * 例如: int rand; get_random(&rand,sizeof(int));
 * @param buf  用于保存随机数的内存区域首指针
 * @param n  需要填充的随机数字节的个数
 * @return 返回0表示操作成功
 */
int get_random(void * buf, size_t n);


/**
 * 获取当天的时间秒数
 * @return
 */
uint get_local_time_secs();


/**
 * 从头比较字符串,返回相同的长度,不区分大小写
 * @param s1 字符串1
 * @param s2 字符串2
 * @return 如果<0，则表示字符串2大; 如果>0，则表示字符串1大，如果=0，则表示相等
 */
int strinstr(const char* s1, const char* s2);


/**
 * 获取IPv4地址(第一个IPv4)
 * @return
 */
const char* get_ipv4_addr();


/**
 * 获取IPv6地址 (第一个IPv6)
 * @return
 */
const char* get_ipv6_addr();


/**
 * 从一个文件描述字读一行('\n'作为行分隔符)
 * @param fd 文件描述字
 * @param buffer 保存行的字节缓冲区
 * @param n  字节缓冲区的长度
 * @return 读到的内容的长度, <=0表示发生了错误
 */
ssize_t read_line(int fd, void *buffer, size_t n);


/**
 * 把一个长度分为多个部分
 * @param size    需要分割的长度
 * @param count   指定分割的份数
 * @param result  保存分割结果
 */
void split_ont_size_to_multi_parts(size_t size, size_t count, split_result * result);

/**
 * 解析拿到目录和文件名
 * @param configFilePath
 * @return
 */
std::pair<std::string,std::string> parseFilePath(const std::string& configFilePath);

/**
 * 保存数据
 * @param saved_data_directory
 * @param buffer
 * @param buffer_len
 */
void save_proto_data(const char * saved_data_directory, void * buffer, size_t buffer_len);

/**
 * 往网络写2字节长度
 * @param length_value
 * @param output
 */
void write_two_byte_length(uint16_t length_value, uchar * output);

/**
 * 往网络写4字节长度
 * @param length_value
 * @param output
 */
void write_four_byte_length(uint32_t length_value, uchar * output);

/**
 * 从网络读2字节长度
 * @param input
 * @return
 */
uint16_t read_two_byte_length(uchar * input);

/**
 * 从网络读4字节长度
 * @param input
 * @return
 */
uint32_t read_four_byte_length(uchar * input);


#ifdef __cplusplus
}
#endif

#endif //UVTEST_COMMON_H
