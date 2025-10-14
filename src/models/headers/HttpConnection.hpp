#ifndef HTTPCONNECTION_HPP
#define HTTPCONNECTION_HPP

#include <string>

class HttpRequest;
class Router;

enum ParseState
{
    PS_RequestLine,
    PS_Headers,
    PS_BodyContentLength,
    PS_BodyChunkSize,
    PS_BodyChunkData,
    PS_BodyChunkCRLF,
    PS_Trailers,
    PS_Done,
    PS_Error
};

class HttpConnection
{
    int fd;
    std::string inbuf;
    std::string outbuf;
    ParseState state;
    HttpRequest *req;
    size_t expectedBody;
    size_t currentChunkSize;
    bool headLike;
    bool shouldClose;
    size_t headersSize;
    size_t headerCount;
    bool _closed;
    bool expectContinue;
    bool sentContinue;
    Router *router;

    static const size_t MAX_START_LINE = 8192;
    static const size_t MAX_HEADERS_SIZE = 32768;
    static const size_t MAX_HEADERS_COUNT = 100;
    static const size_t MAX_BODY_SIZE = 10 * 1024 * 1024;

  public:
    HttpConnection(int f, Router *r);
    ~HttpConnection();

    void onReadable();
    void onWritable();
    bool wantsWrite() const;
    bool closed() const;

  private:
    bool readSome();
    bool writeSome();
    bool parseStep();
    bool sendError(int status, const std::string &reason, const std::string &msgBody);
    bool finalizeAndQueueResponse(struct HttpResponse &res, bool headOnly);

    static bool parseStartLine(const std::string &line, std::string &method, std::string &target, std::string &version);
    static bool parseChunkSizeLine(const std::string &line, size_t &outSize);
    static bool popLineCRLF(std::string &buf, std::string &outLine);
};

#endif
