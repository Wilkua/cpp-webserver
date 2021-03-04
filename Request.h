#pragma once

#include <cstdint>
#include <map>
#include <optional>
#include <string>

namespace web
{
namespace http
{
    class Request
    {
    private:
        std::u32string m_body;
        std::map<std::string, std::string> m_headers;
        std::string m_httpVersion;
        std::string m_method; // for simplicity
        std::map<std::string, std::string> m_params;
        std::string m_path;
        int m_socket;

    public:
        // Property definitions
        std::u32string body() const;
        void body(const std::u32string &b);

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
} // namespace http
} // namespace web

