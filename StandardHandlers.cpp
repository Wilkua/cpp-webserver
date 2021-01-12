#include "webserver.h"

namespace web
{
namespace http
{
namespace Handlers
{
    void NotFound(const web::http::Request &req, web::http::Response &resp)
    {
        resp.status(404);
        resp.statusMessage("Not Found");
        resp.body("Not Found");
    }
} // namespace Handlers
} // namespace http
} // namespace web
