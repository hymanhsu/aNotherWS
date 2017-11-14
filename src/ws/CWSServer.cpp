/**
* @file       CWSServer.cpp
* @brief      Websocket服务端的实现
* @details
* @author     Kevin.XU
* @date       2017.5.4
* @version    1.0.0
* @par Copyright (c):
*      ARC
* @par History:
*   1.0.0: Kevin.XU, 2017.5.4, Create\n
*/



#include <ws/CWSServer.h>
#include <ws/uvutil.h>
#include <ws/wsutil.h>
#include <ws/common.h>
#include <ws/CChecker.h>
#include <simple_log.h>


namespace arc {



    static char on_close(uv_stream_t* peer){
        if(!peer->loop->data){
            CWSServer * wsServer = (CWSServer *)peer->loop->data;
            wsServer->onClose((websocket_handle*)peer->data,peer);
        }
        return 0;
    }

    static void on_read_websocket(CWSServer * wsserver, uv_stream_t* peer, websocket_handle* hd, char* data, size_t len) {
        char * dataptr = data;
        size_t haveReadCount = 0L;
        do{
            try_parse_protocol(hd, dataptr, len-haveReadCount);
            if(hd->parseState == PARSE_COMPLETE){ //正常解析到了数据包
#ifdef SERVER_DATA_RECORD_ENABLE
                save_proto_data(SERVER_DATA_PATH"/proto/",hd->protoBuf.data,hd->protoBuf.size);
#endif
                switch (hd->type) {
                    case 0: //0x0表示附加数据帧
                    {
                        if(hd->isEof == 0) { //这是中间的分片
                            LOG_DEBUG("continuous fragment, len = %u", hd->protoBuf.size);
                            membuf_append_data(&hd->mergeBuf,hd->protoBuf.data,hd->protoBuf.size);
                        }else{ //最后一个分片
                            LOG_DEBUG("last fragment, len = %u", hd->protoBuf.size);
                            membuf_append_data(&hd->mergeBuf,hd->protoBuf.data,hd->protoBuf.size);
#ifdef SERVER_DATA_RECORD_ENABLE
                            save_proto_data(SERVER_DATA_PATH"/merge/",hd->mergeBuf.data,hd->mergeBuf.size);
#endif
                            //接收数据回调
                            wsserver->onData(hd, peer, hd->mergeBuf.data, hd->mergeBuf.size, hd->mergeBuf.flag);
                        }
                        break;
                    }
                    case 1: //0x1表示文本数据帧
                    {
                        SET_FLAG(hd->protoBuf.flag,FLAG_WEBSOCKET_TEXT);
                        if(hd->isEof == 0){ //这是第一个分片
                            LOG_DEBUG("first text fragment, len = %u", hd->protoBuf.size);
                            membuf_clear(&hd->mergeBuf,0);  //清理原来的缓冲数据
                            SET_FLAG(hd->mergeBuf.flag,FLAG_WEBSOCKET_TEXT);
                            membuf_append_data(&hd->mergeBuf,hd->protoBuf.data,hd->protoBuf.size);
                        }else{ //就一个分片
                            LOG_DEBUG("one text datapacket, len = %u", hd->protoBuf.size);
                            //接收数据回调
                            wsserver->onData(hd, peer, hd->protoBuf.data, hd->protoBuf.size, hd->protoBuf.flag);
                        }
                        break;
                    }
                    case 2: //0x2表示二进制数据帧
                    {
                        SET_FLAG(hd->protoBuf.flag, FLAG_WEBSOCKET_BIN);
                        if(hd->isEof == 0) { //这是第一个分片
                            LOG_DEBUG("first bin fragment, len = %u", hd->protoBuf.size);
                            membuf_clear(&hd->mergeBuf,0);  //清理原来的缓冲数据
                            SET_FLAG(hd->mergeBuf.flag,FLAG_WEBSOCKET_BIN);
                            membuf_append_data(&hd->mergeBuf,hd->protoBuf.data,hd->protoBuf.size);
                        }else{ //就一个分片
                            LOG_DEBUG("one bin datapacket, len = %u", hd->protoBuf.size);
                            //接收数据回调
                            wsserver->onData(hd, peer, hd->protoBuf.data, hd->protoBuf.size, hd->protoBuf.flag);
                        }
                        break;
                    }
                    case 3: case 4: case 5: case 6: case 7: //0x3 - 7暂时无定义，为以后的非控制帧保留
                        //DONOTHIND
                        break;
                    case 8: //0x8表示连接关闭
                    {
                        LOG_DEBUG("received websocket control frame : close");
                        tw_close_client(peer,on_close);
                        break;
                    }
                    case 9: //0x9表示ping
                    {
                        LOG_DEBUG("received websocket control frame : ping");
                        uchar *p = generate_pong_control_frame();
                        wsserver->sendDataWithAsync(peer, p, 2);
                        LOG_DEBUG("sent websocket control frame : pong");
                        break;
                    }
                    case 10://0xA表示pong
                    default://0xB - F暂时无定义，为以后的控制帧保留
                        break;
                }
            }else if(hd->parseState == PARSE_FAILED){ //解析错误，关闭连接
                tw_close_client(peer,on_close);
            }else { // 未读完一个完整的包,等待下回继续读
                //DONOHTING
                break;
            }
            haveReadCount += hd->readCount;
            dataptr += hd->readCount;
        }while( haveReadCount < len );
    }


