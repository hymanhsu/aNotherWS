/**
* @file       CLoop.h
* @brief      Loop的封装
* @details
* @author     Kevin.XU
* @date       2017.5.4
* @version    1.0.0
* @par Copyright (c):
*      ...
* @par History:
*   1.0.0: Kevin.XU, 2017.5.4, Create\n
*/

#ifndef UVTEST_CLOOP_H
#define UVTEST_CLOOP_H


#include <uv.h>

#include <functional>
#include <memory>
#include <string>


namespace arc {

    class CLoop {
    public:
        CLoop(const std::string & name);
        ~CLoop();

        /**
         * 运行
         */
        void run();

        /**
         * 关闭
         */
        void stop();

    public:
        std::string m_name;
        uv_loop_t* m_pLoop;
        uv_signal_t m_signal;
        uv_async_t m_sendAsync;
        volatile bool m_bRunnig;
    };

}


#endif //UVTEST_CLOOP_H
