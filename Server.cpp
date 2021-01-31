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

#include <iostream>

#include "webserver.h"

typedef std::queue<int>* WorkQueuePtr;

std::mutex workQueueMutex;
std::condition_variable queueCv;

void threadHandleConnection(WorkQueuePtr queue, web::Server *srv)
{
    if (queue == nullptr)
        return;

    for (;;)
    {
        std::unique_lock<std::mutex> lk(workQueueMutex);
        queueCv.wait(lk, [=]{ return !(queue->empty()); });

        int workSocket = queue->front(); // Take work
        queue->pop();
        lk.unlock(); // don't hold the lock too long

        if (workSocket == -1) // quit signal
            break;

        timeval timeout;
        timeout.tv_sec = 0;
        timeout.tv_usec = 100000; // 100ms

        if (setsockopt(workSocket, SOL_SOCKET, SO_RCVTIMEO, (void *)&timeout, sizeof(timeval)) < 0)
        {
            std::cerr << "Failed to set socket option: " << std::strerror(errno) << std::endl;
            std::string errStr = "HTTP/1.1 500 Internal Error\r\n\r\n";
            send(workSocket, errStr.c_str(), errStr.length(), 0);
            close(workSocket);
            continue;
        }

        web::http::Request req = web::http::parseRequest(workSocket);
        req.socket(workSocket);

        std::shared_ptr<std::vector<web::http::RoutePair> > procList = nullptr;
        if (req.method() == "GET")
        {
            procList = srv->getProc();
        }

        web::http::RouteHandler handler = web::http::Handlers::NotFound;
        if (procList != nullptr)
            for (web::http::RoutePair &v : *srv->getProc())
            {
                if (v.first == req.path())
                {
                    handler = v.second;
                    break;
                }
            }

        web::http::Response res(req.version());

        // Run through all middleware before going to route requests
        for (auto f : *srv->middleware())
            f(req, res);

        /* TODO(william): Possible better solution is to have all
         *   requests handled by onRequest middleware and handlers
         *   with methods and paths are given a wrapper which
         *   checks if the method and path match before running
         *   the handler.
         */
        if (!res.skipRoute())
            handler(req, res);

        std::string outStr = res.text();
        send(workSocket, outStr.c_str(), outStr.length(), 0);

        close(workSocket);
    } // for(;;)
}

web::Server::Server() : m_running(false)
{
    m_getProc = std::make_unique<std::vector<http::RoutePair> >();
    m_middleware = new std::vector<http::RouteHandler>();
}

web::Server::~Server()
{
    if (m_middleware != nullptr)
    {
        delete m_middleware;
        m_middleware = nullptr;
    }
}

Logger *web::Server::logger()
{
    return m_logger;
}

void web::Server::configure(int port)
{
    m_port = port;
}

void web::Server::configure(Logger *logger)
{
    m_logger = logger;
}

std::shared_ptr<std::vector<web::http::RoutePair> > web::Server::getProc()
{
    return m_getProc;
}

std::vector<web::http::RouteHandler> *web::Server::middleware()
{
    return m_middleware;
}

void web::Server::onRequest(http::RouteHandler handler)
{
    if (m_logger != nullptr)
        m_logger->debug("Adding side-effect middleware");

    m_middleware->push_back(handler);
}

void web::Server::route(http::Method method, std::string path, http::RouteHandler handler)
{
    std::stringstream log;
    log << "register handler for route " << path << " with method " << method;
    if (m_logger != nullptr)
        m_logger->debug(log.str());
    if (method == http::Method::GET)
        m_getProc->push_back(http::RoutePair(path, handler));
}

void web::Server::route(http::Method method, const char *path, std::function<void(const http::Request &req, http::Response &resp)> handler)
{
    route(method, std::string(path), handler);
}

int web::Server::run()
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

    std::queue<int> workQueue;
    std::vector<std::thread> workers;

    size_t numThreads = std::thread::hardware_concurrency();
    if (numThreads < 2)
        numThreads = 1; // 1 or 0 response should have 1 worker thread
    else
        --numThreads;

    for (size_t i = 0; i < numThreads; ++i)
        workers.push_back(std::thread(threadHandleConnection, &workQueue, this));

    std::stringstream log;
    log << "found threads = " << numThreads << ", workers = " << workers.size();
    if (m_logger != nullptr)
        m_logger->debug(log.str());

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
            {
                std::lock_guard<std::mutex> g(workQueueMutex);
                workQueue.push(clientSock);
                queueCv.notify_one();
            }
        }

        if (!m_running)
            break;
    }

    if (sock >= 0)
    {
        close(sock);
        sock = -1;
        if (m_logger != nullptr)
            m_logger->info("Closed incoming socket - stopped accepting connections");
    }

    if (m_logger != nullptr)
        m_logger->info("Shutting down - waiting on workers to quit ...");

    {
        std::lock_guard<std::mutex> g(workQueueMutex);
        for (size_t i = 0; i < workers.size(); ++i)
            workQueue.push(-1);
    }
    queueCv.notify_all();

    for (size_t i = 0; i < workers.size(); ++i)
        workers[i].join();

    return status;
}

void web::Server::shutdown()
{
    if (m_logger != nullptr)
        m_logger->info("Received shutdown command");
    m_running = false;
}

