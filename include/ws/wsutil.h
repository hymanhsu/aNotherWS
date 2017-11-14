/**
* RFC6455 Websocket 帧格式, Payload data only accept UTF-8 octs.
* 0                   1                   2                   3
* 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
* +-+-+-+-+-------+-+-------------+-------------------------------+
* |F|R|R|R| opcode|M| Payload len |    Extended payload length    |
* |I|S|S|S|  (4)  |A|     (7)     |             (16/64)           |
* |N|V|V|V|       |S|             |   (if payload len==126/127)   |
* | |1|2|3|       |K|             |                               |
* +-+-+-+-+-------+-+-------------+ - - - - - - - - - - - - - - - +
* |     Extended payload length continued, if payload len == 127  |
* + - - - - - - - - - - - - - - - +-------------------------------+
* |                               |Masking-key, if MASK set to 1  |
* +-------------------------------+-------------------------------+
* | Masking-key (continued)       |          Payload Data         |
* +-------------------------------- - - - - - - - - - - - - - - - +
* :                     Payload Data continued ...                :
* + - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - +
* |                     Payload Data continued ...                |
* +---------------------------------------------------------------+
*
* @file       uvutil.h
* @brief      与websocket有关的工具
* @details    包括协议封装和解析等
* @author     Kevin.XU
* @date       2017.4.25
* @version    1.0.0
* @par Copyright (c):
*      ...
* @par History:
*   1.0.0: Kevin.XU, 2017.4.25, Create\n
*/


#ifndef UVTEST_WSUTIL_H
#define UVTEST_WSUTIL_H

#include <uv.h>
#include <ws/common.h>
#include <ws/membuf.h>

