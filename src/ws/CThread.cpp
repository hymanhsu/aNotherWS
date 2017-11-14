//
// Created by xuhuahai on 2017/5/4.
//

#include <ws/CThread.h>
#include <simple_log.h>


namespace arc {


    CThread::CThread(void * data, ThreadFunction threadFunction): m_data(data), m_ThreadFunction(threadFunction) {
    }


    CThread::~CThread() {
    }


    void CThread::run() {
        uv_thread_create(&m_thid, (uv_thread_cb)m_ThreadFunction, m_data);
    }


    void CThread::join() {
        uv_thread_join(&m_thid);
    }

    void CThread::stop() {
        //DO NOTHIND
    }

}
