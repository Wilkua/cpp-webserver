#pragma once

#include <cstdint>
#include <map>
#include <optional>
#include <string>

namespace web
{
namespace http
{
    class Response
    {
    private:
        std::string m_body; // this should probably be represented by a binary data type
        std::map<std::string, std::string> m_headers;
        bool m_skipRoute;
        uint16_t m_statusCode;
        std::string m_statusMessage;
        std::string m_version;

    public:
        Response(const std::string &version) : m_skipRoute(false), m_statusCode(200), m_statusMessage("Ok"), m_version(version) {};

        std::string text();

        void body(const std::string &b);

        std::optional<std::string> header(const std::string &key);
        void header(const std::string &key, const std::string &value);
        std::map<std::string, std::string> headers() const;

        bool skipRoute() const;
        void skipRoute(bool b);
        uint16_t status() const;
        void status(uint16_t s);

        std::string statusMessage() const;
        void statusMessage(const std::string &msg);
    };
} // namespace http
} // namespace web

