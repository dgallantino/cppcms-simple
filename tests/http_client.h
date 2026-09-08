#ifndef TESTS_HTTP_CLIENT_H
#define TESTS_HTTP_CLIENT_H

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <cerrno>
#include <cstdlib>
#include <cstring>
#include <cstdint>
#include <sstream>
#include <string>

static const char *TEST_HTTP_HOST = "127.0.0.1";
static const int TEST_HTTP_PORT = 18080;
static const char *TEST_TOKEN = "person-test-token";

struct HttpResult
{
    int status;
    std::string body;
    std::string raw;
};

inline bool wait_for_http_server(int attempts, int sleep_ms)
{
    for (int i = 0; i < attempts; ++i) {
        int fd = ::socket(AF_INET, SOCK_STREAM, 0);
        if (fd < 0)
            return false;
        sockaddr_in addr;
        std::memset(&addr, 0, sizeof(addr));
        addr.sin_family = AF_INET;
        addr.sin_port = htons(static_cast<uint16_t>(TEST_HTTP_PORT));
        inet_pton(AF_INET, TEST_HTTP_HOST, &addr.sin_addr);
        int rc = ::connect(fd, reinterpret_cast<sockaddr *>(&addr), sizeof(addr));
        ::close(fd);
        if (rc == 0)
            return true;
        ::usleep(sleep_ms * 1000);
    }
    return false;
}

inline HttpResult http_request(const char *method, const char *path,
                               const char *token, const char *body)
{
    HttpResult result;
    result.status = 0;

    int fd = ::socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) {
        result.raw = "socket() failed";
        return result;
    }

    timeval tv;
    tv.tv_sec = 5;
    tv.tv_usec = 0;
    setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
    setsockopt(fd, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv));

    sockaddr_in addr;
    std::memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(static_cast<uint16_t>(TEST_HTTP_PORT));
    inet_pton(AF_INET, TEST_HTTP_HOST, &addr.sin_addr);
    if (::connect(fd, reinterpret_cast<sockaddr *>(&addr), sizeof(addr)) != 0) {
        result.raw = "connect() failed";
        ::close(fd);
        return result;
    }

    std::ostringstream req;
    req << method << " " << path << " HTTP/1.0\r\n";
    req << "Host: " << TEST_HTTP_HOST << ":" << TEST_HTTP_PORT << "\r\n";
    req << "Connection: close\r\n";
    if (token != 0)
        req << "Token: " << token << "\r\n";
    if (body != 0) {
        req << "Content-Type: application/json\r\n";
        req << "Content-Length: " << std::strlen(body) << "\r\n";
    }
    req << "\r\n";
    if (body != 0)
        req << body;

    std::string reqStr = req.str();
    const char *p = reqStr.c_str();
    size_t left = reqStr.size();
    while (left > 0) {
        ssize_t n = ::send(fd, p, left, 0);
        if (n < 0) {
            result.raw = "send() failed";
            ::close(fd);
            return result;
        }
        p += n;
        left -= static_cast<size_t>(n);
    }

    std::string raw;
    char buf[4096];
    for (;;) {
        ssize_t n = ::recv(fd, buf, sizeof(buf), 0);
        if (n > 0) {
            raw.append(buf, static_cast<size_t>(n));
            continue;
        }
        break;
    }
    ::close(fd);

    result.raw = raw;
    std::string::size_type lineEnd = raw.find("\r\n");
    if (lineEnd == std::string::npos)
        lineEnd = raw.find('\n');
    if (lineEnd != std::string::npos) {
        std::string statusLine = raw.substr(0, lineEnd);
        std::string::size_type sp1 = statusLine.find(' ');
        if (sp1 != std::string::npos) {
            result.status = std::atoi(statusLine.c_str() + sp1 + 1);
        }
    }

    std::string::size_type hdrEnd = raw.find("\r\n\r\n");
    if (hdrEnd != std::string::npos)
        result.body = raw.substr(hdrEnd + 4);
    else {
        std::string::size_type alt = raw.find("\n\n");
        if (alt != std::string::npos)
            result.body = raw.substr(alt + 2);
        else
            result.body = raw;
    }
    return result;
}

#endif
