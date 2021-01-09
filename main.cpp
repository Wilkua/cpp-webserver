#include <cstring>
#include <iostream>

#include "Logger.h"
#include "webserver.h"

int main(int argc, const char *argv[])
{
    // Logger logger("servd.log", "error.log");
    // Log as "[2020-09-08T07:33:19-02:00](INFO) Starting server..."
    // logger.format("[%Y-%M-%DT%H:%M:%S%Z](%l) %m");
    Logger logger;

    web::Server server;

    server.configure(8080);
    server.configure(logger);

    server.route(web::http::Method::GET, "/", [](const web::http::Request &req, web::http::Response &resp)
    {
        // srv.logger()->debug("route /");
        // std::string name = req.params["name"].value_or("World");
        // std::string body = "Hello, ";
        // body.append(name);
        // body.append("!");

        // resp.bodyContent(body);
        // resp.header("Content-Type", "text/plain");
    });

    std::stringstream log;
    log << "Buffer width = " << INBUF_SIZE << " bytes";
    logger.debug(log.str());
    logger.info("Starting server");
    int retCode = server.run();
    if (retCode > 0)
    {
        log.clear();
        log << "retcode = " << retCode
            << " " << strerror(retCode);
        logger.error(log.str());
    }

    return retCode;
}
