#pragma once

#include <cstdint>
#include <functional>
#include <map>
#include <optional>
#include <sstream>
#include <string>
#include <vector>

#include <TcpServer.h>
#include <Logger.h>
#include <Request.h>
#include <Response.h>

#ifndef WINDOW_SIZE
#define WINDOW_SIZE 4096
#endif

namespace web
{
namespace http
{
    enum Method
    {
        DELETE,
        GET,
        POST,
        PUT
    };

    typedef std::function<void(const http::Request &req, http::Response &resp)> RouteHandler;
    typedef std::pair<std::string, RouteHandler> RoutePair;

    Request parseRequest(int sock);

    namespace Handlers
    {
        void NotFound(const http::Request &, http::Response &);
    } // namespace Handlers

    class HttpServer : public TcpServer
    {
    private:
        Logger *m_logger;
        // int m_port;
        std::shared_ptr<std::vector<RoutePair> > m_getProc;
        std::vector<RouteHandler> *m_httpMiddleware;
        // bool m_running;

        // Request parseRequest(int sock);

    public:
        HttpServer();
        ~HttpServer();

        Logger *logger();

        // void configure(int port);
        using TcpServer::configure;
        void configure(Logger *logger);
        std::shared_ptr<std::vector<RoutePair> > getProc();
        std::vector<RouteHandler> *middleware();
        void onRequest(RouteHandler handler);
        void route(Method method, std::string path, RouteHandler handler);
        void route(Method method, const char *path, RouteHandler handler);
        // int run();
        // void shutdown();
    };
} // namespace http
} // namespace web


