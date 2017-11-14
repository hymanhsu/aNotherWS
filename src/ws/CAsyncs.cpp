//
// Created by xuhuahai on 2017/5/5.
//

#include <ws/CAsyncs.h>
#include <ws/uvutil.h>
#include <ws/wsutil.h>
#include <ws/common.h>
#include <ws/CChecker.h>
#include <simple_log.h>

#include <sstream>


namespace arc {


    /**
     * 发送完的回调函数
     * @param req
     * @param status
     */
    static void after_write(uv_write_t* req, int status) {
        if(status){
            LOG_ERROR("write failed,  status  = %d, %s", status, uv_strerror(status));
        }
        free(req->data);
        free(req);
        LOG_DEBUG("free uv_write_t & uv_write_t.data");
    }


    /**
    * 发送数据到对端
    * @param peer     连接对象的指针
    * @param data     需要发送的数据缓冲的首指针
    * @param len      需要发送的数据长度
    */
    static int send_data_impl(uv_stream_t* peer, void* data, uint len) {
        if(!arc::CChecker::instance().checkConnection(peer)){
            LOG_ERROR("connection is invalid !!!");
            return -1;
        }

        uv_buf_t buf = uv_buf_init((char*)data, len);

        uv_write_t* req = (uv_write_t*)malloc(sizeof(uv_write_t));
        if(!req){
            LOG_ERROR("malloc uv_write_t failed");
            return -2;
        }
        req->data = data;  ///直接使用参数中提供的内存地址，没有重新分配

        if(!arc::CChecker::instance().checkConnection(peer)){
            free(req);
            LOG_ERROR("connection is invalid !!!");
            return -1;
        }
        uv_write(req, peer, &buf, 1, after_write);

        LOG_DEBUG("write %u",len);
        return 0;
    }


    /**
    * 异步发送数据反调函数
    * @param handle
    */
    static void async_send_callback(uv_async_t *handle) {
        LOG_DEBUG("invoke async_send_callback");
        if(handle->data){
            SendDataContext * ctx = (SendDataContext *)handle->data;
            if(send_data_impl(ctx->peer, ctx->data, ctx->len) < 0){
                LOG_ERROR("send data failed");
                free(ctx->data);  ///如果没有发送出去，那么就不会调用after_write，这里需要对新分配的内存进行释放
            }
            free(ctx);
            LOG_DEBUG("free send_data_context");
            handle->data = NULL;  ///这里设置异步对象的关联对象为NULL，相当于标记为释放
        }
    }

    /**
     * 构造器
     * @param size
     */
    CAsyncs::CAsyncs(arc::CLoop * pCLoop, ushort size):m_pCLoop(pCLoop),m_size(size),m_identifier(0){
        for(ushort i = 0; i < m_size; ++i ){
            AsyncContext * asyncCtx = (AsyncContext *)malloc(sizeof(AsyncContext));
            asyncCtx->id = i;
            asyncCtx->async.data = NULL;
            uv_async_init(m_pCLoop->m_pLoop, &asyncCtx->async, async_send_callback);
            m_asyncQueue.push_back(asyncCtx);
        }
        LOG_DEBUG("Init asyncs with size %u",m_size);
    }

    /**
     * 析构必须在loop关闭后调用!
     */
    CAsyncs::~CAsyncs(){
        std::deque<AsyncContext *>::iterator beginIter = m_asyncQueue.begin();
        while(beginIter != m_asyncQueue.end()){
            AsyncContext * asyncCtx = *beginIter;
            free(asyncCtx);
            beginIter++;
        }
        m_asyncQueue.clear();
        LOG_DEBUG("Free asyncs with size %u",m_size);
    }

    /**
     * 设置识别码
     * @param identifier
     */
    void CAsyncs::setIdentifier(size_t identifier){
        this->m_identifier = identifier;
    }

