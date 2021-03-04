#include <arpa/inet.h>
#include <condition_variable>
#include <cstring>
#include <mutex>
#include <poll.h>
#include <queue>
#include <sys/socket.h>
#include <sys/time.h>
#include <thread>
#include <unistd.h>

// includes WorkQueue
#include <TcpServer.h>

namespace web
{
    static void threadHandleConnection(SocketWorkQueuePtr workQueue, TcpServer *srv)
    {
        if (workQueue == nullptr)
            return;

        for (;;)
        {
            int workSocket = workQueue->findWork();

            if (workSocket == -1) // quit signal
                break;

            // Run through all middleware before going to route requests
            for (auto f : *srv->middleware())
               if (f(workSocket))
                   break;

            close(workSocket);
        } // for(;;)
    }

    TcpServer::TcpServer() : m_running(false)
    {
        m_middleware = new std::vector<RequestHandler>();
        m_workThreadFunc = threadHandleConnection;
    }

    TcpServer::~TcpServer()
    {
        if (m_middleware != nullptr)
        {
            delete m_middleware;
            m_middleware = nullptr;
        }
    }

    void TcpServer::configure(int port)
    {
        m_port = port;
    }

    std::vector<RequestHandler> *TcpServer::middleware()
    {
        return m_middleware;
    }

    void TcpServer::onRequest(RequestHandler handler)
    {
        if (m_middleware)
            m_middleware->push_back(handler);
    }

    int TcpServer::run()
    {
        int sock = socket(AF_INET, SOCK_STREAM, 0);
        if (sock < 0)
            return errno;

        sockaddr_in saddr;
        saddr.sin_family = AF_INET;
        saddr.sin_addr.s_addr = INADDR_ANY;
        saddr.sin_port = htons(m_port);

        if (bind(sock, (sockaddr *)&saddr, sizeof(saddr)) < 0)
            return errno;

        if (listen(sock, 20) < 0)
            return errno;

        WorkQueue<int> workQueue;
        std::vector<std::thread> workers;

        size_t numThreads = std::thread::hardware_concurrency();
        if (numThreads < 2)
            numThreads = 1; // 1 or 0 response should have 1 worker thread
        else
            --numThreads;

        for (size_t i = 0; i < numThreads; ++i)
            workers.push_back(std::thread(m_workThreadFunc, &workQueue, this));

        struct pollfd mainSock { sock, POLLIN };
        struct pollfd pollList[1] = { mainSock };

        int status = 0;
        sockaddr_in client;
        socklen_t clientLen = sizeof(client);
        m_running = true;
        for (;;) {
            int pollCount = poll(pollList, 1, POLL_TIMEOUT);

            if (pollCount > 0 && pollList[0].revents & POLLIN)
            {
                int clientSock = accept(sock, (sockaddr *)&client, &clientLen);
                if (errno > 0 && errno != EWOULDBLOCK)
                {
                    status = errno;
                    if (clientSock >= 0)
                    {
                        close(clientSock);
                        clientSock = -1;
                    }

                    break;
                }

                if (clientSock >= 0)
                    workQueue.dispatchWork(clientSock);
            }

            if (!m_running)
                break;
        }

        if (sock >= 0)
        {
            close(sock);
            sock = -1;
        }

        // int *work = new int[workers.size()] { -1 };
        int *work = (int *)calloc(workers.size(), sizeof(int));
        for (int *i = work; i < (work + sizeof(int) * workers.size()); i += sizeof(int))
            *i = -1;
        workQueue.dispatchWork(work, workers.size());

        for (size_t i = 0; i < workers.size(); ++i)
            workers[i].join();

        free(work);
        return status;
    }

    void TcpServer::shutdown()
    {
        m_running = false;
    }
} // namespace web

