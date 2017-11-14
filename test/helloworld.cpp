//
// Created by xuhuahai on 2017/4/20.
//

#include <stdio.h>
#include <stdlib.h>
#include "uv.h"

int64_t counter = 0;

void wait_for_a_while(uv_idle_t* handle) {
    counter++;
    printf("idling %lld\n",counter);
    if (counter >= 10e6)
        uv_idle_stop(handle);
}

int main() {
    uv_idle_t idler;

    uv_loop_t *loop = static_cast<uv_loop_t *>(malloc(sizeof(uv_loop_t)));
    uv_loop_init(loop);

    //init & start idle
    uv_idle_init(loop, &idler);
    uv_idle_start(&idler, wait_for_a_while);

    printf("Idling ...\n");
    uv_run(loop, UV_RUN_DEFAULT);

    uv_loop_close(loop);
    free(loop);
    return 0;
}
