//
// Created by xuhuahai on 2017/4/24.
//

#include "uv.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <pthread.h>


#define FIB_UNTIL 20

static uv_loop_t* loop;
uv_async_t async;


void print_progress(uv_async_t *handle)
{
    int index = *((int*) handle->data);
    fprintf(stdout, "[%u] print_progress %d\n", uv_thread_self(), index);
    if(FIB_UNTIL == index ){
        uv_close((uv_handle_t*)&async, NULL);
    }
}


long fib_(long n){
    if(n==1 || n==2){
        return 1L;
    }else {
        return fib_(n-1) + fib_(n-2);
    }
}

void fib(uv_work_t *req) {
    int n = *(int *) req->data;
    if (random() % 2)
        sleep(1);
    else
        sleep(3);
    long fib = fib_(n);
    fprintf(stdout, "[%u] %dth fibonacci is %lu\n", pthread_self(), n, fib);
    //
//    async.data = (void*) req->data;
//    uv_async_send(&async);
}

void after_fib(uv_work_t *req, int status) {
    fprintf(stdout, "[%u] Done calculating %dth fibonacci\n", pthread_self(), *(int *) req->data);
}


int main() {
    loop = uv_default_loop();
    int data[FIB_UNTIL];
    uv_work_t req[FIB_UNTIL];
    int i;
    for (i = 0; i < FIB_UNTIL; i++) {
        data[i] = i+1;
        req[i].data = (void *) &data[i];
        uv_queue_work(loop, &req[i], fib, after_fib);
    }
//    uv_async_init(loop, &async, print_progress);
    return uv_run(loop, UV_RUN_DEFAULT);

}

