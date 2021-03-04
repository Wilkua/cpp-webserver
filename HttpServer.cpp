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

#include <HttpServer.h>

namespace web
{
namespace http
{
    static void threadHandleConnection(SocketWorkQueuePtr queue, TcpServer *srv)
    {
        if (queue == nullptr)
            return;

        HttpServer *httpSrv = dynamic_cast<HttpServer *>(srv);
        if (!httpSrv)
            return;

        for (;;)
        {
            int workSocket = queue->findWork();

            if (workSocket == -1) // quit signal
                break;

            timeval timeout;
            timeout.tv_sec = 0;
            timeout.tv_usec = 100000; // 100ms

            if (setsockopt(workSocket, SOL_SOCKET, SO_RCVTIMEO, (void *)&timeout, sizeof(timeval)) < 0)
            {
                std::string errStr = "HTTP/1.1 500 Internal Error\r\n\r\n";
                send(workSocket, errStr.c_str(), errStr.length(), 0);
                close(workSocket);
                continue;
            }

            Request req = parseRequest(workSocket);
            req.socket(workSocket);

            std::shared_ptr<std::vector<RoutePair> > procList = nullptr;
            if (req.method() == "GET")
                procList = httpSrv->getProc();

            RouteHandler handler = Handlers::NotFound;
            if (procList != nullptr)
                for (RoutePair &v : *httpSrv->getProc())
                    if (v.first == req.path())
                    {
                        handler = v.second;
                        break;
                    }

            Response res(req.version());

            // Run through all middleware before going to route requests
            for (auto f : *httpSrv->middleware())
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

    HttpServer::HttpServer()
    {
        m_workThreadFunc = threadHandleConnection;
        m_getProc = std::make_unique<std::vector<RoutePair> >();
        m_httpMiddleware = new std::vector<RouteHandler>();
    }

    HttpServer::~HttpServer()
    {
        if (m_httpMiddleware != nullptr)
        {
            delete m_httpMiddleware;
            m_httpMiddleware = nullptr;
        }
    }

    Logger *HttpServer::logger()
    {
        return m_logger;
    }

    void HttpServer::configure(Logger *logger)
    {
        m_logger = logger;
    }

    std::shared_ptr<std::vector<RoutePair> > HttpServer::getProc()
    {
        return m_getProc;
    }

    std::vector<RouteHandler> *HttpServer::middleware()
    {
        return m_httpMiddleware;
    }

    void HttpServer::onRequest(RouteHandler handler)
    {
        if (m_logger != nullptr)
            m_logger->debug("Adding side-effect middleware");

        m_httpMiddleware->push_back(handler);
    }

    void HttpServer::route(Method method, std::string path, RouteHandler handler)
    {
        std::stringstream log;
        log << "register handler for route " << path << " with method " << method;
        if (m_logger != nullptr)
            m_logger->debug(log.str());
        if (method == Method::GET)
            m_getProc->push_back(RoutePair(path, handler));
    }

    void HttpServer::route(Method method, const char *path, RouteHandler handler)
    {
        route(method, std::string(path), handler);
    }
} // namespace http
} // namespace web

