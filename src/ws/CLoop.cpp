//
// Created by xuhuahai on 2017/5/4.
//

#include <ws/CLoop.h>
#include <simple_log.h>

#include <cstdlib>

namespace arc {


    /**
     * 接收来自外面的请求
     * 目前只处理关闭
     * @param handle
     */
    static void async_send_callback(uv_async_t *handle){
        if(handle->data){
            CLoop * pCLoop= (CLoop *)handle->data;
//            uv_loop_delete(pCLoop->m_pLoop);
            uv_stop(pCLoop->m_pLoop);
//            sleep(1);
            uv_loop_close(pCLoop->m_pLoop);
            LOG_DEBUG("%s : loop be stopped", pCLoop->m_name.c_str());
        }
    }


    /**
    * 信号处理
    * @param req
    * @param signum
    */
    static void task_signal_handler(uv_signal_t *req, int signum){
        LOG_INFO("loop received signal : %d", signum);
    }


    /**
     * 构造器
     */
    CLoop::CLoop(const std::string & name): m_name(name){
        m_pLoop = uv_loop_new();
        uv_async_init(this->m_pLoop, &this->m_sendAsync, async_send_callback);
        uv_signal_init(this->m_pLoop, &this->m_signal);
    }


    /**
     * 析构器
     */
    CLoop::~CLoop() {
        if(this->m_bRunnig){
            stop();
        }
    }


    /**
    * 运行
    */
    void CLoop::run(){
        if(this->m_bRunnig){
            LOG_DEBUG("%s : loop already running", this->m_name.c_str());
            return;
        }
        LOG_DEBUG("%s : loop run", this->m_name.c_str());
        this->m_bRunnig = true;
//        uv_signal_start(&this->m_signal, task_signal_handler, SIGINT); //Ctrl + C
        uv_run(this->m_pLoop, UV_RUN_DEFAULT);
        LOG_DEBUG("%s : loop stop", this->m_name.c_str());
    }


    /**
     * 关闭
     */
    void CLoop::stop(){
        if(this->m_bRunnig){
            this->m_bRunnig = false;
            LOG_DEBUG("%s : begin to stop loop", this->m_name.c_str());
            this->m_sendAsync.data = this;
            uv_async_send(&this->m_sendAsync);
            sleep(1);
        }
    }


}

