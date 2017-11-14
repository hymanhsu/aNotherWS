/**
 * @author haisheng.yhs
 * @description
 *      create a client and connect to server,
 *      send end msg to server to ask server and client all closed
 * @version 1.0, initial version, 2017/05/16
 *
 */


#include "ws/membuf.h"
#include "ws/uvutil.h"
#include "ws/CWSClient.h"
#include "test_header.h"
#include "simple_log.h"

#include <string>

#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>


class MyCWSClient : public arc::CWSClient{

public:
    MyCWSClient(const std::string& ip, ushort port):
            arc::CWSClient(ip,port){}
    ~MyCWSClient(){}

    int result = 0;
    bool isRun = true;

    void onData(websocket_handle* handle, uv_stream_t* peer, void * buffer, size_t bufferLen, uchar type){
        LOG_INFO("-------------------------------------------response len=%lu-------------------------------------------\n", bufferLen);
        if( ISSET_FLAG(type,FLAG_WEBSOCKET_TEXT) ){
            LOG_DEBUG("this is one text message\n");
        }else if(ISSET_FLAG(type,FLAG_WEBSOCKET_BIN)){
            LOG_DEBUG("this is one binary message\n");
        }
    }

    void onError(websocket_handle* handle, uv_stream_t* peer, int errcode, const char * errstr){
        LOG_ERROR("error ocuurs : %d, %s\n", errcode, errstr);
        this->isRun = false;
        this->result = 1;
    }

    void onClose(websocket_handle* handle, uv_stream_t* peer){
        LOG_INFO("close connection\n");
    }

    void onConnect(uv_stream_t* peer){
        LOG_INFO("open connection\n");
    }

    void sendData() {
        unsigned char *buffer = (unsigned char*)calloc(1, this->sizeMsg+5);

        LOG_INFO("wait 10 seconds to ensure event loop started well\n");
        sleep(10);
        //end end msg, actual size = buffer head(4) + msg real size(8)
        unsigned long size = this->sizeMsg;
        buffer[3] = (size >> 8*3) & 0xFF;
        buffer[2] = (size >> 8*2) & 0xFF;
        buffer[1] = (size >> 8*1) & 0xFF;
        buffer[0] = size & 0xFF;
        strncpy((char*)buffer+4, this->msgEnd, size);
        this->sendMsgWithAsync(this->m_peer, buffer, 4+size, WS_BIN_FRAME, NULL);
        LOG_INFO("free buffer. testing end....");
        free(buffer);
        buffer = NULL;
    }

private:
    char* msgEnd = "++end++";
    int sizeMsg = sizeof(msgEnd)/sizeof(msgEnd[0]);
};


int main(int argc, char** argv) {

    LOG_INFO("client ARG[1]: %s", argv[1]);
    ushort port = std::stoi(argv[2]);
    LOG_INFO("client ARG[2]: %d", port);

    MyCWSClient* wsclient = new MyCWSClient(argv[1], port);
    wsclient->startup();
    wsclient->sendData();
    sleep(10);
    int result = wsclient->result;
    wsclient->shutdown();
    delete wsclient;
    LOG_INFO("client delete wsclient, result: %d", result);

    return result;
}


