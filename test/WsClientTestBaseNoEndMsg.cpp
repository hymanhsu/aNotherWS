/**
 * @author haisheng.yhs
 * @description
 *      create a client and connect to server, after data sent exit client only.
 *      server will keep running.
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
            arc::CWSClient(ip,port,500000){}
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
        exit(1);
    }

    void onClose(websocket_handle* handle, uv_stream_t* peer){
        LOG_INFO("close connection\n");
    }

    void onConnect(uv_stream_t* peer){
        LOG_INFO("open connection\n");
    }

    /**
     * @param  unsigned long maxSize, the max data size
     * @param  unsigned long repeatTimes, data sending max loop, one loop include sending data from LEN_TEST(123) to maxSize
     * @param  char startChar, the data of sending, from startChar to ~. while reached ~, restart data at startChar
     */
    void sendData(unsigned long maxSize, unsigned long repeatTimes, char startChar) {
        unsigned long MAX_LEN = maxSize;  //32bits--max: 4294967295; 536870912==512*1024*1024, 512M
        unsigned long times = 0; //data sending loop, one loop include sending data from LEN_TEST(123) to maxSize
        unsigned long size = LEN_TEST;
        char fill_char = startChar; //start from startChar, unitl ~(126)
        unsigned char *buffer = (unsigned char*)calloc(1, MAX_LEN);
        bool isSendData = true;

        this->maxSize = maxSize;
        this->repeatTimes = repeatTimes;
        LOG_INFO("wait 10 seconds to ensure event loop started well\n");
        sleep(10);

        while(isSendData) {

            //exit directly, to conveniently trace issue
            if(this->m_peer->data == NULL || ! this->isRun){
                exit(1);
            }

            //if data size exceed MAX_LEN, reset size to MAX_LEN
            //and loop + 1
            if (size >= MAX_LEN) {
                size = MAX_LEN;
                times++;
            }
            LOG_INFO("send %c, size: %lu", fill_char, size);
            memset(buffer, 0, MAX_LEN);

            //buffer, the first 4 bytes from head, used to store data total size( header 4bytes + char size bytes)
            buffer[3] = (size >> 8*3) & 0xFF;
            buffer[2] = (size >> 8*2) & 0xFF;
            buffer[1] = (size >> 8*1) & 0xFF;
            buffer[0] = size & 0xFF;
            LOG_DEBUG("buffer[3]： 0x%2x\n", buffer[3]);
            LOG_DEBUG("buffer[2]： 0x%2x\n", buffer[2]);
            LOG_DEBUG("buffer[1]： 0x%2x\n", buffer[1]);
            LOG_DEBUG("buffer[0]： 0x%2x\n", buffer[0]);
            //buffer+4, after the 4 bytes from head, used to store real char data
            //char data size is (total size - 4)
            memset(buffer+4, fill_char, size-4);
            checkBufferChars(fill_char, (unsigned char*)buffer+4, size-4);
            this->sendMsgWithAsync(this->m_peer, buffer, size, WS_BIN_FRAME,
                                   [](size_t inputLen, split_result * result){
                                       split_ont_size_to_multi_parts(inputLen,3,result);
                                   });
            if (fill_char == '~') {
                fill_char = '!';
            } else {
                fill_char++;
            }

            //total data size step forward by double current total size
            size *= 2;
            if (times >= repeatTimes) {
                isSendData = false;
            } else if (size == MAX_LEN*2) {
                //while one loop done, reset data size to initial size(123)
                size = LEN_TEST;
                //simulate normal human speech, and let async pool release resource
                if (times%100 == 0)
                    sleep(2);
            }
        }
        LOG_INFO("free buffer. testing end....");
        free(buffer);
        buffer = NULL;
        this->isSent = true;
    }


private:
    int LEN_TEST = 123;
    char* msgEnd = "++end++";
    bool isSent = false; //true --- data send done, false -- data send not finished
    unsigned long maxSize;
    unsigned long repeatTimes;
    unsigned long receivedLoop = 0;
    int sizeMsg = sizeof(msgEnd)/sizeof(msgEnd[0]);

    //check buffer store chars if correct
    void checkBufferChars(const char expect, const void* buffer, unsigned long bufferSize) {
        for (unsigned long i = 0; i <  bufferSize; i++) {
            if (((char*)buffer)[i] != expect) {
                LOG_ERROR("[FAIL] client chars in your memory NOT all same, differ: ((char*)buffer)[%lu] == %c", i, ((char*)buffer)[i]);
            }
        }
    }

    //verify data which received from server
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
                LOG_ERROR("[FAIL] do NOT find expect char in char list\n");
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

            //check if all data has been received from server
            if (this->maxSize == bufferLen) {
                this->receivedLoop++;
                LOG_INFO("The %lu loops done", this->receivedLoop)
                if (this->isSent && this->receivedLoop == this->repeatTimes) {
                    LOG_INFO("all data has been received from server");;
                    this->isRun = false;
                }
            }

        } else {
            char msgGot[this->sizeMsg];
            strncpy(msgGot, (char*)buffer+4, this->sizeMsg);
            if (0 == strncmp(msgGot, this->msgEnd, this->sizeMsg)) {
                //received end client msg, normally stop client
                //this used to double ensure client stop. normally, client will auto stop after all data sended.
                LOG_INFO("client got end msg, ending client");
                this->isRun = false;
            } else {
                LOG_ERROR("[FAIL] sent and received data size NOT same\n");
                exit(1);
            }
        }
    }

};


int main(int argc, char** argv) {

    log_level = INFO_LEVEL;
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
    sleep(30); //wait server sending remaining data done
    int result = wsclient->result;
    wsclient->shutdown();
    delete wsclient;
    LOG_INFO("client-end-msg delete wsclient, result: %d", result);

    return result;
}


