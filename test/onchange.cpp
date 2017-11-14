//
// Created by xuhuahai on 2017/4/21.
//

#include <stdio.h>
#include <stdlib.h>
#include "uv.h"


char * command = NULL;
uv_loop_t *loop = NULL;

void run_command(uv_fs_event_t *handle, const char *filename, int events, int status) {
    char path[1024];
    size_t size = 1023;
    // Does not handle error if path is longer than 1023.
    uv_fs_event_getpath(handle, path, &size);
    path[size] = '\0';
    fprintf(stderr, "Change detected in %s: ", path);
    if (events & UV_RENAME)
        fprintf(stderr, "renamed");
    if (events & UV_CHANGE)
        fprintf(stderr, "changed");
    fprintf(stderr, " %s\n", filename ? filename : "");
    system(command);
}

int main(int argc, char** argv){
    loop = uv_default_loop();
    command = argv[1];
    while (argc-- > 2) {
        fprintf(stderr, "Adding watch on %s\n", argv[argc]);
        uv_fs_event_t *fs_event_req = static_cast<uv_fs_event_t *>(malloc(sizeof(uv_fs_event_t)));
        uv_fs_event_init(loop, fs_event_req);
        // The recursive flag watches subdirectories too.
        uv_fs_event_start(fs_event_req, run_command, argv[argc], UV_FS_EVENT_RECURSIVE);
    }
    return uv_run(loop, UV_RUN_DEFAULT);
}
