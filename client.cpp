#include <iostream>
#include <sys/socket.h>
// #include <netinet/in.h>
#include <arpa/inet.h>
#include <string>
#include <unistd.h>

int main(int argc, const char *argv[])
{
    if (argc < 2)
    {
        std::wcerr << L"Too few arguments: Need PORT" << std::endl;
        return EXIT_FAILURE;
    }
    int sock = socket(AF_INET, SOCK_STREAM, 0);

    sockaddr_in saddr = { 0 };
    saddr.sin_family = AF_INET;
    saddr.sin_port = htons(atoi(argv[1]));
    inet_pton(AF_INET, "127.0.0.1", &saddr.sin_addr);

    if (connect(sock, (sockaddr *)&saddr, sizeof(saddr)) < 0)
    {
        std::wcerr << L"Failed to connect to remote host" << std::endl;
        return EXIT_FAILURE;
    }

    std::string out = "GET / HTTP/1.1\r\n";
    int bytesSent = send(sock, out.c_str(), out.size(), 0);

    std::wcout << L"Send " << bytesSent << L" bytes." << std::endl;

    char inBuf[1024] = { 0 };
    int bytesRead = recv(sock, inBuf, sizeof(inBuf), 0);
    inBuf[bytesRead] = 0;

    std::wcout
        << inBuf << std::endl
        << L"Recv " << bytesRead << L" bytes." << std::endl;

    close(sock);

    return 0;
}
