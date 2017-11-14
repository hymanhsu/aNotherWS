//
// Created by xuhuahai on 2017/4/25.
//

#define  LEN_M_10  10485760


#include "ws/membuf.h"
#include "ws/uvutil.h"
#include "ws/CWSClient.h"
#include "test_header.h"
#include "simple_log.h"

#include <string>

#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <stdio.h>


#define  PART_LEN  500
#define  LEN_TEST  2000

//const char * saved_data_directory = "/Users/xuhuahai/data/client/";


class MyCWSClient : public arc::CWSClient{

public:
    MyCWSClient(const std::string& ip, ushort port):
            arc::CWSClient(ip,port){
        beginTime = time(NULL);
    }
    ~MyCWSClient(){}

    void onData(websocket_handle* handle, uv_stream_t* peer, void * buffer, size_t bufferLen, uchar type){
        LOG_DEBUG("-------------------------------------------response len=%lu-------------------------------------------\n", bufferLen);
        if( ISSET_FLAG(type,FLAG_WEBSOCKET_TEXT) ){
            printf("this is one text message\n");
        }else if(ISSET_FLAG(type,FLAG_WEBSOCKET_BIN)){
            printf("this is one binary message\n");
        }
        printx(buffer, bufferLen < 256 ? bufferLen : 256);
        //save_proto_data(saved_data_directory,buffer,bufferLen);
        if(bufferLen!=LEN_TEST){
            time_t endTime = time(NULL);
            time_t costSeconds = endTime - beginTime;
            LOG_DEBUG("Maybe occurs some error !!!!! cost time = %u s\n", costSeconds);
            exit(-1);
        }
    }

    void onError(websocket_handle* handle, uv_stream_t* peer, int errcode, const char * errstr){
        LOG_ERROR("error ocuurs : %d, %s\n", errcode, errstr);
        exit(-1);
    }

    void onClose(websocket_handle* handle, uv_stream_t* peer){
        LOG_ERROR("close connection\n");
        exit(-1);
    }

    void onConnect(uv_stream_t* peer){
        LOG_ERROR("open connection\n");
    }

private:
    time_t  beginTime;

};


MyCWSClient * wsclient;

//typedef struct border_mark{
//    uchar a;
//    uchar b;
//    ushort c;
//} border_mark;
//
//void generateTestMark(uchar * buf, size_t size){
//    srand((unsigned)time(NULL));
//    if(size<8) {
//        return;
//    }
//    border_mark * header_mark = (border_mark*)buf;
//    header_mark->a = (uchar)(rand() % 255);
//    header_mark->b = (uchar)(rand() % 255);
//    header_mark->c =  header_mark->a + header_mark->b;
//
//    border_mark * tail_mark = (border_mark*)(buf+size-4);
//    tail_mark->a = (uchar)(rand() % 255);
//    tail_mark->b = (uchar)(rand() % 255);
//    tail_mark->c =  tail_mark->a + tail_mark->b;
//
//}
//
//int checkTestMark(uchar * buf, size_t size){
//
//}

static void one_test(){
    static uchar fill_char = 'a';
    printf("DO ONE TEST !!!!!!!!\n");
    char buffer[LEN_TEST];
    char * p = buffer;
    memset(p,fill_char,PART_LEN);
    p += PART_LEN;
    fill_char++;
    memset(p,fill_char,PART_LEN);
    p += PART_LEN;
    fill_char++;
    memset(p,fill_char,PART_LEN);
    p += PART_LEN;
    fill_char++;
    memset(p,fill_char,PART_LEN);
    fill_char++;
    bool ret = wsclient->sendMsgWithAsync(wsclient->m_peer, buffer, LEN_TEST, WS_BIN_FRAME
//                                          ,[](size_t inputLen, split_result * result){
//                                              split_ont_size_to_multi_parts(inputLen,9,result);
//                                          }
    );
    if(!ret){
        LOG_ERROR("send failed!!!!!!!!!\n");
        exit(-1);
    }
}


static void uv_timer_callback (uv_timer_t* handle){
    one_test();
    //sleep(1000);
}


int main(int argc, char** argv) {

    wsclient = new MyCWSClient("127.0.0.1",8080);

    uv_timer_t gc_req;
    uv_timer_init(wsclient->m_pCLoop->m_pLoop, &gc_req);
    uv_timer_start(&gc_req, uv_timer_callback, 30000, 6000);

    wsclient->startup();


    while(1){
        unsigned char ch = getchar();
        if(ch == 'Q' || ch == 'q'){
            break;
        }
        if(ch == '\n' || ch == 255){
            continue;
        }
        if(ch == 'T'){
            one_test();
        }
    }

    wsclient->shutdown();
    delete wsclient;


    return 0;
}

