#pragma once

#include <cstdint>
#include <functional>
#include <vector>

#include <WorkQueue.h>

#ifndef POLL_TIMEOUT
#define POLL_TIMEOUT 1000
#endif

namespace web
{
    typedef std::function<bool(int socketId)> RequestHandler;
    typedef WorkQueue<int> * SocketWorkQueuePtr;

    class TcpServer
    {
    protected:
        int m_port;
        std::vector<RequestHandler> *m_middleware;
        bool m_running;
        std::function<void(SocketWorkQueuePtr, TcpServer *)> m_workThreadFunc;

    public:
        TcpServer();
        virtual ~TcpServer(); //  = default;

        void configure(int port);
        std::vector<RequestHandler> *middleware();
        void onRequest(RequestHandler handler);
        int run();
        void shutdown();
    }; // class TcpServer
} // namespace web

