/**
* @file       CTask.h
* @brief      Task的封装
* @details
* @author     Kevin.XU
* @date       2017.5.3
* @version    1.0.0
* @par Copyright (c):
*      ...
* @par History:
*   1.0.0: Kevin.XU, 2017.5.3, Create\n
*/

#ifndef UVTEST_CTASK_H
#define UVTEST_CTASK_H


#include <uv.h>

#include <ws/CLoop.h>
#include <ws/CThread.h>

#include <functional>
#include <memory>

namespace arc {

    class CTask;

    typedef void (*CallbackFunction)(void * data);
    typedef void (*AfterCallbackFunction)(CTask * task, void * data);

    /**
     * @brief 任务对象
     */
    class CTask {
    public:
        CTask(void * data, CallbackFunction pCallbackFunction, AfterCallbackFunction pAfterCallbackFunction):
                m_pData(data), m_pCallbackFunction(pCallbackFunction),
                m_pAfterCallbackFunction(pAfterCallbackFunction){
        }

        ~CTask(){}

    public:
        /**
         * 指向数据的指针
         */
        void * m_pData;

        /**
         * 任务执行回调函数
         */
        CallbackFunction m_pCallbackFunction;

        /**
         * 任务完成后回调函数
         */
        AfterCallbackFunction m_pAfterCallbackFunction;
    };


    /**
     * @brief  任务管理器对象
     * @note  一个进程内部一般只需要一个任务管理器
     */
    class CTaskManager{
    public:
        CTaskManager();
        ~CTaskManager();

        /**
         * 发送一个任务
         * @param pTask
         */
        void postTask(CTask * pTask);

        /**
         * 启动
         */
        void startup();

        /**
         * 停止
         */
        void shutdown();

    public:
        arc::CLoop   * m_pCLoop;
        arc::CThread * m_pCThread;
    };


}



#endif //UVTEST_CCALLBACK_H
