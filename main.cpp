#include <cstring>
#include <iostream>
#include <signal.h>

#include "Logger.h"
#include "webserver.h"

static web::Server *g_server;

void onSig(int signum)
{
    if (g_server != nullptr)
        g_server->shutdown();
}

int main(int argc, const char *argv[])
{
    // Logger logger("servd.log", "error.log");
    // Log as "[2020-09-08T07:33:19-02:00](INFO) Starting server..."
    // logger.format("[%Y-%M-%DT%H:%M:%S%Z](%l) %m");
    Logger logger;

    g_server = new web::Server();

    g_server->configure(8080);
    g_server->configure(&logger);

    g_server->route(web::http::Method::GET, "/", [](const web::http::Request &req, web::http::Response &resp)
    {
        // srv.logger()->debug("route /");
        // std::string name = req.params["name"].value_or("World");
        // std::string body = "Hello, ";
        // body.append(name);
        // body.append("!");

        resp.header("Content-Type", "text/plain; charset=utf-8");
        resp.body("Hello, world!");
    });

    g_server->route(web::http::Method::GET, "/test", [](const web::http::Request &req, web::http::Response &res)
    {
        res.header("Content-Type", "text/html; charset=utf-8");
        std::stringstream html;
        html << "<!DOCTYPE html>"
            << "<html><head>"
            << "<title>Test Page</title>"
            << "<meta charset=\"utf-8\">"
            << "</head><body>"
            << "<h1>Testing</h1>"
            << "</body></html>";
        res.body(html.str());
    });

    signal(SIGINT, onSig);

    {
        std::stringstream log;
        log << "Buffer width = " << INBUF_SIZE << " bytes";
        logger.debug(log.str());
        logger.info("Starting server");
    }
    int retCode = g_server->run();
    if (retCode > 0)
    {
        std::stringstream log;
        log << "retcode = " << retCode
            << " " << strerror(retCode);
        logger.error(log.str());
    }

    if (g_server != nullptr)
    {
        delete g_server;
        g_server = nullptr;
    }

    return retCode;
}
