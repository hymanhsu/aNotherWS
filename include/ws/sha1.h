/**
* @file       sha1.h
* @brief      使用SHA1的Hash算法
* @details    SHA1的Hash方法
* @author     Kevin.XU
* @date       2017.4.25
* @version    1.0.0
* @par Copyright (c):
*      ...
* @par History:
*   1.0.0: Kevin.XU, 2017.4.25, Create\n
*/

#ifndef UVTEST_SHA1_H
#define UVTEST_SHA1_H

#include <ws/common.h>
#include <ws/membuf.h>

#ifdef __cplusplus
extern "C" {
#endif


/**
 * @brief SHA1上下文
 */
typedef struct SHA1_CONTEXT {
    char bFinal:1;    ///是否计算完成
    uint  h0, h1, h2, h3, h4;
    uint  nblocks;
    uint  count;
    uchar buf[64];
} SHA1_CONTEXT;


/**
 * 初始化/重置结构体
 * @param hd 指向SHA1上下文的指针
 */
void hash1_reset(SHA1_CONTEXT* hd);


/**
 * Update the message digest with the contents of INBUF with length INLEN.
 * @param hd   指向SHA1上下文的指针
 * @param inbuf  需要被处理的字节缓冲区
 * @param inlen  需要被处理的字节缓冲区的长度
 */
void hash1_write(SHA1_CONTEXT* hd, uchar *inbuf, uint inlen);


/**
 * The routine final terminates the computation and returns the digest.
 * The handle is prepared for a new cycle, but adding bytes to the
 * handle will the destroy the returned buffer.
 * Finally, got 20 bytes representing the digest.
 * @param hd   指向SHA1上下文的指针
 */
void hash1_final(SHA1_CONTEXT* hd);


/**
 * 取得hash值
 * @param hd   指向SHA1上下文的指针
 * @return
 */
uchar* hash1_get(SHA1_CONTEXT* hd);


#ifdef __cplusplus
}
#endif

#endif //UVTEST_SHA1_H
