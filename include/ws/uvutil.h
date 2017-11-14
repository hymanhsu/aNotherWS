/**
* @file       uvutil.h
* @brief      与uv库有关的工具
* @details    包括连接关闭,发送消息等
* @author     Kevin.XU
* @date       2017.4.25
* @version    1.0.0
* @par Copyright (c):
*      ...
* @par History:
*   1.0.0: Kevin.XU, 2017.4.25, Create\n
*/

#ifndef UVTEST_UVUTIL_H
#define UVTEST_UVUTIL_H

#include <uv.h>
#include <ws/common.h>

#ifdef __cplusplus
extern "C" {
#endif


/**
 * 从http报文中解析出http头信息
 * @param buf
 * @param key_name
 * @return
 */
char* parse_http_heads(const uv_buf_t* buf, const char * key_name);


/**
 * 关闭客户端连接后，释放客户端连接的数据
 * @param client  连接对象的指针
 */
void after_uv_close_client(uv_handle_t* client);


/**
 * 关闭连接
 * @param client    连接对象的指针
 * @param on_close  应用层在关闭前的反调函数
 */
void tw_close_client(uv_stream_t* client, char (*on_close)(uv_stream_t*));


#ifdef __cplusplus
}
#endif


#endif //UVTEST_UVUTIL_H
