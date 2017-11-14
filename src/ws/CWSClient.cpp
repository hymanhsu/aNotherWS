/**
* @file       CWSClient.cpp
* @brief      Websocket客户端的实现
* @details
* @author     Kevin.XU
* @date       2017.5.4
* @version    1.0.0
* @par Copyright (c):
*      ARC
* @par History:
*   1.0.0: Kevin.XU, 2017.5.4, Create\n
*/


#include <ws/CWSClient.h>
#include <ws/uvutil.h>
#include <ws/wsutil.h>
#include <ws/common.h>
#include <ws/CChecker.h>
#include <simple_log.h>


namespace arc {


    static void reconnect_timer_callback (uv_timer_t* handle){
        LOG_DEBUG("begin to reconnect ...");
        void * data = handle->loop->data;
        if(data){
            CWSClient * client = (CWSClient *)data;
            client->connect();
        }
        LOG_DEBUG("reconnect done.");
    }


    static void hb_timer_callback (uv_timer_t* handle){
        uchar *p = generate_ping_control_frame();
        void * data = handle->loop->data;
        if(data){
            CWSClient * client = (CWSClient *)data;
            client->sendDataWithAsync(client->m_peer, p, 2);
        }
        LOG_DEBUG("Sent one ping");
    }

    static char on_close(uv_stream_t* peer){
        if(!peer->loop->data){
            CWSClient * wsClient = (CWSClient *)peer->loop->data;
            wsClient->onClose((websocket_handle*)peer->data,peer);
        }
        return 0;
    }

    //on_read_WebSocket
    static void on_read_websocket(CWSClient * wsclient, websocket_handle* hd, uv_stream_t* peer, char* data, size_t len) {
        char * dataptr = data;
        size_t haveReadCount = 0L;
        do{
            try_parse_protocol(hd, dataptr, len-haveReadCount);
            if(hd->parseState == PARSE_COMPLETE){ //正常解析到了数据包
#ifdef CLIENT_DATA_RECORD_ENABLE
                save_proto_data(CLIENT_DATA_PATH"/proto/",hd->protoBuf.data,hd->protoBuf.size);
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
#ifdef CLIENT_DATA_RECORD_ENABLE
                            save_proto_data(CLIENT_DATA_PATH"/merge/",hd->mergeBuf.data,hd->mergeBuf.size);
#endif
                            //接收数据回调
                            wsclient->onData(hd, peer, hd->mergeBuf.data, hd->mergeBuf.size, hd->mergeBuf.flag);
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
                            wsclient->onData(hd, peer, hd->protoBuf.data, hd->protoBuf.size, hd->protoBuf.flag);
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
                            wsclient->onData(hd, peer, hd->protoBuf.data, hd->protoBuf.size, hd->protoBuf.flag);
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
                        wsclient->sendDataWithAsync(peer, p, 2);
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
                //break;
            }
            haveReadCount += hd->readCount;
            dataptr += hd->readCount;
        }while( haveReadCount < len );
    }


