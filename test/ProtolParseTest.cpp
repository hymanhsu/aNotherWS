//
// Created by xuhuahai on 2017/5/8.
//

#include "ws/common.h"
#include "ws/membuf.h"
#include "ws/wsutil.h"
#include "simple_log.h"
#include "test_header.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <sstream>


const char * simple_data_file = "/Users/xuhuahai/gitlab.alibaba-inc.com.projects/uvws/test/data/simple_protocol.dat";
const char * complex_data_file = "/Users/xuhuahai/gitlab.alibaba-inc.com.projects/uvws/test/data/complex_protocol.dat";
const char * middle_big_data_file = "/Users/xuhuahai/gitlab.alibaba-inc.com.projects/uvws/test/data/mid_big_protocol.dat";
const char * big_data_file = "/Users/xuhuahai/gitlab.alibaba-inc.com.projects/uvws/test/data/big_protocol.dat";


static char * read_data(const char * file_name, size_t * len){
    FILE *fp;
    fp = fopen(file_name, "rb");
    if(NULL==fp) {
        printf("error : %s\n", strerror(errno));
        return NULL;
    }
    fseek (fp, 0, SEEK_END);
    size_t size = ftell(fp);
    *len = size;
    fseek(fp, 0, SEEK_SET);
    char * buffer = (char *)malloc(size);
    fread(buffer,1,size,fp);
    fclose(fp);
    return buffer;
}


static int write_data(const char * file_name, void * buffer, size_t len){
    FILE *fp;
    fp = fopen(file_name, "wb");
    if(NULL==fp) {
        printf("error : %s\n", strerror(errno));
        return -1;
    }
    fwrite(buffer,1,len,fp);
    fclose(fp);
    return 0;
}


void generate_complex_test_data(){
    membuf_t buf;
    membuf_init(&buf, 128);

    char buffer[200];
    memset(buffer,0X60,200);
    generate_websocket_frame_without_fragment(&buf,buffer,200,WS_BIN_FRAME,FIN_ENABLE,1);  //with mask

    write_data(complex_data_file,buf.data,buf.size);

    membuf_uninit(&buf);
}

void generate_middle_big_test_data(){
    membuf_t buf;
    membuf_init(&buf, 128);

    char buffer[0x1234];
    memset(buffer,0X50,0x1234);
    generate_websocket_frame_without_fragment(&buf,buffer,0x1234,WS_BIN_FRAME,FIN_DISABLE,1);  //with mask

    write_data(middle_big_data_file,buf.data,buf.size);

    membuf_uninit(&buf);
}

void generate_big_test_data(){
    membuf_t buf;
    membuf_init(&buf, 128);

    char buffer[0x123456];
    memset(buffer,0X40,0x123456);
    generate_websocket_frame_without_fragment(&buf,buffer,0x123456,WS_BIN_FRAME,FIN_ENABLE,1);  //with mask

    write_data(big_data_file,buf.data,buf.size);

    membuf_uninit(&buf);
}


static void print_protocol(websocket_handle * hd){
    printf("---------------------------------------------\n");
    std::stringstream ss;
    char buffer[1024];
    memset(buffer,0,1024);
    snprintf(buffer,1024,"isEof : %x, dfExt : %x, type : %x, hasMask : %x, payloadLen : %x, mask : %x,%x,%x,%x, realPayloadLen : %lu, readCount : %lu, parseState : %u",
             hd->isEof,hd->dfExt,hd->type, hd->hasMask,hd->payloadLen,
             hd->mask[0], hd->mask[1], hd->mask[2], hd->mask[3],
             hd->realPayloadLen, hd->readCount, hd->parseState
    );
    printf("%s\n",buffer);
    printx(hd->protoBuf.data, hd->protoBuf.size<1000 ? hd->protoBuf.size : 1000);
}


static int random_int(int max){
    int ret = (int)(rand() % max);
    if(ret ==0){
        return max;
    }else{
        return ret;
    }
}