    static void on_read(uv_stream_t* peer, ssize_t nread, const uv_buf_t* buf) {
        LOG_DEBUG(">>>>>>>>>>received data len = %lu",nread);
        websocket_handle* hd;
        if (peer->data)
            hd = (websocket_handle*)peer->data;
        else {
            hd = (websocket_handle*)calloc(1, sizeof(websocket_handle));
            hd->endpointType = ENDPOINT_SERVER;
            hd->parseState = PARSE_COMPLETE;
            peer->data = hd;
        }
        if(NULL == hd->protoBuf.data){
            membuf_init(&hd->protoBuf, 128);
        }
        if(NULL == hd->mergeBuf.data){
            membuf_init(&hd->mergeBuf, 128);
        }
        void * ctx = peer->loop->data;
        if(!ctx){
            return;
        }
        CWSServer * wsserver = (CWSServer *)ctx;
        if (nread > 0) {
            if ( ISSET_FLAG(hd->protoBuf.flag,FLAG_WEBSOCKET) )
            {
                //WebSocket握手已经通过
                on_read_websocket(wsserver, peer, hd, buf->base, nread);
            }
            else
            {
                //WebSocket握手
                char* p,*p2;
                //get Sec-WebSocket-Key ?
                p = parse_http_heads(buf, "Sec-WebSocket-Key:");
                if (p) {
                    //WebSocket 握手
                    LOG_DEBUG("received one websocket handshake request");
                    SET_FLAG(hd->protoBuf.flag,FLAG_WEBSOCKET);
                    //SET_FLAG(hd->mergeBuf.flag,FLAG_WEBSOCKET);
                    p2 = generate_websocket_server_handshake(p);
                    wsserver->sendDataWithAsync(peer, p2, strlen(p2));
                    free(p2);
                    LOG_DEBUG("sent one websocket handshake response");
                    //客户端接入回调
                    wsserver->onConnect(peer);
                }
                else
                { //不是握手协议，关闭连接
                    tw_close_client(peer,on_close);
                }
            }
        }
        else if(nread == 0)
        {
            LOG_DEBUG("libuv requested a buffer through the alloc callback, but then decided that it didn't need that buffer");
        }
        else
        {
            //在任何情况下出错, read 回调函数 nread 参数都<0，如：出错原因可能是 EOF(遇到文件尾)
            if (nread != UV_EOF) {
                char errstr[60] = { 0 };
                sprintf(errstr, "%d:%s,%s", (int)nread, uv_err_name((int)nread), uv_strerror((int)nread));
                wsserver->onError(hd, peer, nread, errstr);
            }else{
                LOG_WARN("the remote endpoint is offline");
            }
            //关闭连接
            tw_close_client(peer,on_close);
        }
        //每次使用完要释放
        if (buf->base)
            free(buf->base);
    }

    static void on_uv_alloc(uv_handle_t* handle, size_t suggested_size, uv_buf_t* buf) {
        buf->base = (char*)calloc(1,suggested_size);
        buf->len = suggested_size;
    }

    static void close_client(uv_handle_t* client){
        free(client);
    }

