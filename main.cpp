#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <signal.h>

#include <Logger.h>
#include <HttpServer.h>

static web::http::HttpServer *g_server;

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

    g_server = new web::http::HttpServer();

    g_server->configure(8080);
    g_server->configure(&logger);

    g_server->onRequest([](const web::http::Request req, web::http::Response &res)
    {
        if (req.path().substr(0, 7) != "/public")
            return;

        res.skipRoute(true); // don't do route handle

        // THIS IS BAD!
        // You should really get the full path and then check
        // that you're not reading files you shouldn't.
        std::string fileName = req.path().substr(8); // get end of path
        FILE *file = fopen(fileName.c_str(), "rb");
        if (file == NULL) // not open - 404
        {
            web::http::Handlers::NotFound(req, res);
            return;
        }

        fseek(file, 0, SEEK_END);
        size_t fs = ftell(file);
        fseek(file, 0, SEEK_SET);

        void *data = malloc(fs);
        if (data == NULL)
        {
            web::http::Handlers::NotFound(req, res);
            return;
        }

        size_t nread = fread(data, 1, fs, file);
        fclose(file);

        if (nread != fs)
        {
            free(data);
            web::http::Handlers::NotFound(req, res);
            return;
        }

        res.body(std::string((char *)data, fs));

        std::string ext = fileName.substr(fileName.find_last_of(".") + 1);
        std::string typ;
        if (ext == "html")
            typ = "text/html; charset=utf-8";
        else if (ext == "css")
            typ = "text/css; charset=utf-8";
        else if (ext == "js")
            typ = "application/javascript; charset=utf-8";
        else
            typ = "text/plain; charset=utf-8";
        res.header("Content-Type", typ);

        free(data);
    });

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
        log << "Buffer width = " << WINDOW_SIZE << " bytes";
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
