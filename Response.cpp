#include "webserver.h"

namespace web
{
namespace http
{
    std::string Response::text()
    {
        std::stringstream out;
        out << m_version
            << " " << m_statusCode
            << " " << m_statusMessage
            << "\r\n";

        for (auto &p : m_headers)
            out << p.first << ": " << p.second << "\r\n";

        out << "\r\n" << m_body;

        return out.str();
    }

    void Response::body(const std::string &b)
    {
        m_headers["Content-Length"] = std::to_string(b.size());
        m_body = b;
    }

    std::optional<std::string> Response::header(const std::string &key)
    {
        if (m_headers.count(key) > 0)
            return m_headers[key];
        return {};
    }

    void Response::header(const std::string &key, const std::string &value)
    {
        m_headers[key] = value;
    }

    std::map<std::string, std::string> Response::headers() const
    {
        return m_headers;
    }

    uint16_t Response::status() const
    {
        return m_statusCode;
    }

    void Response::status(uint16_t s)
    {
        m_statusCode = s;
    }

    std::string Response::statusMessage() const
    {
        return m_statusMessage;
    }

    void Response::statusMessage(const std::string &msg)
    {
        m_statusMessage = msg;
    }
} // namespace http
} // namespace web
