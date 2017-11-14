//
// Created by xuhuahai on 2017/4/24.
//

#include <ws/membuf.h>

#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <memory.h>
#include <assert.h>


/**
 * 缓冲区初始化
 * @param buf  指向缓冲区的指针
 * @param initial_buffer_size  缓冲区的尺寸
 * @see membuf_uninit()
 * @note 其销毁方法是membuf_uninit()
 */
void membuf_init(membuf_t* buf, uint initial_buffer_size) {
    memset(buf, 0, sizeof(membuf_t));
    buf->data = initial_buffer_size > 0 ? (uchar*)calloc(1, initial_buffer_size) : NULL;
    buf->buffer_size = initial_buffer_size;
}


/**
 * 销毁缓冲区
 * @param buf  指向缓冲区的指针
 * @see membuf_init()
 */
void membuf_uninit(membuf_t* buf) {
    if (buf->data)
        free(buf->data);
    memset(buf, 0, sizeof(membuf_t));
}


/**
 * 清理缓冲区
 * @param buf  指向缓冲区的指针
 * @param maxSize   保留的缓冲区长度
 */
void membuf_clear(membuf_t* buf, size_t maxSize)
{
    if (buf->data && buf->size)
    {
        if (maxSize>1 && buf->buffer_size > maxSize)
        {
            uchar* p = (uchar*)realloc(buf->data, maxSize);
            //防止realloc分配失败，或返回的地址一样
            assert(p);
            if (p != buf->data)
                buf->data = p;
            buf->size = 0;
            buf->buffer_size = maxSize;
        }
        else
        {
            buf->size = 0;
        }
        memset(buf->data, 0, buf->buffer_size);
    }
}


/**
 * 确保缓冲区的可用区域的长度
 * @param buf   指向缓冲区的指针
 * @param extra_size   需要确保的用于保存数据尺寸
 */
void membuf_reserve(membuf_t* buf, size_t extra_size) {
    if (extra_size > buf->buffer_size - buf->size) {
        //calculate new buffer size
        uint new_buffer_size = buf->buffer_size == 0 ? extra_size : buf->buffer_size << 1;
        uint new_data_size = buf->size + extra_size;
        while (new_buffer_size < new_data_size)
            new_buffer_size <<= 1;

        // malloc/realloc new buffer
        uchar* p = (uchar*)realloc(buf->data, new_buffer_size); // realloc new buffer
        //防止realloc分配失败，或返回的地址一样
        assert(p);
        if (p != buf->data)
            buf->data = p;
        memset((buf->data + buf->size), 0, new_buffer_size - buf->size);
        buf->buffer_size = new_buffer_size;
    }
}


/**
 * 截断(释放)多余的内存 或者增加内存,至 size+4 的大小; 后面4字节填充0
 * @param buf  指向缓冲区的指针
 */
void membuf_trunc(membuf_t* buf) {
    if (buf->buffer_size > (buf->size + 4) || buf->buffer_size < (buf->size + 4)) {
        uchar* p = (uchar*)realloc(buf->data, buf->size + 4); // realloc new buffer
        //防止realloc分配失败，或返回的地址一样
        assert(p);
        if (p && p != buf->data)
            buf->data = p;
        memset((buf->data + buf->size), 0, 4);
        buf->buffer_size = buf->size + 4;
    }
}


/**
 * 向缓冲区追加数据
 * @param buf  指向缓冲区的指针
 * @param data  指向需要追加的数据的首位置的指针
 * @param size  需要追加的数据的长度
 */
void membuf_append_data(membuf_t* buf, const void* data, size_t size) {
    if(data == NULL || size == 0){
        return;
    }
    membuf_reserve(buf, size);
    memmove((buf->data + buf->size), data, size);
    buf->size += size;
}


/**
 * 向缓冲区追加格式化数据
 * @param buf  指向缓冲区的指针
 * @param fmt  类似printf的format串
 * @param ...  类型printf的...参数
 * @return 追加的字节数量
 */
uint membuf_append_format(membuf_t* buf, const char* fmt, ...) {
    assert(fmt);
    va_list ap, ap2;
    va_start(ap, fmt);
    int size = vsnprintf(0, 0, fmt, ap) + 1;
    va_end(ap);
    membuf_reserve(buf, size);
    va_start(ap2, fmt);
    vsnprintf((char*)(buf->data + buf->size), size, fmt, ap2);
    va_end(ap2);
    return size;
}


/**
 * 向缓冲区中插入数据
 * @param buf  指向缓冲区的指针
 * @param offset  插入的位置，比如，设置为0则表示从头部插入
 * @param data   指向需要插入的数据的首位置的指针
 * @param size   需要插入的数据的长度
 */
void membuf_insert(membuf_t* buf, uint offset, void* data, size_t size) {
    assert(offset < buf->size);
    membuf_reserve(buf, size);
    memcpy((buf->data + offset + size), buf->data + offset, buf->size - offset);
    memcpy((buf->data + offset), data, size);
    buf->size += size;
}


/**
 * 从缓冲区的指定位置移除部分数据
 * @param buf   指向缓冲区的指针
 * @param offset 需要移除数据的位置，如果指定位置比数据长度还大则该操作没有任何影响
 * @param size  需要移除的数据长度
 */
void membuf_remove(membuf_t* buf, uint offset, size_t size) {
    if(offset >= buf->size){
        return;
    }
    if (offset + size >= buf->size) {
        buf->size = offset;
    } else {
        memmove((buf->data + offset), buf->data + offset + size, buf->size - offset - size);
        buf->size -= size;
    }
    if (buf->buffer_size >= buf->size)
        buf->data[buf->size] = 0;
}

