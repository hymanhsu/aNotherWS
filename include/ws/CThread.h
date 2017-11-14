/**
* @file       CThread.h
* @brief      Websocket服务端的实现
* @details
* @author     Kevin.XU
* @date       2017.5.4
* @version    1.0.0
* @par Copyright (c):
*      ...
* @par History:
*   1.0.0: Kevin.XU, 2017.5.4, Create\n
*/

#ifndef UVTEST_CTHREAD_H
#define UVTEST_CTHREAD_H


#include <uv.h>

#include <functional>
#include <memory>
#include <string>


namespace arc {

    typedef void (*ThreadFunction)(void * data);

    class CThread{
    public:
        CThread(void * data, ThreadFunction threadFunction);
        ~CThread();

        void run();

        void join();

        void stop();

    private:
        void * m_data;
        uv_thread_t m_thid;
        ThreadFunction m_ThreadFunction;
    };

}


#endif //UVTEST_CTHREAD_H
