/**
* @file       CLock.h
* @brief      互斥锁的实现
* @details
* @author     Kevin.XU
* @date       2017.5.3
* @version    1.0.0
* @par Copyright (c):
*      ...
* @par History:
*   1.0.0: Kevin.XU, 2017.5.3, Create\n
*/

#ifndef UVTEST_CMUTEXLOCK_H
#define UVTEST_CMUTEXLOCK_H

#include <uv.h>

namespace arc {

    class CMutexLock {
    public:
        CMutexLock() {
            uv_mutex_init(&m_mutex);
        }

        ~CMutexLock() {
            uv_mutex_destroy(&m_mutex);
        }

        void Lock() {
            uv_mutex_lock(&m_mutex);
        }

        void Trylock() {
            uv_mutex_trylock(&m_mutex);
        }

        void Unlock() {
            uv_mutex_unlock(&m_mutex);
        }

    private:
        uv_mutex_t m_mutex;
    };

    class CScopeMutexLock{
    public:
        CScopeMutexLock(CMutexLock * lock):m_plock(lock){
            m_plock->Lock();
        }
        ~CScopeMutexLock(){
            m_plock->Unlock();
        }
    public:
        CMutexLock * m_plock;
    };

}

#endif //UVTEST_CMUTEXLOCK_H
