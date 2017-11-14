/**
* @file       CCchecker.h
* @brief      连接对象检查器
* @details
* @author     Kevin.XU
* @date       2017.5.25
* @version    1.0.0
* @par Copyright (c):
*      ...
* @par History:
*   1.0.0: Kevin.XU, 2017.5.25, Create\n
*/

#ifndef UVWS_CCHECKER_H
#define UVWS_CCHECKER_H

#include <ws/CSingleton.h>
#include <ws/uvutil.h>
#include <ws/CLock.h>

#include <map>
#include <set>
#include <string>


namespace arc{

    class CChecker
    {
    DEFINE_SINGLETON(CChecker)
    public:
        /**
         * 注册连接对象指针
         * @param client
         */
        void registerConnection(uv_stream_t* client);
        /**
         * 解注册连接对象指针
         * @param client
         */
        void unregisterConnection(uv_stream_t* client);
        /**
         * 检查连接对象指针有效性
         * @param client
         * @return
         */
        bool checkConnection(uv_stream_t* client);

    private:
        std::map<uv_stream_t*,std::string> m_connections;
        arc::CMutexLock                    m_lock;
    };

}

#endif //UVWS_CCHECKER_H
