/**
 * @author haisheng.yhs
 * @description
 *      duplex send data from 123 to 1K
 *      last 30 minutes
 * @version 1.0, initial version, 2017/05/16
 *
 */

#include "simple_log.h"

#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdio.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <ctime>






int start_server( void *ptr ) {
    LOG_INFO("start server");

    int result = system("rm -rf server.smallSizeData.log; ./WsServerTestBase 0.0.0.0 8080 2>&1 | tee server.smallSizeData.log");
    LOG_INFO("server return %d", result);
    return result;
}

int start_client( void *ptr ) {
    LOG_INFO("start client");
    int result = system("rm -rf client.smallSizeData.log; ./WsClientTestBaseNoEndMsg 127.0.0.1 8080 1024 50000 ! 2>&1 | tee client.smallSizeData.log"); //1024 max
    LOG_INFO("client return %d", result);
    return result;
}

int start_client2( void *ptr ) {
    LOG_INFO("start client");
    int result = system("rm -rf client2.smallSizeData.log; ./WsClientTestBaseNoEndMsg 127.0.0.1 8080 1024 50000 A 2>&1 | tee client2.smallSizeData.log"); //1024 max
    LOG_INFO("client return %d", result);
    return result;
}

int start_client_end( void *ptr ) {
    LOG_INFO("start client to end server and current client");
    int result = system("rm -rf clientEnd.smallSizeData.log; ./WsClientTestSendEnd 127.0.0.1 8080 2>&1 | tee clientEnd.smallSizeData.log");
    LOG_INFO("client-end return %d", result);
    return result;
}

int checkResult() {
    int result = 0;
    if (system("grep ERROR client.smallSizeData.log client2.smallSizeData.log") == 0) {
        LOG_ERROR("[FAIL], find ERROR in output log, please search ERROR to double check log");
        result = 1;
    }

    return result;
}

int main(void) {

    int i = 1;
    pid_t pid, mainPid, serverPid, clientPid, client2Pid;

    mainPid = getpid();

    if((pid = fork()) < 0){
        LOG_INFO("forkerror\n");
    }else if(pid == 0){
        LOG_INFO("%d,childself's pid=%d,parent's pid=%d,returnid=%d\n",i,getpid(),getppid(),pid);
        //start server
        exit(start_server((char*)"start server"));
    }else{
        serverPid = pid;
        LOG_INFO("%d,parentself's pid=%d,parent's father's pid=%d,returnid=%d\n",i,getpid(),getppid(),pid);
        sleep(10);
        if((pid = fork()) < 0){
            LOG_INFO("forkerror\n");
        }else if(pid == 0){
            LOG_INFO("%d,childself's pid=%d,parent's pid=%d,returnid=%d\n",i,getpid(),getppid(),pid);
            //start client
            exit(start_client((char*)"start client"));
        }else{
            clientPid = pid;
            LOG_INFO("%d,parentself's pid=%d,parent's father's pid=%d,returnid=%d\n",i,getpid(),getppid(),pid);
            if((pid = fork()) < 0){
                LOG_INFO("forkerror\n");
            }else if(pid == 0) {
                LOG_INFO("%d,childself's pid=%d,parent's pid=%d,returnid=%d\n", i, getpid(), getppid(), pid);
                //start client2
                exit(start_client2((char *) "start client2"));
            } else {
                client2Pid = pid;
                int statusClient, statusServer;
                if (waitpid(clientPid, &statusClient, 0)) {
                    LOG_INFO("client proc PID=%d has been terminated, status: %d", clientPid, statusClient);
                }
                if (waitpid(client2Pid, &statusClient, 0)) {
                    LOG_INFO("client2 proc PID=%d has been terminated, status: %d", client2Pid, statusClient);
                }
                sleep(10);
                start_client_end((char *) "start end");
                if (waitpid(serverPid, &statusServer, 0)) {
                    LOG_INFO("server proc PID=%d has been terminated, status: %d", serverPid, statusServer);
                }

                exit(checkResult());
            }

        }
    }
}
