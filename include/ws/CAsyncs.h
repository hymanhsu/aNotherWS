/**
* @file       CAsyncs.h
* @brief      异步对象的管理
* @details    uv里面，异步对象是唯一安全的线程间通讯的途径！！记住"唯一安全"四个字，很多情况下都摔进了这个坑，要重视
* @author     Kevin.XU
* @date       2017.5.5
* @version    1.0.0
* @par Copyright (c):
*      ...
* @par History:
*   1.0.0: Kevin.XU, 2017.5.5, Create\n
*/

#ifndef UVTEST_CASYNCS_H
#define UVTEST_CASYNCS_H


#include <uv.h>
#include <ws/common.h>
#include <ws/CLoop.h>
#include <ws/CLock.h>

#include <functional>
#include <memory>
#include <string>
#include <deque>
#include <vector>


namespace arc {

    /**
    * @brief  发送数据的上下文
    */
    typedef struct SendDataContext{
        uv_stream_t* peer;
        void* data;
        uint len;
    } SendDataContext;

    typedef struct AsyncContext{
        uv_async_t async;
        size_t id;
    } AsyncContext;

    /**
     * @brief  长度分割反调函数
     */
    typedef void (*SplitDataLenFunction)(size_t inputLen, split_result * result);


    class CAsyncs{

    public:

        CAsyncs(arc::CLoop * pCLoop, ushort size = 100);

        /**
         * 析构必须在loop关闭后调用!
         */
        ~CAsyncs();

        /**
         * 设置识别码
         * @param identifier
         */
        void setIdentifier(size_t identifier);

        /**
         * 返回一个uv异步对象
         * @return
         */
        uv_async_t * acquireOneUvAsync();

        /**
         * 获取连续多个uv异步对象
         * @note uv异步对西那个一旦被初始化完之后，就具备了自身的执行优先顺序了，先初始化的，优先级更高，
         *       即使一批异步对象一起执行，也是初始化在前的先执行，这个问题在大包分片的时候曾经导致了问题，
         *       再次初始化也无法改变什么。
         *       比如，async2,async3,async1一起连续先后提交执行，那么async1是最先执行的，随后才是async2,async3。
         *       注：序号表示了初始化的先后顺序。
         * @param num
         * @return 是否执行成功
         */
        bool acquireManyUvAsync(size_t num, std::vector<uv_async_t *> & result);

        /**
         * 使用异步对象来发送数据给对端
         * @param async
         * @param peer
         * @param data
         * @param len
         */
        bool sendDataWithAsync(uv_async_t * async, uv_stream_t* peer, const void * data, size_t len);

        /**
         * 使用异步对象来发送数据给对端
         * @param peer
         * @param msg
         * @param len
         * @param op
         */
        bool sendDataWithAsync(uv_stream_t * peer, const void * data, size_t len);

        /**
         * 使用异步对象来发送数据给对端，基于自定义分割策略
         * @param peer
         * @param data
         * @param len
         * @param splitDataLenFunction
         */
        bool sendDataWithAsync(uv_stream_t * peer, const void * data, size_t len, SplitDataLenFunction splitDataLenFunction);

        /**
         * 发送Websocket消息
         * @param async
         * @param peer
         * @param buffer
         * @param len
         * @param op   WS_BIN_FRAME | WS_TEXT_FRAME
         */
        bool sendWebsocketMsg(uv_async_t * async, uv_stream_t * peer, const void * buffer, size_t len, uchar op);

        /**
         * 发送Websocket消息
         * @param peer
         * @param buffer
         * @param len
         * @param op
         */
        bool sendWebsocketMsg(uv_stream_t * peer, const void * buffer, size_t len, uchar op);

        /**
         * 发送Websocket消息，基于自定义分割策略
         * @param peer
         * @param buffer
         * @param len
         * @param op
         * @param splitDataLenFunction
         */
        bool sendWebsocketMsg(uv_stream_t * peer, const void * buffer, size_t len, uchar op, SplitDataLenFunction splitDataLenFunction);

    public:
        size_t                      m_identifier;  ///识别编码
        ushort                      m_size;        ///预先初始化的异步对象的个数
        arc::CLoop              *   m_pCLoop;      ///使用调用者上下文的loop对象
        arc::CMutexLock             m_mutexLock;   ///互斥锁
        std::deque<AsyncContext *>  m_asyncQueue;  ///异步对象的队列
    };


}


#endif //UVTEST_CASYNCS_H