    /**
     * 客户端接入反调函数
     * @param server
     * @param status
     */
    static void on_connection(uv_stream_t* server, int status) {
        if (status == 0) {
            LOG_DEBUG("some client connected.");
            void * data =  server->loop->data;
            if(data){
                CWSServer * wsserver = (CWSServer *)data;
                //建立客户端信息,在关闭连接时释放
                uv_tcp_t* client = (uv_tcp_t*)malloc(sizeof(uv_tcp_t));
                client->data = NULL;

                //将客户端放入loop
                uv_tcp_init(server->loop, client);

                //接受客户，保存客户端信息
                if(uv_accept(server, (uv_stream_t*)client) == 0){
                    arc::CChecker::instance().registerConnection( (uv_stream_t*)client );
                    client->close_cb = after_uv_close_client;
                    //开始读取客户端数据
                    uv_read_start((uv_stream_t*)client, on_uv_alloc, on_read);
                }else{
                    uv_close((uv_handle_t*) client, close_client);
                }
            }
        }
    }


    /**
    * 启动反调函数
     * @param manager
    */
    static void server_engine_run(void * data) {
        if(data){
            CWSServer * server = (CWSServer *)data;
            LOG_DEBUG("ws server start , listen at %s:%u", server->m_ip.c_str(), server->m_port);
            server->m_pCLoop->run();
            LOG_DEBUG("ws server end");
        }
    }

    /**
    * @brief 初始化
    */
    CWSServer::CWSServer(const std::string& ip, ushort port, size_t asyncSize):
            m_ip(ip), m_port(port)
    {
        m_pCLoop = new arc::CLoop("WebsocketServer");
        this->m_pCLoop->m_pLoop->data = this;  ///把服务端对象保存到loop关联数据上
        m_pCAsyncs = new arc::CAsyncs(this->m_pCLoop,asyncSize);
        m_pCThread = new arc::CThread(this,server_engine_run);
    }


    CWSServer::~CWSServer() {
        delete m_pCLoop;
        delete m_pCThread;
        delete m_pCAsyncs;
    }


    /**
     * 启动
     */
    void CWSServer::startup() {
        struct sockaddr_in addr;
        uv_ip4_addr(this->m_ip.c_str(), this->m_port, &addr);
        int ret = uv_tcp_init(this->m_pCLoop->m_pLoop, &this->m_uv_tcp_t);
        if (ret < 0)
            return;
        ret = uv_tcp_bind(&this->m_uv_tcp_t, (const struct sockaddr*) &addr, 0);
        if (ret < 0)
            return;
        ret = uv_listen((uv_stream_t*)(&this->m_uv_tcp_t), 8, on_connection);
        if (ret < 0)
            return;
        ///
        m_pCThread->run();
    }

    /**
     * join该线程
     */
    void CWSServer::join() {
        m_pCThread->join();
    }

    /**
     * 关闭
     */
    void CWSServer::shutdown() {
        m_pCLoop->stop();
        m_pCThread->stop();
    }

    /**
     * 发送消息给对端
     * @param peer
     * @param data
     * @param len
     * @param op
     */
    bool CWSServer::sendMsgWithAsync(uv_stream_t * peer, const void * msg, size_t len, uchar op) {
        return this->m_pCAsyncs->sendWebsocketMsg(peer,msg,len,op);
    }

    /**
     * 基于分割策略发送消息给对端
     * @param peer
     * @param msg
     * @param len
     * @param op
     * @param splitDataLenFunction
     */
    bool CWSServer::sendMsgWithAsync(uv_stream_t* peer, const void * msg, size_t len, uchar op, SplitDataLenFunction splitDataLenFunction){
        return this->m_pCAsyncs->sendWebsocketMsg(peer,msg,len,op,splitDataLenFunction);
    }

    /**
     * 发送数据给对端
     * @param peer
     * @param data
     * @param len
     */
    bool CWSServer::sendDataWithAsync(uv_stream_t* peer, const void * data, size_t len){
        return this->m_pCAsyncs->sendDataWithAsync(peer,data,len);
    }

    /**
     * 基于分割策略发送数据给对端
     * @param peer
     * @param data
     * @param len
     * @param splitDataLenFunction
     */
    bool CWSServer::sendDataWithAsync(uv_stream_t* peer, const void * data, size_t len, SplitDataLenFunction splitDataLenFunction){
        return this->m_pCAsyncs->sendDataWithAsync(peer,data,len,splitDataLenFunction);
    }


}
