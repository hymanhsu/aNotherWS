//
// Created by xuhuahai on 2017/4/28.
//

#include <ws/uvutil.h>
#include <ws/sha1.h>
#include <ws/wsutil.h>
#include <ws/CLock.h>
#include <ws/CChecker.h>

#include <simple_log.h>


/**
 * 从http报文中解析出http头信息
 * @param buf
 * @param key_name
 * @return
 */
char* parse_http_heads(const uv_buf_t* buf, const char * key_name){
    char *key,*end;
    char* data = strstr(buf->base, "\r\n\r\n");
    if (data) {
        *data = 0;
        //是否有 Sec-WebSocket-Key
        key = strstr(buf->base, key_name);
        if (key) {
            key += strlen(key_name)+1;
            while (isspace(*key)) key++;
            end = strchr(key, '\r');
            if (end) *end = 0;
            return key;
        }
    }
    return NULL;
}


/**
 * 关闭客户端连接后，释放客户端连接的数据
 * @param client  连接对象的指针
 */
void after_uv_close_client(uv_handle_t* client) {
    LOG_DEBUG("Invoke after_uv_close_client()");
    if(client->data){
        LOG_DEBUG("Destroy WebSocketHandle object");
        arc::CChecker::instance().unregisterConnection( (uv_stream_t*)client );
        websocket_handle* hd = (websocket_handle*)client->data;
        endpoint_type endpointType = hd->endpointType;
        membuf_uninit(&hd->protoBuf);
        membuf_uninit(&hd->mergeBuf);
        free(hd);
        client->data = NULL;
        LOG_DEBUG("Set uv_stream_t.data to NULL");
        if(uv_is_closing((uv_handle_t*)client)){
            LOG_DEBUG("After close, handle is closing");
        }
        if(endpointType == ENDPOINT_SERVER){
            LOG_DEBUG("free uv_stream_t");
            free(client);
        }
    }
}


/**
 * 关闭连接
 * @param client    连接对象的指针
 * @param on_close  应用层在关闭前的反调函数
 */
void tw_close_client(uv_stream_t* client, char (*on_close)(uv_stream_t*)) {
    //关闭连接回调
    if (on_close)
        on_close(client);
    if(!uv_is_closing((uv_handle_t*)client)){
        LOG_DEBUG("Invoke uv_close()");
        uv_close((uv_handle_t*)client, after_uv_close_client);
    }else{
        LOG_DEBUG("uv_stream_t is closing, skip ");
    }
}