    static void on_read(uv_stream_t* peer, ssize_t nread, const uv_buf_t* buf) {
        LOG_DEBUG("<<<<<<<<<<<received data len = %lu",nread);
        websocket_handle* hd;
        if (peer->data)
            hd = (websocket_handle*)peer->data;
        else {
            hd = (websocket_handle*)calloc(1, sizeof(websocket_handle));
            hd->endpointType = ENDPOINT_CLIENT;
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
        CWSClient * wsclient = (CWSClient *)ctx;
        if(nread > 0) {
            switch(wsclient->m_WSClientStatus){
                case HANDSHAKING:
                {
                    char * key = parse_http_heads(buf, "Sec-WebSocket-Accept:");
                    if(key && strlen(key)>0){
                        LOG_DEBUG("handshake response");
                        wsclient->m_WSClientStatus = WORKING;
                        wsclient->onConnect(peer);
                    }
                    break;
                }
                case WORKING:
                {
                    on_read_websocket(wsclient,hd,peer,buf->base,nread);
                    break;
                }
            }
        } else if(nread == 0) {
            LOG_DEBUG("libuv requested a buffer through the alloc callback, but then decided that it didn't need that buffer");
        } else {
            if (nread != UV_EOF) {
                char errstr[60] = { 0 };
                sprintf(errstr, "%d:%s,%s", (int)nread, uv_err_name((int)nread), uv_strerror((int)nread));
                wsclient->onError(hd, peer, nread, errstr);
            }else{
                LOG_WARN("the remote endpoint is offline");
            }
            tw_close_client(peer,on_close);
            //重连
            if(wsclient->m_enableReconnect){
                uv_timer_start(&wsclient->m_reconnect_uv_timer_t, reconnect_timer_callback, 10*1000, 30*1000); //30秒一次
            }
        }
        //每次使用完要释放
        if (buf->base)
            free(buf->base);
    }

    static void alloc_cb(uv_handle_t* handle, size_t size, uv_buf_t* buf) {
        buf->base = static_cast<char *>(malloc(size));
        buf->len = size;
    }

    /**
    * 启动反调函数
     * @param manager
    */
    static void client_engine_run(void * data) {
        if(data){
            CWSClient * client = (CWSClient *)data;
            LOG_DEBUG("ws client start");
            client->m_pCLoop->run();
            LOG_DEBUG("ws client end");
        }
    }

    /**
    * @brief 初始化
    */
    CWSClient::CWSClient(const std::string& ip, ushort port, size_t asyncSize, bool enableReconnect, const std::string& uri, bool enableHeartBeat):
        m_ip(ip), m_port(port), m_uri(uri), m_enableHeartBeat(enableHeartBeat), m_enableReconnect(enableReconnect)
        {
        m_pCLoop = new arc::CLoop("WebsocketClient");
        this->m_pCLoop->m_pLoop->data = this;  ///把客户单对象保存到loop关联数据上
        m_pCAsyncs = new arc::CAsyncs(this->m_pCLoop,asyncSize);
        m_pCThread = new arc::CThread(this,client_engine_run);
    }


    CWSClient::~CWSClient() {
        delete m_pCLoop;
        delete m_pCThread;
        delete m_pCAsyncs;
    }


    void CWSClient::onConnectInternal(uv_connect_t *connection, int status) {
        if(0 == status){
            LOG_DEBUG("connected.");
            void * data =  connection->handle->loop->data;
            if(data){
                CWSClient * client = (CWSClient *)data;
                if(client->m_enableReconnect){
                    //既然连接成功，就把重连计时器关闭掉
                    uv_timer_stop(&client->m_reconnect_uv_timer_t);
                }
                connection->handle->data = NULL;  ///将连接的关联句柄初始化为NULL
                client->m_peer = connection->handle;  ///将对端连接对象保存下来
                if (!client->m_peer->data){  ///如果没有设置websocket_handle就先设置一次，否则一个消息也发送不出去
                    websocket_handle* hd = (websocket_handle*)calloc(1, sizeof(websocket_handle));
                    hd->endpointType = ENDPOINT_CLIENT;
                    client->m_peer->data = hd;
                }
                arc::CChecker::instance().registerConnection( connection->handle );
                //Websocket握手
                char random[16];
                get_random(random,16);
                char * key = sha1_and_base64(random,16);
                client->m_key = key;
                char * p = generate_websocket_client_handshake(client->m_ip.c_str(),
                                                               client->m_port,client->m_uri.c_str(),client->m_key.c_str());
                client->sendDataWithAsync(connection->handle, p, strlen(p));
                client->m_WSClientStatus = HANDSHAKING;
                free(p);
                free(key);
                LOG_DEBUG("handshake request");
                //注册读事件
                uv_read_start(connection->handle, alloc_cb, on_read);
            }
        }else{
            LOG_ERROR("connect failed : %s",uv_strerror(status));
        }
    }


    void CWSClient::connect() {
        int ret = uv_tcp_init(this->m_pCLoop->m_pLoop, &this->m_uv_tcp_t);
        if (ret < 0)
            return;
        struct sockaddr_in addr;
        uv_ip4_addr(this->m_ip.c_str(), this->m_port, &addr);
        uv_tcp_connect(&this->m_uv_connect_t, &this->m_uv_tcp_t, (const struct sockaddr*) &addr, this->onConnectInternal);
    }

    void CWSClient::startup() {
        ///初始化心跳和重连
        uv_timer_init(this->m_pCLoop->m_pLoop, &this->m_hb_uv_timer_t);
        uv_timer_init(this->m_pCLoop->m_pLoop, &this->m_reconnect_uv_timer_t);
        if(m_enableHeartBeat){
            uv_timer_start(&this->m_hb_uv_timer_t, hb_timer_callback, 5*60*1000, 2*60*1000); //120秒一次
        }
        if(m_enableReconnect){
            uv_timer_start(&this->m_reconnect_uv_timer_t, reconnect_timer_callback, 60*1000, 60*1000); //60秒一次
        }

        ///连接
        connect();
        ///
        m_pCThread->run();
    }


    void CWSClient::shutdown() {
        if(m_enableHeartBeat){
            uv_timer_stop(&this->m_hb_uv_timer_t);
        }
        if(m_enableReconnect){
            uv_timer_stop(&this->m_reconnect_uv_timer_t);
        }
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
    bool CWSClient::sendMsgWithAsync(uv_stream_t * peer, const void * msg, size_t len, uchar op) {
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
    bool CWSClient::sendMsgWithAsync(uv_stream_t* peer, const void * msg, size_t len, uchar op, SplitDataLenFunction splitDataLenFunction){
        return this->m_pCAsyncs->sendWebsocketMsg(peer,msg,len,op,splitDataLenFunction);
    }

    /**
     * 发送数据给对端
     * @param peer
     * @param data
     * @param len
     */
    bool CWSClient::sendDataWithAsync(uv_stream_t* peer, const void * data, size_t len){
        return this->m_pCAsyncs->sendDataWithAsync(peer,data,len);
    }

    /**
     * 基于分割策略发送数据给对端
     * @param peer
     * @param data
     * @param len
     * @param splitDataLenFunction
     */
    bool CWSClient::sendDataWithAsync(uv_stream_t* peer, const void * data, size_t len, SplitDataLenFunction splitDataLenFunction){
        return this->m_pCAsyncs->sendDataWithAsync(peer,data,len,splitDataLenFunction);
    }

}

