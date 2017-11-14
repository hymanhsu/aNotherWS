/**
 * @author haisheng.yhs
 * @description
 *      create a client and connect to server,
 *      send ending msg to server after data all be sent,
 *      server and client will be closed all.
 * @version 1.0, initial version 2017/05/05
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
        verifyData(buffer, bufferLen);
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

    void sendData(unsigned long maxSize, unsigned long repeatTimes, char startChar) {
        unsigned long MAX_LEN = maxSize;  //32bits--max: 4294967295; 536870912==512*1024*1024, 512M
        char fill_char = startChar; //start from 33, unitl 126. total 94 chars will be traversed
        unsigned long times = 0;
        unsigned long size = LEN_TEST;
        unsigned char *buffer = (unsigned char*)calloc(1, MAX_LEN);
        bool isSendData = true;

        LOG_INFO("wait 10 seconds to ensure event loop started well\n");
        sleep(10);

        while(isSendData) {

            if (size >= MAX_LEN) {
                size = MAX_LEN;
                times++;
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
            if (times >= repeatTimes) {
                isSendData = false;
            } else if (size == MAX_LEN*2) {
                size = LEN_TEST;
            }
        }
        //end end msg, actual size = buffer head(4) + msg real size(8)
        size = this->sizeMsg;
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
    int LEN_TEST = 123;
    char* msgEnd = "++end++";
    int sizeMsg = sizeof(msgEnd)/sizeof(msgEnd[0]);

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
                    this->isRun = false;
                    this->result = 1;
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
                    this->isRun = false;
                    this->result = 1;
                }
                free(expectData);
                expectData = NULL;
            } else {
                LOG_ERROR("[FAIL] sent and received data NOT same\n");
                LOG_INFO("expect %lu chars %c\n", bufferLen, actual_char);
                this->isRun = false;
                this->result = 1;
            }
        } else {
            char msgGot[this->sizeMsg];
            strncpy(msgGot, (char*)buffer+4, this->sizeMsg);
            if (0 == strncmp(msgGot, this->msgEnd, this->sizeMsg)) {
                LOG_INFO("client got end msg, ending client");
            } else {
                LOG_ERROR("[FAIL] sent and received data size NOT same\n");
                this->result = 1;
            }
            this->isRun = false;
        }
    }

};


int main(int argc, char** argv) {

    LOG_INFO("client ARG[1]: %s", argv[1]);
    ushort port = std::stoi(argv[2]);
    LOG_INFO("client ARG[2]: %d", port);
    unsigned long maxDataSize = std::stoul(argv[3]);
    LOG_INFO("client ARG[3]: %lu", maxDataSize);
    unsigned long repeatTimes = std::stoul(argv[4]);
    LOG_INFO("client ARG[4]: %lu", repeatTimes);
    char startChar = argv[5][0];
    LOG_INFO("client ARG[5]: %s, char: %c", argv[5], startChar);

    MyCWSClient* wsclient = new MyCWSClient(argv[1], port);
    wsclient->startup();
    wsclient->sendData(maxDataSize, repeatTimes, startChar);
    while(wsclient->isRun) {
        sleep(10);
    }
    int result = wsclient->result;
    wsclient->shutdown();
    delete wsclient;
    LOG_INFO("client delete wsclient, result: %d", result);

    return result;
}


