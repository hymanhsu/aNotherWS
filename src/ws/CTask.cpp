//
// Created by xuhuahai on 2017/5/4.
//

#include <ws/CTask.h>
#include <simple_log.h>

#include <cstdlib>

namespace arc {

    /**
     * 任务反调函数
     * @param req
     */
    static void task_callback(uv_work_t *req) {
        if(req->data){
            CTask * pTask = (CTask *)req->data;
            if(pTask->m_pCallbackFunction){
                pTask->m_pCallbackFunction(pTask->m_pData);
            }
        }
    }

    /**
     * 任务完成反调函数
     * @param req
     * @param status
     */
    static void after_task_callback(uv_work_t *req, int status) {
        if(status){
            LOG_ERROR("task execute failed : %s", uv_strerror(status));
        }
        if(req->data){
            CTask * pTask = (CTask *)req->data;
            if(pTask->m_pAfterCallbackFunction){
                pTask->m_pAfterCallbackFunction(pTask, pTask->m_pData);
            }
        }
        free(req);
    }


    /**
     * 启动任务管理器
     * @param manager
     */
    static void task_mgr_run(void * data) {
        if(data){
            CTaskManager * manager = (CTaskManager *)data;
            LOG_DEBUG("task manager start");
            manager->m_pCLoop->run();
            LOG_DEBUG("task manager end");
        }
    }


    /**
     * @brief 初始化
     */
    CTaskManager::CTaskManager(){
        m_pCLoop = new arc::CLoop("TaskManager");
        m_pCThread = new arc::CThread(this,task_mgr_run);
    }


    /**
     * @brief 销毁
     */
    CTaskManager::~CTaskManager(){
        delete m_pCLoop;
        delete m_pCThread;
    }

    /**
     * 启动
     */
    void CTaskManager::startup() {
        m_pCThread->run();
    }


    /**
     * 停止
     */
    void CTaskManager::shutdown() {
        m_pCLoop->stop();
        m_pCThread->stop();
    }

    /**
    * 发送一个任务
    * @param pTask
    */
    void CTaskManager::postTask(CTask * pTask){
        uv_work_t * req = static_cast<uv_work_t *>(malloc(sizeof(uv_work_t)));
        if(!req){
            LOG_ERROR("postTask failed : malloc memory failed for uv_work_t");
            return;
        }
        req->data = pTask;
        uv_queue_work(m_pCLoop->m_pLoop, req, task_callback, after_task_callback);
    }


}




