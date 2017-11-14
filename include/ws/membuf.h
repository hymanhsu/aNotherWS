/**
* @file       membuf.h
* @brief      自动扩展的内存缓冲区
* @details    包括append,insert,revert,clear等操作元语
* @author     Kevin.XU
* @date       2017.4.25
* @version    1.0.0
* @par Copyright (c):
*      ...
* @par History:
*   1.0.0: Kevin.XU, 2017.4.25, Create\n
*/

#ifndef UVTEST_MEMBUF_H
#define UVTEST_MEMBUF_H

#include <ws/common.h>

#ifdef __cplusplus
extern "C" {
#endif


/**
 * @brief 内存缓冲区
 */
typedef struct membuf_t {
    uchar* data;        ///缓冲区指针
    size_t  size;        ///缓冲区的实际数据长度
    size_t  buffer_size; ///缓冲区尺寸
    uchar  flag;        ///Websocket的类型标志字节，为了与协议取值保持兼容，文本和二进制的值是一样的，
                        /// ([0~7]: [0]是否WebSocket文本帧)  [1]是否WebSocket二进制帧  [2]是否websocket协议
} membuf_t;


/**
 * 缓冲区初始化
 * @param buf  指向缓冲区的指针
 * @param initial_buffer_size  缓冲区的尺寸
 * @see membuf_uninit()
 * @note 其销毁方法是membuf_uninit()
 */
void membuf_init(membuf_t* buf, uint initial_buffer_size);


/**
 * 销毁缓冲区
 * @param buf  指向缓冲区的指针
 * @see membuf_init()
 */
void membuf_uninit(membuf_t* buf);


/**
 * 清理缓冲区
 * @param buf  指向缓冲区的指针
 * @param maxSize   保留的缓冲区长度
 */
void membuf_clear(membuf_t* buf, size_t maxSize);


/**
 * 确保缓冲区的可用区域的长度
 * @param buf   指向缓冲区的指针
 * @param extra_size   需要确保的用于保存数据尺寸
 */
void membuf_reserve(membuf_t* buf, size_t extra_size);


/**
 * 截断(释放)多余的内存 或者增加内存,至 size+4 的大小; 后面4字节填充0
 * @param buf  指向缓冲区的指针
 */
void membuf_trunc(membuf_t* buf);


/**
 * 向缓冲区追加数据
 * @param buf  指向缓冲区的指针
 * @param data  指向需要追加的数据的首位置的指针
 * @param size  需要追加的数据的长度
 */
void membuf_append_data(membuf_t* buf, const void* data, size_t size);


/**
 * 向缓冲区追加格式化数据
 * @param buf  指向缓冲区的指针
 * @param fmt  类似printf的format串
 * @param ...  类型printf的...参数
 * @return 追加的字节数量
 */
uint membuf_append_format(membuf_t* buf, const char* fmt, ...);


/**
 * 向缓冲区中插入数据
 * @param buf  指向缓冲区的指针
 * @param offset  插入的位置，比如，设置为0则表示从头部插入
 * @param data   指向需要插入的数据的首位置的指针
 * @param size   需要插入的数据的长度
 */
void membuf_insert(membuf_t* buf, uint offset, void* data, size_t size);


/**
 * 从缓冲区的指定位置移除部分数据
 * @param buf   指向缓冲区的指针
 * @param offset 需要移除数据的位置，如果指定位置比数据长度还大则该操作没有任何影响
 * @param size  需要移除的数据长度
 */
void membuf_remove(membuf_t* buf, uint offset, size_t size);


//---------------------------------------------------------------------------
// 一些操作缓冲区的方便使用的内连函数
//---------------------------------------------------------------------------
inline void membuf_append_byte(membuf_t* buf, uchar b) {
    membuf_append_data(buf, &b, sizeof(b));
}
inline void membuf_append_int(membuf_t* buf, int i) {
    membuf_append_data(buf, &i, sizeof(i));
}
inline void membuf_append_uint(membuf_t* buf, uint ui) {
    membuf_append_data(buf, &ui, sizeof(ui));
}
inline void membuf_append_long(membuf_t* buf, long i) {
    membuf_append_data(buf, &i, sizeof(i));
}
inline void membuf_append_ulong(membuf_t* buf, size_t ui) {
    membuf_append_data(buf, &ui, sizeof(ui));
}
inline void membuf_append_short(membuf_t* buf, short s) {
    membuf_append_data(buf, &s, sizeof(s));
}
inline void membuf_append_ushort(membuf_t* buf, ushort us) {
    membuf_append_data(buf, &us, sizeof(us));
}
inline void membuf_append_float(membuf_t* buf, float f) {
    membuf_append_data(buf, &f, sizeof(f));
}
inline void membuf_append_double(membuf_t* buf, double d) {
    membuf_append_data(buf, &d, sizeof(d));
}
inline void membuf_append_ptr(membuf_t* buf, void* ptr) {
    membuf_append_data(buf, &ptr, sizeof(ptr));
}


#ifdef __cplusplus
}
#endif

#endif //UVTEST_MEMBUF_H
