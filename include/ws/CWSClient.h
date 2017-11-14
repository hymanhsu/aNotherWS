/**
* @file       CWSClient.h
* @brief      Websocket客户端的实现
* @details
* @author     Kevin.XU
* @date       2017.5.4
* @version    1.0.0
* @par Copyright (c):
*      ...
* @par History:
*   1.0.0: Kevin.XU, 2017.5.4, Create\n
*/

#ifndef UVTEST_CWSCLIENT_H
#define UVTEST_CWSCLIENT_H

#include <ws/common.h>
#include <ws/uvutil.h>
#include <ws/wsutil.h>

#include <ws/CLoop.h>
#include <ws/CThread.h>
#include <ws/CAsyncs.h>

#include <functional>
#include <memory>
#include <string>


namespace arc {


    /**
    * @brief  客户端的状态
    */
    typedef enum WSClientStatus{
        HANDSHAKING, ///握手中
        WORKING,     ///工作中
    } WSClientStatus;


    class CWSClient{
    public:

        CWSClient(const std::string& ip, ushort port, size_t asyncSize = 100, bool enableReconnect = true, const std::string& uri = "/", bool enableHeartBeat = false);

        virtual ~CWSClient();

        /**
         * 启动
         */
        void startup();

        /**
         * 停止
         */
        void shutdown();

        /**
        * 发送消息给对端
        * @param peer
        * @param msg
        * @param len
        * @param op   WS_BIN_FRAME | WS_TEXT_FRAME
        */
        bool sendMsgWithAsync(uv_stream_t* peer, const void * msg, size_t len, uchar op);

        /**
         * 基于分割策略发送消息给对端
         * @param peer
         * @param msg
         * @param len
         * @param op
         * @param splitDataLenFunction
         */
        bool sendMsgWithAsync(uv_stream_t* peer, const void * msg, size_t len, uchar op, SplitDataLenFunction splitDataLenFunction);

        /**
         * 发送数据给对端
         * @param peer
         * @param data
         * @param len
         */
        bool sendDataWithAsync(uv_stream_t* peer, const void * data, size_t len);

        /**
         * 基于分割策略发送数据给对端
         * @param peer
         * @param data
         * @param len
         * @param splitDataLenFunction
         */
        bool sendDataWithAsync(uv_stream_t* peer, const void * data, size_t len, SplitDataLenFunction splitDataLenFunction);

        /**
         * 连接反调函数
         * @param connection
         * @param status
         */
        static void onConnectInternal(uv_connect_t* connection, int status);

        /**
         * 接收到数据的反调函数
         * @param peer
         * @param buffer
         * @param bufferLen
         */
        virtual void onData(websocket_handle* handle, uv_stream_t* peer, void * buffer, size_t bufferLen, uchar type) = 0;

        /**
         * 发生异常的反调函数
         * @param handle
         * @param peer
         * @param errcode
         * @param errstr
         */
        virtual void onError(websocket_handle* handle, uv_stream_t* peer, int errcode, const char * errstr) = 0;

        /**
         * 关闭连接的反调函数
         * @param handle
         * @param peer
         */
        virtual void onClose(websocket_handle* handle, uv_stream_t* peer) = 0;

        /**
        * 客户端连接成功后，反调函数
        * @param connection
        * @param status
        */
        virtual void onConnect(uv_stream_t* peer) = 0;

        /**
         * 连接服务器
         */
        void connect();

    public:
        arc::CLoop   * m_pCLoop;
        arc::CThread * m_pCThread;
        arc::CAsyncs * m_pCAsyncs;

        std::string m_ip;     ///服务的IP地址
        ushort      m_port;   ///服务监听端口
        std::string m_uri;    ///服务uri
        bool        m_enableHeartBeat;   ///是否允许心跳
        bool        m_enableReconnect;   ///是否允许重连
        std::string m_key;    ///客户端的key
        WSClientStatus m_WSClientStatus;  ///客户端状态

        uv_stream_t *  m_peer;
        uv_tcp_t       m_uv_tcp_t;
        uv_connect_t   m_uv_connect_t;

        uv_timer_t m_hb_uv_timer_t;  ///心跳timer
        uv_timer_t m_reconnect_uv_timer_t;  ///重连timer

    };


}


#endif //UVTEST_CWSCLIENT_H