    /**
     * 返回一个uv异步对象
     * 从队列头取元素，放回队列尾，每次只检查头部第一个元素是否已经释放，如果释放才能被再次使用
     * @return
     */
    uv_async_t * CAsyncs::acquireOneUvAsync(){
        arc::CScopeMutexLock scopeMutexLock(&m_mutexLock);
        if(m_asyncQueue.empty()){
            return NULL;
        }
        AsyncContext * asyncCtx = NULL;
        asyncCtx = m_asyncQueue[0];
        if(asyncCtx->async.data == NULL){  ///发现第一个被释放的异步对象
            LOG_DEBUG("acquire one async at %u", asyncCtx->id);
            m_asyncQueue.pop_front();
            m_asyncQueue.push_back(asyncCtx);
            return &asyncCtx->async;
        }else{
            LOG_ERROR("Not found available async object");
            //还没有被释放的异步对象
            return NULL;
        }
    }

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
    bool CAsyncs::acquireManyUvAsync(size_t num, std::vector<uv_async_t *> & result){
        arc::CScopeMutexLock scopeMutexLock(&m_mutexLock);
        if(m_asyncQueue.empty()){
            return false;
        }
        if(num>m_asyncQueue.size()){
            return false;
        }
        result.clear();
        size_t beginPos = 0;
        size_t maxPos   = m_asyncQueue.size()-1;
        bool found = false;
        while( beginPos+num <= maxPos && !found){
            bool broken = false;
            size_t prevId = 0L;
            for(size_t i=0; i < num; ++i){
                AsyncContext * asyncCtx = m_asyncQueue[i];
                if(asyncCtx->async.data != NULL || asyncCtx->id < prevId ){
                    broken = true;
                    break;
                }
                prevId = asyncCtx->id;
            }
            if(broken){  ///如果没有找到合适的连续异步对象，则移除队列头部的一个，继续寻找
                AsyncContext * asyncCtx = m_asyncQueue[0];
                m_asyncQueue.pop_front();
                m_asyncQueue.push_back(asyncCtx);
                beginPos++;
            }else{
                found = true;
            }
        }
        if(found){
            std::stringstream info;
            for(size_t i=0; i < num; ++i){
                AsyncContext * asyncCtx = m_asyncQueue[0];
                info << asyncCtx->id << ",";
                result.push_back(&asyncCtx->async);
                m_asyncQueue.pop_front();
                m_asyncQueue.push_back(asyncCtx);
            }
            LOG_DEBUG("Found %u available async objects : %s", num, info.str().c_str());
        }else{
            LOG_ERROR("Not found %u available async objects", num);
        }
        return found;
    }


    /**
     * 使用异步对象来发送数据给对端
     * @param async
     * @param peer
     * @param data
     * @param len
     */
    bool CAsyncs::sendDataWithAsync(uv_async_t * async, uv_stream_t* peer, const void * data, size_t len){
        if(async == NULL || NULL == peer || NULL == data || len == 0){
            return false;
        }
//        if(peer->data == NULL){
//            LOG_ERROR("connection is closed !!!");
//            return false;
//        }
        if(!arc::CChecker::instance().checkConnection(peer)){
            LOG_ERROR("connection is invalid !!!");
            return false;
        }
        //复制一份数据
        void * newdata = malloc(len);
        if(!newdata){
            LOG_ERROR("malloc newdata failed");
            return false;
        }
        memcpy(newdata, data, len);
        //
        SendDataContext * ctx = (SendDataContext*)malloc(sizeof(SendDataContext));
        ctx->data = newdata;
        ctx->len = len;
        ctx->peer = peer;
        async->data = ctx;
        uv_async_send(async);
        return true;
    }

    /**
     * 使用异步对象来发送数据给对端
     * @param peer
     * @param data
     * @param len
     */
    bool CAsyncs::sendDataWithAsync(uv_stream_t * peer, const void * data, size_t len){
//        if(peer->data == NULL){
//            LOG_ERROR("connection is closed !!!");
//            return false;
//        }
        if(!arc::CChecker::instance().checkConnection(peer)){
            LOG_ERROR("connection is invalid !!!");
            return false;
        }
        uv_async_t * async = this->acquireOneUvAsync();
        if(async==NULL){
            LOG_ERROR("Send failed");
            return false;
        }
        return this->sendDataWithAsync(async,peer,data,len);
    }

    /**
     * 发送Websocket消息
     * @param async
     * @param peer
     * @param buffer
     * @param len
     * @param op
     */
    bool CAsyncs::sendWebsocketMsg(uv_async_t * async, uv_stream_t * peer, const void * buffer, size_t len, uchar op){
//        if(peer->data == NULL){
//            LOG_ERROR("connection is closed !!!");
//            return false;
//        }
        if(!arc::CChecker::instance().checkConnection(peer)){
            LOG_ERROR("connection is invalid !!!");
            return false;
        }
        if( len == 0 || buffer == NULL){
            LOG_WARN("Data len is zero which need to send!");
            return false;
        }
        membuf_t buf;
        membuf_init(&buf, 128);
        generate_websocket_frame(&buf,buffer, len, op);
        bool ret = this->sendDataWithAsync(async,peer,buf.data,buf.size);
        membuf_uninit(&buf);
        return ret;
    }

