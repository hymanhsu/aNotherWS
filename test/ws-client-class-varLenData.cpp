//
// Created by haisheng.yhs on 2017/5/5.
//


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

    void onData(websocket_handle* handle, uv_stream_t* peer, void * buffer, size_t bufferLen, uchar type){
        LOG_DEBUG("-------------------------------------------response len=%lu-------------------------------------------\n", bufferLen);
        if( ISSET_FLAG(type,FLAG_WEBSOCKET_TEXT) ){
            printf("this is one text message\n");
        }else if(ISSET_FLAG(type,FLAG_WEBSOCKET_BIN)){
            printf("this is one binary message\n");
        }

        verifyData(buffer, bufferLen);
    }

    void onError(websocket_handle* handle, uv_stream_t* peer, int errcode, const char * errstr){
        LOG_ERROR("error ocuurs : %d, %s\n", errcode, errstr);
    }

    void onClose(websocket_handle* handle, uv_stream_t* peer){
        LOG_INFO("close connection\n");
    }

    void onConnect(uv_stream_t* peer){
        LOG_INFO("open connection\n");
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
                    LOG_INFO("[ERROR] sent and received data NOT same\n");
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

    void sendData() {
        unsigned long MAX_LEN = 536870912;  //32bits--max: 4294967295; 536870912==512*1024*1024, 512M
        char fill_char = '!'; //start from 33, unitl 126. total 94 chars will be traversed
        unsigned long times = MAX_LEN/LEN_TEST;
        unsigned long size = LEN_TEST;
        unsigned char *buffer = (unsigned char*)calloc(1, MAX_LEN);
        bool isRun = true;

        LOG_INFO("wait 10 seconds to ensure event loop started well\n");
        sleep(10);
        while(isRun) {

            if (size >= MAX_LEN) {
                size = MAX_LEN;
                isRun = false;
            }
            LOG_INFO("send %c, size: %lu", fill_char, size);
            memset(buffer, fill_char, size);
            buffer[3] = (size >> 8*3) & 0xFF;
            buffer[2] = (size >> 8*2) & 0xFF;
            buffer[1] = (size >> 8*1) & 0xFF;
            buffer[0] = size & 0xFF;
            LOG_INFO("buffer[3]： 0x%2x\n", buffer[3]);
            LOG_INFO("buffer[2]： 0x%2x\n", buffer[2]);
            LOG_INFO("buffer[1]： 0x%2x\n", buffer[1]);
            LOG_INFO("buffer[0]： 0x%2x\n", buffer[0]);
            memset(buffer+4, fill_char, size-4);
            //tw_send_bin_msg(conf.stream, buffer, size);
            //this->sendMsgWithAsync(this->m_peer, buffer, size, WS_BIN_FRAME);
            this->sendMsgWithAsync(this->m_peer, buffer, size, WS_BIN_FRAME,
                                   [](size_t inputLen, split_result * result){
                                       split_ont_size_to_multi_parts(inputLen,3,result);
                                   });
            if (fill_char == '~') {
                fill_char = '!';
            } else {
                fill_char++;
            }

            size *= 2;
            sleep(2);


//                char ch = getchar();
//                if(ch == 'Q' || ch == 'q'){
//                    break;
//                }


        }

        LOG_INFO("free buffer. testing end....");
        free(buffer);
        buffer = NULL;
        sleep(60);

    }


private:
    int LEN_TEST = 123;

};


MyCWSClient * wsclient;




int main(int argc, char** argv) {

    wsclient = new MyCWSClient("127.0.0.1",8080);
    wsclient->startup();

    wsclient->sendData();

    wsclient->shutdown();
    delete wsclient;


    return 0;
}


