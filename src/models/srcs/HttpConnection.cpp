#include "HttpConnection.hpp"
#include "HttpRequest.hpp"
#include "HttpResponse.hpp"
#include "HttpRouter.hpp"
#include "HttpUtils.hpp"
#include <sys/socket.h>
#include <unistd.h>
#include <cerrno>

HttpConnection::HttpConnection(int f, Router* r)
    : fd(f), state(PS_RequestLine), req(0), expectedBody(0), currentChunkSize(0),
      headLike(false), shouldClose(false), headersSize(0), headerCount(0), _closed(false),
      expectContinue(false), sentContinue(false), router(r) {}

HttpConnection::~HttpConnection() {
    if (fd >= 0) close(fd);
    delete req;
}

void HttpConnection::onReadable() {
    if (!readSome()) {
        _closed = true;
        return;
    }
    while (parseStep()) {}
}

void HttpConnection::onWritable() {
    if (!writeSome()) {
        _closed = true;
    }
}

bool HttpConnection::wantsWrite() const {
    return !outbuf.empty();
}

bool HttpConnection::closed() const {
    return _closed;
}

bool HttpConnection::readSome() {
    char buf[4096];
    ssize_t n = recv(fd, buf, sizeof(buf), 0);
    if (n < 0) {
        if (errno == EAGAIN || errno == EWOULDBLOCK) return true;
        return false;
    }
    if (n == 0) return false;
    inbuf.append(buf, n);
    return true;
}

bool HttpConnection::writeSome() {
    while (!outbuf.empty()) {
        ssize_t n = send(fd, outbuf.data(), outbuf.size(), 0);
        if (n < 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) return true;
            return false;
        }
        outbuf.erase(0, n);
    }
    return true;
}

bool HttpConnection::popLineCRLF(std::string& buf, std::string& outLine) {
    size_t pos = buf.find("\r\n");
    if (pos == std::string::npos) return false;
    outLine = buf.substr(0, pos);
    buf.erase(0, pos + 2);
    return true;
}

bool HttpConnection::parseStartLine(const std::string& line, std::string& method, std::string& target, std::string& version) {
    size_t sp1 = line.find(' ');
    if (sp1 == std::string::npos) return false;
    size_t sp2 = line.find(' ', sp1 + 1);
    if (sp2 == std::string::npos) return false;
    method = line.substr(0, sp1);
    target = line.substr(sp1 + 1, sp2 - sp1 - 1);
    version = line.substr(sp2 + 1);
    return !method.empty() && !target.empty() && !version.empty();
}

bool HttpConnection::parseChunkSizeLine(const std::string& line, size_t& outSize) {
    size_t semi = line.find(';');
    std::string hex = (semi == std::string::npos) ? line : line.substr(0, semi);
    hex = trim(hex);
    if (hex.empty()) return false;
    outSize = parseHex(hex);
    return true;
}

