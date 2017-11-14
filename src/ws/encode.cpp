//
// Created by xuhuahai on 2017/4/30.
//

#include <ws/encode.h>

#include <stdio.h>
#include <memory.h>
#include <ctype.h>


char base64_table[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
char base64_end = '=';


inline char is_base64(uchar c) {
    return (isalnum(c) || (c == '+') || (c == '/'));
}


/**
 * base64的编码
 * @param bytes_to_encode 需要编码的字节序列
 * @param in_len 字节序列的长度
 * @return 编码的结果字符串
 * @see base64_decode()
 * @note 返回的字节序列需要使用free方法进行内存释放
 */
char* base64_encode(const uchar * bytes_to_encode, uint in_len)
{
    membuf_t ret;
    int i = 0, j = 0;
    uchar char_array_3[3];
    uchar char_array_4[4];

    membuf_init(&ret, in_len*3);//初始化缓存字节数为 长度的3倍

    while (in_len--) {
        char_array_3[i++] = *(bytes_to_encode++);
        if (i == 3) {
            char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
            char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
            char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
            char_array_4[3] = char_array_3[2] & 0x3f;

            for (i = 0; (i < 4); i++)
                membuf_append_data(&ret, &base64_table[char_array_4[i]], 1);
            i = 0;
        }
    }

    if (i)
    {
        for (j = i; j < 3; j++)
            char_array_3[j] = '\0';

        char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
        char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
        char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
        char_array_4[3] = char_array_3[2] & 0x3f;

        for (j = 0; (j < i + 1); j++)
            membuf_append_data(&ret, &base64_table[char_array_4[j]],1);

        while ((i++ < 3))
            membuf_append_data(&ret, &base64_end,1);
    }
    return (char*)ret.data;
}


/**
 * base64的解码
 * @param encoded_string 需要解码的字节串
 * @param ret 保存解码的字节序列
 * @return 编码的结果字符串
 */
void base64_decode(const char * encoded_string, membuf_t * ret)
{
    membuf_clear(ret,0);
    int in_len = strlen(encoded_string);
    int i = 0;
    int j = 0;
    int in_ = 0;
    uchar char_array_4[4], char_array_3[3];

    while (in_len-- && (encoded_string[in_] != '=') && is_base64(encoded_string[in_])) {
        char_array_4[i++] = encoded_string[in_]; in_++;
        if (i == 4) {
            for (i = 0; i <4; i++)
                char_array_4[i] = strstr(base64_table,(char*)&char_array_4[i])[0];

            char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
            char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
            char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

            for (i = 0; (i < 3); i++)
                membuf_append_data(ret, &char_array_3[i],1);
            i = 0;
        }
    }

    if (i) {
        for (j = i; j <4; j++)
            char_array_4[j] = 0;

        for (j = 0; j <4; j++)
            char_array_4[j] = strstr(base64_table, (char*)&char_array_4[j])[0];

        char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
        char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
        char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

        for (j = 0; (j < i - 1); j++)
            membuf_append_data(ret, &char_array_3[j], 1);
    }

}

/**
 * URL编码
 * @param url 需要编码的字符串
 * @param len 编码的结果字符串长度
 * @return 编码的结果字符串
 */
char* url_encode(const char *url, uint* len)
{
    if (!url)
        return NULL;
    membuf_t buf;
    const char *p;
    const char urlunsafe[] = "\r\n \"#%&+:;<=>?@[\\]^`{|}";
    const char hex[] = "0123456789ABCDEF";
    char enc[3] = {'%',0,0};
    len--;
    membuf_init(&buf, strlen(url)+1);
    for (p = url; *p; p++) {
        if ((p - url) > *len)
            break;
        if (*p < ' ' || *p > '~' || strchr(urlunsafe, *p)) {
            enc[1] = hex[*p >> 4];
            enc[2] = hex[*p & 0x0f];
            membuf_append_data(&buf, enc, 3);
        }
        else {
            membuf_append_data(&buf,p,1);
        }
    }
    membuf_trunc(&buf);
    *len = buf.size;
    return (char*)buf.data;
}


/**
 * URL解码
 * @param url 需要解码的字符串
 * @return 解码的结果
 */
char* url_decode(char *url)
{
    char *o,*s;
    uint tmp;

    for (o = s=url; *s; s++, o++) {
        if (*s == '%' && strlen(s) > 2 && sscanf(s + 1, "%2x", &tmp) == 1) {
            *o = (char)tmp;
            s += 2;
        }
        else {
            *o = *s;
        }
    }
    *o = '\0';
    return url;
}