    /**
     * 发送Websocket消息
     * @param peer
     * @param buffer
     * @param len
     * @param op
     */
    bool CAsyncs::sendWebsocketMsg(uv_stream_t * peer, const void * buffer, size_t len, uchar op){
//        if(peer->data == NULL){
//            LOG_ERROR("connection is closed !!!");
//            return false;
//        }
        if(!arc::CChecker::instance().checkConnection(peer)){
            LOG_ERROR("connection is invalid !!!");
            return false;
        }
        uv_async_t * async = this->acquireOneUvAsync();
        if(async==NULL){
            LOG_ERROR("acquire async failed");
            return false;
        }
        return this->sendWebsocketMsg(async,peer,buffer,len,op);
    }

    /**
     * 发送Websocket消息，带自定义分割策略
     * @param peer
     * @param buffer
     * @param len
     * @param op
     * @param splitDataLenFunction
     */
    bool CAsyncs::sendWebsocketMsg(uv_stream_t * peer, const void * buffer, size_t len, uchar op, SplitDataLenFunction splitDataLenFunction){
//        if(peer->data == NULL){
//            LOG_ERROR("connection is closed !!!");
//            return false;
//        }
        if(!arc::CChecker::instance().checkConnection(peer)){
            LOG_ERROR("connection is invalid !!!");
            return false;
        }
        if( len == 0 || buffer == NULL){
            LOG_WARN("Data len is zero which need to send!");
            return false;
        }

        if(splitDataLenFunction == NULL){
            return this->sendWebsocketMsg(peer,buffer,len,op);
        }

        membuf_t buf;
        membuf_init(&buf, 128);
        generate_websocket_frame(&buf, buffer, len, op);

        //调用分割函数获取分割结果
        split_result result;
        splitDataLenFunction(buf.size,&result);
        //获取连续的异步对象
        std::vector<uv_async_t *> asyncResult;
        if(!this->acquireManyUvAsync(result.split_count,asyncResult)){
            LOG_ERROR("acquire multi asyncs failed");
            membuf_uninit(&buf);
            return false;
        }
        uchar * dataptr = buf.data;
        for(int i=0; i<result.split_count; ++i){
            LOG_DEBUG("send %u", result.split_result[i]);
            uv_async_t * async = asyncResult[i];
            bool ret = this->sendDataWithAsync(async,peer,dataptr,result.split_result[i]);
            if(!ret){
                LOG_ERROR("send part %d failed",i);
                return false;
            }
            dataptr += result.split_result[i];
        }

        membuf_uninit(&buf);
        return true;
    }


    /**
     * 使用异步对象来发送数据给对端，基于自定义分割策略
     * @param peer
     * @param data
     * @param len
     * @param splitDataLenFunction
     */
    bool CAsyncs::sendDataWithAsync(uv_stream_t * peer, const void * data, size_t len, SplitDataLenFunction splitDataLenFunction){
//        if(peer->data == NULL){
//            LOG_ERROR("connection is closed !!!");
//            return false;
//        }
        if(!arc::CChecker::instance().checkConnection(peer)){
            LOG_ERROR("connection is invalid !!!");
            return false;
        }
        if( len == 0 || data == NULL){
            LOG_WARN("Data len is zero which need to send!");
            return false;
        }

        if(splitDataLenFunction == NULL){
            return this->sendDataWithAsync(peer,data,len);
        }

        //调用分割函数获取分割结果
        split_result result;
        splitDataLenFunction(len,&result);
        //获取连续的异步对象
        std::vector<uv_async_t *> asyncResult;
        if(!this->acquireManyUvAsync(result.split_count,asyncResult)){
            LOG_ERROR("acquire multi asyncs failed");
            return false;
        }
        uchar * dataptr = (uchar *)data;
        for(int i=0; i<result.split_count; ++i){
            LOG_DEBUG("send %u", result.split_result[i]);
            uv_async_t * async = asyncResult[i];
            bool ret = this->sendDataWithAsync(async,peer,dataptr,result.split_result[i]);
            if(!ret){
                LOG_ERROR("send part %d failed",i);
                return false;
            }
            dataptr += result.split_result[i];
        }

        return true;
    }


}
