/**
* @file       encode.h
* @brief      编解码实现
* @details    base64和URL编解码实现
* @author     Kevin.XU
* @date       2017.4.25
* @version    1.0.0
* @par Copyright (c):
*      ...
* @par History:
*   1.0.0: Kevin.XU, 2017.4.25, Create\n
*/

#ifndef UVTEST_ENCODE_H
#define UVTEST_ENCODE_H

#include <ws/membuf.h>
#include <ws/common.h>

#ifdef __cplusplus
extern "C" {
#endif


/**
 * base64的编码
 * @param bytes_to_encode 需要编码的字节序列
 * @param in_len 字节序列的长度
 * @return 编码的结果字符串
 * @see base64_decode()
 * @note 返回的字节序列需要使用free方法进行内存释放
 */
char* base64_encode(const uchar * bytes_to_encode, uint in_len);


/**
 * base64的解码
 * @param encoded_string 需要解码的字节串
 * @param ret 保存解码的字节序列
 * @return 编码的结果字符串
 */
void base64_decode(const char * encoded_string, membuf_t * ret);


/**
 * URL编码
 * @param url 需要编码的字符串
 * @param len 编码的结果字符串长度
 * @return 编码的结果字符串
 */
char* url_encode(const char *url, uint* len);


/**
 * URL解码
 * @param url 需要解码的字符串
 * @return 解码的结果
 */
char* url_decode(char *url);


#ifdef __cplusplus
}
#endif

#endif //UVTEST_ENCODE_H
