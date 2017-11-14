//
// Created by xuhuahai on 2017/5/25.
//

#include <ws/CChecker.h>
#include <simple_log.h>

#include <utility>

namespace arc{

    CChecker::CChecker() {
        LOG_DEBUG("CChecker init");
    }


    CChecker::~CChecker() {
        LOG_DEBUG("CChecker destroy");
    }


    /**
     * 注册连接对象指针
     * @param client
     */
    void CChecker::registerConnection(uv_stream_t *client) {
        arc::CScopeMutexLock lock(&m_lock);
        m_connections.insert( std::make_pair(client,std::string("1")) );
        LOG_DEBUG("register client %x", client);
    }


    /**
     * 解注册连接对象指针
     * @param client
     */
    void CChecker::unregisterConnection(uv_stream_t *client) {
        arc::CScopeMutexLock lock(&m_lock);
        std::map<uv_stream_t*,std::string>::iterator iter;
        iter = m_connections.find(client);
        if( iter == m_connections.end() ){
            LOG_DEBUG("unregister client %x failed : not found", client);
        }else{
            LOG_DEBUG("unregister client %x", client);
            m_connections.erase(iter);
        }
    }


    /**
     * 检查连接对象指针有效性
     * @param client
     * @return
     */
    bool CChecker::checkConnection(uv_stream_t *client) {
        arc::CScopeMutexLock lock(&m_lock);
        std::map<uv_stream_t*,std::string>::iterator iter;
        iter = m_connections.find(client);
        if( iter == m_connections.end() ){
            LOG_DEBUG("checkConnection client %x failed : not found", client);
            return false;
        }else{
            LOG_DEBUG("checkConnection client %x success", client);
            return true;
        }
    }


}