/**
 * 解析一个简单的协议报文
 */
void test_parse_simple(){
    //读数据
    size_t len;
    char * data = read_data(simple_data_file,&len);
    if(!data){
        return;
    }
    //创建一个模拟句柄对象
    websocket_handle * hd = (websocket_handle*)calloc(1, sizeof(websocket_handle));
    hd->endpointType = ENDPOINT_SERVER;
    hd->parseState = PARSE_COMPLETE;
    ///
    size_t pos = 0;
    while(pos<len){
        int n = 1;
        try_parse_protocol(hd,data,n);
        data += n;
        pos += n;
    }
    ///
    if(hd->parseState == PARSE_COMPLETE){
        LOG_DEBUG("parse success\n");
        sleep(2);
        print_protocol(hd);
    }

}


/**
 * 解析一个带mask的协议报文
 */
void test_parse_complex(){
    //读数据
    size_t len;
    char * data = read_data(complex_data_file,&len);
    if(!data){
        return;
    }
    //创建一个模拟句柄对象
    websocket_handle * hd = (websocket_handle*)calloc(1, sizeof(websocket_handle));
    hd->endpointType = ENDPOINT_SERVER;
    hd->parseState = PARSE_COMPLETE;
    ///
    size_t pos = 0;
    while(pos<len){
        int n = random_int(len-pos);
        printf("read %u\n",n);
        try_parse_protocol(hd,data,n);
        data += n;
        pos += n;
    }
    ///
    if(hd->parseState == PARSE_COMPLETE){
        LOG_DEBUG("parse success\n");
        sleep(2);
        print_protocol(hd);
    }

}

void test_parse_mid_big(){
    //读数据
    size_t len;
    char * data = read_data(middle_big_data_file,&len);
    if(!data){
        return;
    }
    //创建一个模拟句柄对象
    websocket_handle * hd = (websocket_handle*)calloc(1, sizeof(websocket_handle));
    hd->endpointType = ENDPOINT_SERVER;
    hd->parseState = PARSE_COMPLETE;
    ///
    size_t pos = 0;
    while(pos<len){
        int n = random_int(len-pos);
        printf("read %u\n",n);
        try_parse_protocol(hd,data,n);
        data += n;
        pos += n;
    }
    ///
    if(hd->parseState == PARSE_COMPLETE){
        LOG_DEBUG("parse success\n");
        sleep(2);
        print_protocol(hd);
    }

}


void test_parse_big(){
    //读数据
    size_t len;
    char * data = read_data(big_data_file,&len);
    if(!data){
        return;
    }
    //创建一个模拟句柄对象
    websocket_handle * hd = (websocket_handle*)calloc(1, sizeof(websocket_handle));
    hd->endpointType = ENDPOINT_SERVER;
    hd->parseState = PARSE_COMPLETE;
    ///
    size_t pos = 0;
    while(pos<len){
        int n = random_int(len-pos);
        printf("read %u\n",n);
        try_parse_protocol(hd,data,n);
        data += n;
        pos += n;
    }
    ///
    if(hd->parseState == PARSE_COMPLETE){
        LOG_DEBUG("parse success\n");
        sleep(2);
        print_protocol(hd);
    }

}


void judge_little_endian(){
    unsigned int value = 1;
    /**
     * little endian : 0x01 0x00 0x00 0x00
     * big endian    : 0x00 0x00 0x00 0x01
     */
    unsigned char * str = (unsigned char *)&value;
    if(1 == *str){
        LOG_DEBUG("little endian\n");
    }else{
        LOG_DEBUG("big endian\n");
    }
}


int main(int argc, char** agrv){

    judge_little_endian();

    //generate_complex_test_data();
//    generate_middle_big_test_data();
//    generate_big_test_data();

//    test_parse_simple();
//    sleep(2);
//
//    test_parse_complex();
//    sleep(2);

//    test_parse_mid_big();
//    test_parse_big();


}

