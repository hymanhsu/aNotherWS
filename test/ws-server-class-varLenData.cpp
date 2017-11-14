//
// Created by haisheng.yhs on 2017/5/5.
//


#include "test_header.h"
#include "ws/CWSServer.h"
#include "simple_log.h"

#include <stdlib.h>
#include <string.h>
#include <assert.h>

class MyCWSServer : public arc::CWSServer{

public:
    MyCWSServer(const std::string& ip, ushort port):
            arc::CWSServer(ip,port){}
    ~MyCWSServer(){}

    void onConnect(uv_stream_t* peer){
        LOG_INFO("One client comming ....\n");
    }

    void onClose(websocket_handle* hd, uv_stream_t* peer){
        LOG_INFO("One client exiting ....\n");
    }

    void onData(websocket_handle* handle, uv_stream_t* peer, void * buffer, size_t bufferLen, uchar type){
        LOG_DEBUG("-------------------------------------------response len=%lu-------------------------------------------\n", bufferLen);
        if( ISSET_FLAG(type,FLAG_WEBSOCKET_TEXT) ){
            printf("this is one text message\n");
        }else if(ISSET_FLAG(type,FLAG_WEBSOCKET_BIN)){
            printf("this is one binary message\n");
        }

        verifyData(buffer, bufferLen);

        if( ISSET_FLAG(type,FLAG_WEBSOCKET_TEXT) ){
            //this->sendMsgWithAsync(peer, buffer, bufferLen, WS_TEXT_FRAME);
            this->sendMsgWithAsync(peer, buffer, bufferLen, WS_TEXT_FRAME,
                                   [](size_t inputLen, split_result * result){
                                       split_ont_size_to_multi_parts(inputLen,3,result);});
        }else if(ISSET_FLAG(type,FLAG_WEBSOCKET_BIN)){
            //this->sendMsgWithAsync(peer, buffer, bufferLen, WS_BIN_FRAME);
            this->sendMsgWithAsync(peer, buffer, bufferLen, WS_BIN_FRAME,
                                   [](size_t inputLen, split_result * result){
                                       split_ont_size_to_multi_parts(inputLen,3,result);});
        }

    }

    void verifyData(void * buffer, size_t bufferLen) {
        //verify data
        unsigned long LEN = 123;
        char fill_char = 'a';

        //get data len, buffer, first 32 bits stored data len
        unsigned char size[4];
        memset(size, 0, 4);
        memcpy(size, (unsigned char*)buffer, 4);
        unsigned long dataExpectSize = size[3] << 3*8;
        dataExpectSize += size[2] << 2*8;
        dataExpectSize += size[1] << 1*8;
        dataExpectSize += size[0];

        LOG_INFO("dataExpectSize: %lu\n", dataExpectSize);
        if (dataExpectSize == bufferLen) {
            unsigned long dataSize = dataExpectSize - 4; //strip off first 4 bytes
            unsigned char fill_char = '!';
            unsigned char actual_char = ((unsigned char*)buffer)[4];
            bool isFindChar = true;

            LOG_INFO("actual char array: %c\n", actual_char);
            while (fill_char != actual_char) {
                if ('~' == fill_char) {
                    //the last char has been checked, still not same, report FAIL directly
                    LOG_ERROR("[FAIL] sent and received data NOT same\n");
                    isFindChar = false;
                    break;
                }
                fill_char++;
            }
            if (isFindChar) {
                //compare data
                unsigned char* expectData = (unsigned char*)calloc(1, dataSize);
                memset(expectData, fill_char, dataSize);

                if (0 == memcmp(expectData, (unsigned char*)buffer+4, dataSize)) {
                    LOG_INFO("[SUCCESS] sent and received data same\n");
                } else {
                    LOG_ERROR("[FAIL] sent and received data NOT same\n");
                    LOG_INFO("expect %lu chars %c\n", bufferLen, fill_char);
                    free(expectData);
                    expectData = NULL;
                    LOG_INFO("EXIT 1");
                    exit(EXIT_FAILURE);
                }
                free(expectData);
                expectData = NULL;
            } else {
                LOG_ERROR("[FAIL] sent and received data NOT same\n");
                LOG_INFO("expect %lu chars %c\n", bufferLen, actual_char);
                LOG_INFO("EXIT 1");
                exit(EXIT_FAILURE);
            }
        } else {
            LOG_ERROR("[FAIL] sent and received data size NOT same\n");
            LOG_INFO("EXIT 1");
            exit(EXIT_FAILURE);
        }
    }

    void onError(websocket_handle* handle, uv_stream_t* peer, int errcode, const char * errstr){
        LOG_ERROR("error ocuurs : %d, %s\n", errcode, errstr);
    }

};

MyCWSServer * wsserver;

int main(int argc, char** argv) {

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

