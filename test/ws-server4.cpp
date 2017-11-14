//
// Created by xuhuahai on 2017/4/24.
//


#include "test_header.h"
#include "ws/CWSServer.h"
#include "simple_log.h"
#include "test_header.h"

#include <stdlib.h>
#include <string.h>



class MyCWSServer : public arc::CWSServer{

public:
    MyCWSServer(const std::string& ip, ushort port):
            arc::CWSServer(ip,port){}
    ~MyCWSServer(){}

    void onConnect(uv_stream_t* peer){
        LOG_ERROR("One client comming ....\n");
    }

    void onClose(websocket_handle* hd, uv_stream_t* peer){
        LOG_ERROR("One client exiting ....\n");
    }

    void onData(websocket_handle* handle, uv_stream_t* peer, void * buffer, size_t bufferLen, uchar type){
        LOG_DEBUG("-------------------------------------------request len=%lu-------------------------------------------\n", bufferLen);
        if( ISSET_FLAG(type,FLAG_WEBSOCKET_TEXT) ){
            printf("this is one text message\n");
        }else if(ISSET_FLAG(type,FLAG_WEBSOCKET_BIN)){
            printf("this is one binary message\n");
        }
        printx(buffer, bufferLen < 256 ? bufferLen : 256);
        //save_proto_data(saved_data_directory,buffer,bufferLen);
        if( ISSET_FLAG(type,FLAG_WEBSOCKET_TEXT) ){
            this->sendMsgWithAsync(peer, buffer, bufferLen, WS_TEXT_FRAME);
        }else if(ISSET_FLAG(type,FLAG_WEBSOCKET_BIN)){
            this->sendMsgWithAsync(peer, buffer, bufferLen, WS_BIN_FRAME);
        }
        if(bufferLen != 2000){
            LOG_DEBUG("Maybe occurs some error !!!!! \n");
            exit(-1);
        }

    }

    void onError(websocket_handle* handle, uv_stream_t* peer, int errcode, const char * errstr){
        LOG_ERROR("error ocuurs : %d, %s\n", errcode, errstr);
        exit(-1);
    }

};

MyCWSServer * wsserver;

int main(int argc, char** argv) {

    printf("u32 = %lu\n", sizeof(uint));

    wsserver = new MyCWSServer("0.0.0.0",8080);

    wsserver->startup();


    while(1){
        unsigned char ch = getchar();
        if(ch == 'Q' || ch == 'q'){
            break;
        }
        if(ch == '\n' || ch == 255){
            continue;
        }
    }

    wsserver->shutdown();
    delete wsserver;
    return 0;
}

