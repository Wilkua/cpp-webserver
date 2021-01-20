#pragma once

#include <cstdint>
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

#ifndef POLL_TIMEOUT
#define POLL_TIMEOUT 1000
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
        std::string m_body; // this should probably be represented by a binary data type
        std::map<std::string, std::string> m_headers;
        uint16_t m_statusCode;
        std::string m_statusMessage;
        std::string m_version;

    public:
        Response(const std::string &version) : m_statusCode(200), m_statusMessage("Ok"), m_version(version) {};

        std::string text();

        void body(const std::string &b);

        std::optional<std::string> header(const std::string &key);
        void header(const std::string &key, const std::string &value);
        std::map<std::string, std::string> headers() const;

        uint16_t status() const;
        void status(uint16_t s);

        std::string statusMessage() const;
        void statusMessage(const std::string &msg);
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
        std::optional<std::string> header(const std::string &key);
        void header(const std::string &key, const std::string &value);
        std::map<std::string, std::string> headers() const;

        std::string method() const;
        void method(const std::string &method);

        std::optional<std::string> param(const std::string &key);
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
        Logger *m_logger;
        int m_port;
        std::shared_ptr<std::vector<http::RoutePair> > m_getProc;
        bool m_running;

        // http::Request parseRequest(int sock);

    public:
        Server();

        Logger *logger();

        void configure(int port);
        void configure(Logger *logger);
        void route(http::Method method, std::string path, http::RouteHandler handler);
        void route(http::Method method, const char *path, http::RouteHandler handler);
        std::shared_ptr<std::vector<http::RoutePair> > getProc();
        int run();
        void shutdown();
    };
} // namespace web

