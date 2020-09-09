#include "webserver.h"

namespace web
{
namespace http
{
    std::string Request::header(const std::string &key)
    {
        if (m_headers.count(key) > 0)
            return m_headers[key];
        return std::string();
    }

    void Request::header(const std::string &key, const std::string &value)
    {
        m_headers[key] = value;
    }

    std::map<std::string, std::string> Request::headers() const
    {
        return m_headers;
    }

    std::string Request::method() const
    {
        return m_method;
    }

    void Request::method(const std::string &method)
    {
        m_method = method;
    }

    std::string Request::param(const std::string &key)
    {
        if (m_params.count(key) > 0)
            return m_params[key];
        return std::string();
    }

    void Request::param(const std::string &key, const std::string &value)
    {
        m_params[key] = value;
    }

    std::map<std::string, std::string> Request::params() const
    {
        return m_params;
    }

    std::string Request::path() const
    {
        return m_path;
    }

    void Request::path(const std::string &path)
    {
        m_path = path;
    }

    std::string Request::version() const
    {
        return m_httpVersion;
    }

    void Request::version(const std::string &version)
    {
        m_httpVersion = version;
    }

    int Request::socket() const
    {
        return m_socket;
    }

    void Request::socket(int sock)
    {
        m_socket = sock;
    }
} // namespace http
} // anmespace web

