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
            arc::CWSServer(ip,port,500000){}
    ~MyCWSServer(){}

    int result = 0;
    bool isRun = true;

    void onConnect(uv_stream_t* peer){
        LOG_INFO("One client comming ....\n");
    }

    void onClose(websocket_handle* hd, uv_stream_t* peer){
        LOG_INFO("One client exiting ....\n");
    }

    void onData(websocket_handle* handle, uv_stream_t* peer, void * buffer, size_t bufferLen, uchar type){
        LOG_INFO("-------------------------------------------response len=%lu-------------------------------------------\n", bufferLen);
        if( ISSET_FLAG(type,FLAG_WEBSOCKET_TEXT) ){
            LOG_DEBUG("this is one text message\n");
        }else if(ISSET_FLAG(type,FLAG_WEBSOCKET_BIN)){
            LOG_DEBUG("this is one binary message\n");
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

    void onError(websocket_handle* handle, uv_stream_t* peer, int errcode, const char * errstr){
        LOG_ERROR("error ocuurs : %d, %s\n", errcode, errstr);
        this->isRun = false;
        exit(1);
    }

private:
    int LEN_TEST = 123;
    char* msgEnd = "++end++";
    int sizeMsg = sizeof(msgEnd)/sizeof(msgEnd[0]);

    //check buffer store chars if correct
    void checkBufferChars(const char expect, const void* buffer, unsigned long bufferSize) {
        for (unsigned long i = 0; i <  bufferSize; i++) {
            if (((char*)buffer)[i] != expect) {
                LOG_ERROR("[FAIL] server chars in your memory NOT all same, differ: ((char*)buffer)[%lu] == %c", i, ((char*)buffer)[i]);
            }
        }
    }

    //verify data which received from client
    void verifyData(void * buffer, size_t bufferLen) {

        //buffer, the first 4 bytes from head, stored data total size( header 4bytes + char size bytes)
        unsigned char size[4];
        memset(size, 0, 4);
        memcpy(size, (unsigned char*)buffer, 4);
        //convert buffer total size from the first 4 bytes to local var
        unsigned long dataExpectSize = size[3] << 3*8;
        dataExpectSize += size[2] << 2*8;
        dataExpectSize += size[1] << 1*8;
        dataExpectSize += size[0];

        LOG_INFO("dataExpectSize: %lu\n", dataExpectSize);
        if (dataExpectSize == bufferLen) {
            unsigned long dataSize = dataExpectSize - 4; //strip off first 4 bytes
            unsigned char actual_char = ((unsigned char*)buffer)[4];

            LOG_INFO("actual char array: %c\n", actual_char);
            if(actual_char < '!' || actual_char > '~'){
                //the last char has been checked, still not same, report FAIL directly
                LOG_ERROR("[FAIL] server do NOT find expect char in char list\n");
                this->isRun = false;
                exit(1);
            }
            //compare data
            //buffer+4, after the 4 bytes from head, stored real char data
            //char data size is (total size - 4)
            unsigned char* expectData = (unsigned char*)calloc(1, dataSize);
            memset(expectData, actual_char, dataSize);
            if (0 == memcmp(expectData, (unsigned char*)buffer+4, dataSize)) {
                LOG_INFO("[SUCCESS] sent and received data same\n");
                free(expectData);
                expectData = NULL;
            } else {
                LOG_ERROR("[FAIL] sent and received data NOT same\n");
                LOG_INFO("expect %lu chars %c\n", bufferLen, actual_char);
                checkBufferChars(actual_char, (unsigned char*)buffer+4, dataSize);
                free(expectData);
                expectData = NULL;
                this->isRun = false;
                exit(1);
            }
        } else {
            char msgGot[this->sizeMsg];
            strncpy(msgGot, (char*)buffer+4, this->sizeMsg);
            if (0 == strncmp(msgGot, this->msgEnd, this->sizeMsg)) {
                //received end server msg, normally stop server
                LOG_INFO("server got end msg, ending server");
                this->isRun = false;
            } else {
                LOG_ERROR("[FAIL] server sent and received data size NOT same\n");
                exit(1);
            }
        }
    }

};

MyCWSServer * wsserver;

int main(int argc, char** argv) {

    log_level = INFO_LEVEL;
    LOG_INFO("server ARG[1]: %s", argv[1]);
    ushort port = std::stoi(argv[2]);
    LOG_INFO("server ARG[2]: %d", port);
    wsserver = new MyCWSServer(argv[1], port);
    wsserver->startup();
    while(wsserver->isRun) {
        sleep(10);
    }
    int result = wsserver->result;
    wsserver->shutdown();
    delete wsserver;
    LOG_INFO("server delete wsserver, result: %d", result);

    return result;
}