bool HttpConnection::parseStep() {
    if (state == PS_RequestLine) {
        if (inbuf.size() > MAX_START_LINE) {
            return sendError(414, "Request-URI Too Large", "");
        }
        std::string line;
        if (!popLineCRLF(inbuf, line)) return false;
        if (line.size() > MAX_START_LINE) {
            return sendError(414, "Request-URI Too Large", "");
        }
        std::string method, target, version;
        if (!parseStartLine(line, method, target, version)) {
            return sendError(400, "Bad Request", "Invalid request line");
        }
        req = makeRequestByMethod(method);
        if (!req) {
            return sendError(501, "Not Implemented", "Method not supported");
        }
        req->setMethod(method);
        req->setVersion(version);
        std::string cleanPath;
        std::map<std::string, std::string> qmap;
        HttpRequest::parseQuery(target, cleanPath, qmap);
        req->setPath(cleanPath);
        req->setQuery(qmap);
        headLike = (toLowerStr(method) == "head");
        state = PS_Headers;
        headersSize = 0;
        headerCount = 0;
        return true;
    } else if (state == PS_Headers) {
        std::string line;
        if (!popLineCRLF(inbuf, line)) {
            if (headersSize + inbuf.size() > MAX_HEADERS_SIZE) {
                return sendError(431, "Request Header Fields Too Large", "");
            }
            return false;
        }
        headersSize += line.size() + 2;
        if (headersSize > MAX_HEADERS_SIZE) {
            return sendError(431, "Request Header Fields Too Large", "");
        }
        if (line.empty()) {
            if (headerCount > MAX_HEADERS_COUNT) {
                return sendError(431, "Request Header Fields Too Large", "");
            }
            std::map<std::string, std::string>::const_iterator it = req->getHeaders().find("expect");
            if (it != req->getHeaders().end() && toLowerStr(it->second).find("100-continue") != std::string::npos) {
                expectContinue = true;
            }
            if (req->isChunked()) {
                if (expectContinue && !sentContinue) {
                    outbuf += "HTTP/1.1 100 Continue\r\n\r\n";
                    sentContinue = true;
                }
                state = PS_BodyChunkSize;
            } else {
                size_t cl = req->contentLength();
                if (cl > 0) {
                    if (cl > MAX_BODY_SIZE) {
                        return sendError(413, "Payload Too Large", "");
                    }
                    if (expectContinue && !sentContinue) {
                        outbuf += "HTTP/1.1 100 Continue\r\n\r\n";
                        sentContinue = true;
                    }
                    expectedBody = cl;
                    state = PS_BodyContentLength;
                } else {
                    state = PS_Done;
                }
            }
            return true;
        }
        std::string k, v;
        if (!HttpRequest::parseHeaderLine(line, k, v)) {
            return sendError(400, "Bad Request", "Invalid header");
        }
        req->addHeader(k, v);
        headerCount++;
        if (headerCount > MAX_HEADERS_COUNT) {
            return sendError(431, "Request Header Fields Too Large", "");
        }
        return true;
    } else if (state == PS_BodyContentLength) {
        if (inbuf.size() >= expectedBody) {
            req->appendBody(inbuf.substr(0, expectedBody));
            inbuf.erase(0, expectedBody);
            state = PS_Done;
            return true;
        }
        return false;
    } else if (state == PS_BodyChunkSize) {
        std::string line;
        if (!popLineCRLF(inbuf, line)) return false;
        size_t sz;
        if (!parseChunkSizeLine(line, sz)) {
            return sendError(400, "Bad Request", "Invalid chunk size");
        }
        if (sz == 0) {
            state = PS_Trailers;
            return true;
        }
        if (req->getBody().size() + sz > MAX_BODY_SIZE) {
            return sendError(413, "Payload Too Large", "");
        }
        currentChunkSize = sz;
        state = PS_BodyChunkData;
        return true;
    } else if (state == PS_BodyChunkData) {
        if (inbuf.size() >= currentChunkSize) {
            req->appendBody(inbuf.substr(0, currentChunkSize));
            inbuf.erase(0, currentChunkSize);
            state = PS_BodyChunkCRLF;
            return true;
        }
        return false;
    } else if (state == PS_BodyChunkCRLF) {
        if (inbuf.size() >= 2) {
            if (inbuf[0] != '\r' || inbuf[1] != '\n') {
                return sendError(400, "Bad Request", "Expected CRLF after chunk");
            }
            inbuf.erase(0, 2);
            state = PS_BodyChunkSize;
            return true;
        }
        return false;
    } else if (state == PS_Trailers) {
        std::string line;
        if (!popLineCRLF(inbuf, line)) return false;
        if (line.empty()) {
            state = PS_Done;
            return true;
        }
        return true;
    } else if (state == PS_Done) {
        std::string err;
        if (!req->validate(err)) {
            return sendError(400, "Bad Request", err);
        }
        HttpResponse res;
        bool routed = router->dispatch(*req, res);
        if (!routed) {
            std::string allow = router->allowForPath(req->getPath());
            if (!allow.empty()) {
                res.status = 405;
                res.reason = "Method Not Allowed";
                res.setHeader("Allow", allow);
                res.body = "Method Not Allowed";
                res.setHeader("Content-Type", "text/plain");
                res.setHeader("Content-Length", itoa_custom(res.body.size()));
            } else {
                res.status = 404;
                res.reason = "Not Found";
                res.body = "Not Found";
                res.setHeader("Content-Type", "text/plain");
                res.setHeader("Content-Length", itoa_custom(res.body.size()));
            }
        }
        std::map<std::string, std::string>::const_iterator cit = req->getHeaders().find("connection");
        if (cit != req->getHeaders().end() && toLowerStr(cit->second) == "close") {
            shouldClose = true;
        }
        return finalizeAndQueueResponse(res, headLike);
    } else if (state == PS_Error) {
        _closed = true;
        return false;
    }
    return false;
}

bool HttpConnection::sendError(int status, const std::string& reason, const std::string& msgBody) {
    HttpResponse res;
    res.status = status;
    res.reason = reason;
    res.body = msgBody;
    res.setHeader("Content-Type", "text/plain");
    res.setHeader("Content-Length", itoa_custom(res.body.size()));
    res.setHeader("Connection", "close");
    shouldClose = true;
    outbuf += res.serialize(false);
    state = PS_Error;
    return false;
}

bool HttpConnection::finalizeAndQueueResponse(HttpResponse& res, bool headOnly) {
    if (res.headers.find("Connection") == res.headers.end()) {
        if (shouldClose) {
            res.setHeader("Connection", "close");
        } else {
            res.setHeader("Connection", "keep-alive");
        }
    }
    outbuf += res.serialize(headOnly);
    if (shouldClose) {
        _closed = true;
        return false;
    }
    delete req;
    req = 0;
    state = PS_RequestLine;
    expectedBody = 0;
    currentChunkSize = 0;
    headLike = false;
    headersSize = 0;
    headerCount = 0;
    expectContinue = false;
    sentContinue = false;
    return true;
}
