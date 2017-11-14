//
// Created by xuhuahai on 2017/4/23.
//


#include "uv.h"
#include "ws/membuf.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>


/* Have our own assert, so we are sure it does not get optimized away in
 * a release build.
 */
#define ASSERT(expr)                                      \
 do {                                                     \
  if (!(expr)) {                                          \
    fprintf(stderr,                                       \
            "Assertion failed in %s on line %d: %s\n",    \
            __FILE__,                                     \
            __LINE__,                                     \
            #expr);                                       \
    abort();                                              \
  }                                                       \
 } while (0)


/* Die with fatal error. */
#define FATAL(msg)                                        \
  do {                                                    \
    fprintf(stderr,                                       \
            "Fatal error in %s on line %d: %s\n",         \
            __FILE__,                                     \
            __LINE__,                                     \
            msg);                                         \
    fflush(stderr);                                       \
    abort();                                              \
  } while (0)


#define TEST_PORT 9123





//uv_handle_t > uv_stream_t > uv_tcp_t
static uv_loop_t* loop;
static uv_handle_t* client;
static uv_tcp_t tcpClient;

static uv_stream_t* stream;

static void on_close(uv_handle_t* handle);
static void alloc_cb(uv_handle_t* handle, size_t size, uv_buf_t* buf);
static void on_read(uv_stream_t* tcp, ssize_t nread, const uv_buf_t* buf);
static void on_write(uv_write_t* req, int status);
static void on_connect(uv_connect_t* connection, int status);
static int tcp4_echo_start(int port);

static void on_close(uv_handle_t* handle)
{
    printf("closed.");
}

static void alloc_cb(uv_handle_t* handle, size_t size, uv_buf_t* buf)
{
    buf->base = static_cast<char *>(malloc(size));
    buf->len = size;
}

static void on_read(uv_stream_t* tcp, ssize_t nread, const uv_buf_t* buf)
{
    if(nread >= 0) {
        printf("read: [");
        ssize_t num = 0;
        while(num<nread){
            printf("%c", *(buf->base+num));
            num++;
        }
        printf("]\n");
        //printf("read: [%s]\n", protoBuf->base);
    }
    else {
        uv_close((uv_handle_t*)tcp, on_close);
    }
    free(buf->base);
}

static void after_write(uv_write_t* req, int status)
{
    printf("wrote.\n");
    //uv_close((uv_handle_t*)req->handle, on_close);
    free(req->data);
}

static void send_message(uv_stream_t* stream, const char * buffer, size_t n)
{
    static int flag = 0;

    char * data = static_cast<char *>(malloc(n));
    memcpy(data,buffer,n);

    uv_buf_t buf = uv_buf_init(data,n);

    uv_write_t request;
    request.data = data;

    uv_write(&request, stream, &buf, 1, after_write);

    if(0==flag)
        uv_read_start(stream, alloc_cb, on_read);
    flag++;
}

static void on_connect(uv_connect_t* connection, int status) {
    printf("connected.\n");
    stream = connection->handle;

    send_message(stream, "helloworld", 10);
}

static int tcp4_echo_start(int port) {
    struct sockaddr_in addr;
    int r;

    ASSERT(0 == uv_ip4_addr("0.0.0.0", port, &addr));

    client = (uv_handle_t*)&tcpClient;

    r = uv_tcp_init(loop, &tcpClient);
    if (r) {
        /* TODO: Error codes */
        fprintf(stderr, "Socket creation error\n");
        return 1;
    }

    uv_connect_t* connect = (uv_connect_t*)malloc(sizeof(uv_connect_t));
    uv_tcp_connect(connect, &tcpClient,  (const struct sockaddr*) &addr, on_connect);
    return 0;
}

static void thread_entry(void* arg) {
    ASSERT(arg == (void *) 42);
    loop = uv_default_loop();

    if (tcp4_echo_start(TEST_PORT))
        return;

    uv_run(loop, UV_RUN_DEFAULT);
}

int main() {
    uv_thread_t tid;
    int r;

    r = uv_thread_create(&tid, thread_entry, (void *) 42);
    ASSERT(r == 0);

//    r = uv_thread_join(&tid);
//    ASSERT(r == 0);
//    printf("Joined !\n");

    while(1){
        //printf("Enter your sentence:");
        char buffer[512];
        ssize_t n = read_line(STDIN_FILENO,buffer,512);
        if(n>0){
            printf("your sentence:[%s]\n",buffer);
            send_message(stream,buffer,n);
        }
    }


    return 0;
}