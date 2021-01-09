#pragma once

#include <functional>
#include <map>
#include <optional>
#include <sstream>
#include <string>
#include <vector>

#include "Logger.h"

#ifndef INBUF_SIZE
#define INBUF_SIZE 1024
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

    class Response
    {
    private:
        std::stringstream m_body;

    public:
        std::map<std::string, std::string> headers;

        void bodyContent(std::string content);
        void bodyContent(const wchar_t *content);
        void bodyContent(const char *content);
    };

    class Request
    {
    private:
        std::map<std::string, std::string> m_headers;
        std::string m_httpVersion;
        std::string m_method; // for simplicity
        std::map<std::string, std::string> m_params;
        std::string m_path;
        int m_socket;

    public:
        // Property definitions
        std::string header(const std::string &key);
        void header(const std::string &key, const std::string &value);
        std::map<std::string, std::string> headers() const;

        std::string method() const;
        void method(const std::string &method);

        std::string param(const std::string &key);
        void param(const std::string &key, const std::string &value);
        std::map<std::string, std::string> params() const;

        std::string path() const;
        void path(const std::string &path);

        std::string version() const;
        void version(const std::string &version);

        int socket() const;
        void socket(int sock);
    };

    typedef std::function<void(const http::Request &req, http::Response &resp)> RouteHandler;
    typedef std::pair<std::string, RouteHandler> RoutePair;

    Request parseRequest(int sock);

    namespace Handlers
    {
        void NotFound(const http::Request &, http::Response &);
    } // namespace Handlers
} // namespace http

    class Server
    {
    private:
        std::unique_ptr<Logger> m_logger;
        int m_port;
        std::unique_ptr<std::vector<http::RoutePair> > m_getProc;

        // http::Request parseRequest(int sock);

    public:
        Server();

        void configure(int port);
        void configure(Logger &logger);
        void route(http::Method method, std::string path, http::RouteHandler handler);
        void route(http::Method method, const char *path, http::RouteHandler handler);
        std::unique_ptr<std::vector<http::RoutePair> > getProc();
        int run();
    };
} // namespace web
