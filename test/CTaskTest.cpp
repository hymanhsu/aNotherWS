//
// Created by xuhuahai on 2017/4/24.
//

#include "ws/CTask.h"
#include "simple_log.h"



int main(int argc, char** agrv) {

    arc::CTaskManager * cTaskManager = new arc::CTaskManager;
    cTaskManager->startup();


    sleep(5);

    for(int i=0; i< 20; ++i){
        int * data  = new int;
        *data = i;
        arc::CTask  * task = new arc::CTask(
            data,
            [](void * data){
                int value = *(int *)data;
                LOG_DEBUG("rec %d\n", value);
                sleep(2);
            },
            [](arc::CTask * task, void * data){
                int value = *(int *)data;
                LOG_DEBUG("free %d\n", value);
                delete (int *)data;
                delete task;
            }
        );
        cTaskManager->postTask(task);
    }


    while(1){
        unsigned char ch = getchar();
        if(ch == 'Q' || ch == 'q'){
            break;
        }
        if(ch == '\n' || ch == 255 ){
            continue;
        }
        int * data  = new int;
        *data = ch & 0xFF;
        arc::CTask  * task = new arc::CTask(
                data,
                [](void * data){
                    int value = *(int *)data;
                    LOG_DEBUG("rec %d\n", value);
                    sleep(2);
                },
                [](arc::CTask * task, void * data){
                    int value = *(int *)data;
                    LOG_DEBUG("free %d\n", value);
                    delete (int *)data;
                    delete task;
                }
        );
        cTaskManager->postTask(task);
    }

    cTaskManager->shutdown();
    delete cTaskManager;

}