#ifdef __cplusplus
extern "C" {
#endif


//---------------------------------------------------------------------------
// Websocket protocol's key suffix
//---------------------------------------------------------------------------
#define WS_KEY_SUFFFIX "258EAFA5-E914-47DA-95CA-C5AB0DC85B11"


#define FIN_ENABLE        0x1
#define FIN_DISABLE       0x0


#define WS_CONTINUE_FRAME 0x0
#define WS_TEXT_FRAME     0x1
#define WS_BIN_FRAME      0x2
#define WS_CLOSE_FRAME    0x8
#define WS_PING_FRAME     0x9
#define WS_PONG_FRAME     0xA


/**
 * @brief  业务数据被分片的单片最大大小， 等于 65535 - 14 - 1
 */
#define DATA_FRAME_MAX_LEN  65520
#define HANDSHAKE_SIZE      1024


#define FLAG_WEBSOCKET_TEXT  0x1
#define FLAG_WEBSOCKET_BIN   0x2
#define FLAG_WEBSOCKET       0x4


#define SET_FLAG(flag,value)    \
    ((flag) |= (value))

#define ISSET_FLAG(flag,value)   \
    ((flag) & (value))

/**
 * 记录协议报文数据的开关
 */
//#define SERVER_DATA_RECORD_ENABLE 1
#define SERVER_DATA_PATH "/Users/xuhuahai/data/server"
//#define CLIENT_DATA_RECORD_ENABLE 1
#define CLIENT_DATA_PATH "/Users/xuhuahai/data/client"


/**
 * @brief  表示解析状态的枚举
 */
typedef enum parse_state {
//    PARSE_NORMAL = 0,            //正常解析开始状态
//
//    PARSE_FINISH_FIRST_BYTE,
//    PARSE_FINISH_SECOND_BYTE,
//    PARSE_FINISH_EXTEND_PAYLOAD_LENGTH,
//    PARSE_FINISH_MASK,

    PARSE_COMPLETE,   ///完整状态
    PARSE_FAILED,     ///解析出现错误
    PARSE_INCOMPLETE  ///非完整状态
} parse_state;

typedef enum endpoint_type {
    ENDPOINT_CLIENT = 0,
    ENDPOINT_SERVER
} endpoint_type;

/**
 * @brief  Websocket句柄，用于保存处理的中间状态数据
 */
typedef struct websocket_handle {
    membuf_t protoBuf;  ///原始帧数据
    membuf_t mergeBuf;  ///需要缓存的多个分片
    endpoint_type endpointType;   ///在哪个端创建的句柄

    uchar isEof:1;  ///是否是结束帧 data[0]>>7
    uchar dfExt:3;  ///是否有扩展定义 (data[0]>>4) & 0x7
    uchar type:4;   ///消息类型

    uchar hasMask:1;      ///是否有掩码
    uchar payloadLen:7;   ///payload len field

    uchar mask[4];   ///掩码内容

    size_t realPayloadLen;     ///真实数据长度
    size_t readCount;         ///本次解析读取的字节数量
    parse_state parseState;   ///解析状态
} websocket_handle;


/**
 * 对输入先进行SHA1，再进行Base64处理
 * @param input   输入内容的首指针
 * @param n       输入内容的长度
 * @note 返回的内容，需要调用free()进行内存回收
 * @return  处理完得到的字符串
 */
char* sha1_and_base64(char * input, size_t n);


/**
 * 产生websocket客户端的Handshake请求报文内容
 * @param ip     WS服务器IP
 * @param port   WS服务器端口
 * @param path   请求URI路径
 * @param key    Sec-WebSocket-Key的值
 * @note  返回的内容需要使用free()进行内容回收
 * @return  报文内容
 */
char* generate_websocket_client_handshake(const char* ip, ushort port, const char * path, const char * key);


/**
 * 产生websocket服务器端的Handshake应答报文内容
 * @param key    Sec-WebSocket-Key的值
 * @note  返回的内容需要使用free()进行内容回收
 * @return  报文内容
 */
char* generate_websocket_server_handshake(const char* key);

/**
 * 对数据进行掩码计算
 * @param data
 * @param len
 * @param mask
 */
void do_mask(uchar* data, size_t len, uchar* mask);

/**
 * 尝试解析一个完整报文，会从缓冲和数据区混合读取数据，这样就统一了读取源的模型，极大简化了逻辑复杂度，
 * 每一次都是从协议头开始读取，这样做可以大幅度简化逻辑。
 * 由于实际上多次读取才能读完整的情况并不常见，所以对性能影响可以忽略，
 * @param handle
 * @param data
 * @param datalen
 * @return 返回1表示解析出了一个完整报文，返回0表示协议解析不完整(可能可以读出0或n个字节)
 */
int try_parse_protocol(websocket_handle* handle, char* data, size_t datalen);


/**
 * 解析websocket帧
 * @param handle    websocket句柄的指针
 * @param data      需要解析的数据缓冲的首指针
 * @param datalen   需要解析的数据缓冲的长度
 */
//void parse_websocket_frame(websocket_handle* handle, char* data, ulong datalen);


/**
 * 产生一个websocket帧
 * @param buf     用来存放帧的缓冲区
 * @param data    实际数据的首指针
 * @param datalen 实际数据的长度
 * @param op      控制码/帧类型
 *                 %x0 denotes a continuation frame
 *                 %x1 denotes a text frame
 *                 %x2 denotes a binary frame
 *                 %x3-7 are reserved for further non-control frames
 *                 %x8 denotes a connection close
 *                 %x9 denotes a ping
 *                 %xA denotes a pong
 *                 %xB-F are reserved for further control frames
 * @param fin     是否结束帧
 * @param enable_mask 是否使用掩码, 1|0
 */
void generate_websocket_frame_without_fragment(membuf_t * buf, const void * data, size_t datalen, uchar op, uchar fin, uchar enable_mask);


/**
 * 产生一个close控制帧
 * @return
 */
uchar* generate_close_control_frame();


/**
 * 产生一个ping控制帧
 * @return
 */
uchar* generate_ping_control_frame();


/**
 * 产生一个pong控制帧
 * @return
 */
uchar* generate_pong_control_frame();


/**
 * 产生websocket帧，支持fragment
 * @param buf      用来存放帧的缓冲区
 * @param data     实际数据的首指针
 * @param datalen  实际数据的长度
 * @param op       控制码/帧类型
 */
void generate_websocket_frame(membuf_t *buf, const void * data, size_t datalen, uchar op);


#ifdef __cplusplus
}
#endif

#endif //UVTEST_WSUTIL_H
