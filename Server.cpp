#include <arpa/inet.h>
#include <signal.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <unistd.h>
#include <iostream>

#include "webserver.h"

web::Server::Server()
{
}

void web::Server::configure(int port)
{
    m_port = port;
}

void web::Server::configure(Logger &logger)
{
    m_logger = std::unique_ptr<Logger>(&logger);
}

void web::Server::route(http::Method method, std::string path, http::RouteHandler handler)
{
    std::stringstream log;
    log << "register handler for route " << path << " with method " << method;
    if (m_logger != nullptr)
        m_logger->debug(log.str());
    // m_routeMap.insert(method, std::vector<http::RoutePair>());
    // m_routeMap[method].push_back(http::RoutePair(path, handler));
}

void web::Server::route(http::Method method, const char *path, std::function<void(const http::Request &req, http::Response &resp)> handler)
{
    route(method, std::string(path), handler);
}

int web::Server::run()
{
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0)
    {
        return errno;
    }

    sockaddr_in saddr;
    saddr.sin_family = AF_INET;
    saddr.sin_addr.s_addr = INADDR_ANY;
    saddr.sin_port = htons(m_port);

    if (bind(sock, (sockaddr *)&saddr, sizeof(saddr)) < 0)
    {
        return errno;
    }

    if (listen(sock, 20) < 0)
    {
        return errno;
    }

    int status = 0;
    sockaddr_in client;
    socklen_t clientLen = sizeof(client);
    int running = true;
    for (;;) {
        int clientSock = accept(sock, (sockaddr *)&client, &clientLen);
        if (errno > 0 && errno != EWOULDBLOCK)
        {
            status = errno;
            if (clientSock >= 0)
            {
                close(clientSock);
                clientSock = -1;
            }
            if (sock >= 0)
            {
                close(sock);
                sock = -1;
            }
            break;
        }

        if (clientSock >= 0)
        {
            timeval timeout;
            timeout.tv_sec = 0;
            timeout.tv_usec = 100000; // 100ms

            if (setsockopt(clientSock, SOL_SOCKET, SO_RCVTIMEO, (void *)&timeout, sizeof(timeval)) < 0)
            {
                std::cerr << "Failed to set socket option: " << std::strerror(errno) << std::endl;
                std::string errStr =  "HTTP/1.1 500 Internal Error\r\n\r\n";
                send(clientSock, errStr.c_str(), errStr.length(), 0);
                close(clientSock);
                continue;
            }

            http::Request req = http::parseRequest(clientSock);
            req.socket(sock);

            std::stringstream output;
            output << "HTTP/1.1 200 Ok\r\n"
                << "Content-Type: text/plain\r\n"
                << "Content-Length: 2\r\n"
                << "\r\n"
                << "OK";

            std::string outStr = output.str();
            int bytesSent = send(clientSock, outStr.c_str(), outStr.length(), 0);

            close(clientSock);

            char clientIp[INET_ADDRSTRLEN] = { 0 };
            inet_ntop(client.sin_family, &client.sin_addr, clientIp, INET_ADDRSTRLEN);

            for (auto &header : req.headers())
            {
                std::cout
                    << header.first << ": "
                    << header.second << std::endl;
            }

            std::cout << clientIp << " "
                << "- - "
                << "[%t] "
                << '"' << req.method()
                << " " << req.path()
                << " " << req.version() << '"'
                << " 200 " << bytesSent
                << std::endl;
        }

        if (!running)
            break;
    }

    return status;
}

