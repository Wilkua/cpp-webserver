#include <sys/socket.h>

#include <HttpServer.h>

namespace web
{
namespace http
{
    Request parseRequest(int sock)
    {
        http::Request req;

        char inBuf[WINDOW_SIZE] = { 0 };
        int bytesRead = recv(sock, (void *)inBuf, WINDOW_SIZE, 0);
        // TODO(william): Check for and handle read errors
        uint8_t step = 0;
        int subStep = 0;
        std::string httpVersion; // request-line parser HTTP version
        std::string method; // request-line parser method
        std::string path; // request-line parser path
        std::string key; // header parser key
        std::string val; // header parser value
        std::u32string rawBody;
        // Step 0 - Request line
        //   Substep 0 - Method
        //   Substep 1 - URL Path
        //   Substep 2 - HTTP Version
        // Step 1 - Headers
        //   Substep 0 - Header key
        //   Substep 1 - Header value
        // Step 2 - Body
        // TODO(william): This needs a timer to avoid slow-loris attacks
        do
        {
            int pos = 0;
            if (step == 0)
            {
                while (pos < bytesRead)
                {
                    // TODO(william): this should check for end of buffer!
                    if (inBuf[pos] == '\r')
                    {
                        pos = pos + 2;
                        ++step;
                        subStep = 0;
                        break;
                    }
                    if (inBuf[pos] == ' ' && pos < bytesRead)
                    {
                        ++pos;
                        ++subStep;
                    }
                    else if (inBuf[pos] == ' ')
                    {
                        // space at end of buffer
                        ++subStep;
                        break; // read more buffer
                    }

                    if (subStep == 0)
                        method.append(1, inBuf[pos]);
                    if (subStep == 1)
                        path.append(1, inBuf[pos]);
                    if (subStep == 2)
                        httpVersion.append(1, inBuf[pos]);
                    ++pos;
                } // while (pos < bytesRead)

                req.method(method);
                req.path(path);
                req.version(httpVersion);
            }
            if (step == 1)
            {
                while (pos < bytesRead)
                {
                    if (inBuf[pos] == '\r')
                    {
                        if (key.empty() && val.empty())
                        {
                            // \r\n on blank line - end of headers
                            pos += 2; // NOTE(william): could cause a problem if it crosses a buffer boudnary!
                            ++step;
                            subStep = 0;
                            break;
                        }
                        req.header(key, val);
                        pos += 2;
                        subStep = 0;
                        val.clear();
                        key.clear();
                    }
                    else if (subStep == 0 && inBuf[pos] == ':')
                    {
                        subStep = 1;
                        if ((pos + 1) < bytesRead && inBuf[pos + 1] == ' ')
                            pos += 2;
                        else
                            ++pos;
                    }
                    if (inBuf[pos] == '\r')
                        continue;
                    if (subStep == 0)
                        key.append(1, inBuf[pos]);
                    else
                        val.append(1, inBuf[pos]);
                    ++pos;
                } // while (pos < bytesRead)
            }
            if (step == 2)
            {
                // body content
                uint64_t contentLength = std::stoull(req.header("Content-Type").value_or("0"));

                // This may be overly-aggressive
                if (contentLength == 0)
                    break;

                // If pos >= bytesRead then we went past the chunk boundary and we need to read
                // more data. This shouldn't happen often.
                while (pos < bytesRead)
                {
                    // NOTE(william): this is probably not an efficient way to move data
                    rawBody.push_back(inBuf[pos]);
                    ++pos;
                } // while (pos < bytesRead)
                if (rawBody.length() >= contentLength)
                    break;
            } // if (step == 2)

            bytesRead = recv(sock, (void *)inBuf, WINDOW_SIZE, 0);
        } while (bytesRead > 0);

        req.body(rawBody);

        return req;
    }
} // Http
} // web

