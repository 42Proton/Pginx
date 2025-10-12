#include "HttpRequest.hpp"
#include "HttpResponse.hpp"
#include "HttpUtils.hpp"

// HttpRequest base class implementation
HttpRequest::HttpRequest() {}
HttpRequest::~HttpRequest() {}

std::string HttpRequest::toLower(const std::string& s) {
    return toLowerStr(s);
}

const std::string& HttpRequest::getMethod() const { return method; }
const std::string& HttpRequest::getPath() const { return path; }
const std::string& HttpRequest::getVersion() const { return version; }
const std::map<std::string, std::string>& HttpRequest::getHeaders() const { return headers; }
const std::string& HttpRequest::getBody() const { return body; }
const std::map<std::string, std::string>& HttpRequest::getQuery() const { return query; }

void HttpRequest::setMethod(const std::string& m) { method = m; }
void HttpRequest::setPath(const std::string& p) { path = p; }
void HttpRequest::setVersion(const std::string& v) { version = v; }
void HttpRequest::addHeader(const std::string& k, const std::string& v) { headers[k] = v; }
void HttpRequest::appendBody(const std::string& data) { body.append(data); }
void HttpRequest::setQuery(const std::map<std::string, std::string>& q) { query = q; }

bool HttpRequest::isChunked() const {
    std::map<std::string, std::string>::const_iterator it = headers.find("transfer-encoding");
    if (it == headers.end()) return false;
    return toLower(it->second).find("chunked") != std::string::npos;
}

size_t HttpRequest::contentLength() const {
    std::map<std::string, std::string>::const_iterator it = headers.find("content-length");
    if (it == headers.end()) return 0;
    return safeAtoi(it->second);
}

void HttpRequest::parseQuery(const std::string& target, std::string& cleanPath, std::map<std::string, std::string>& outQuery) {
    size_t qpos = target.find('?');
    if (qpos == std::string::npos) {
        cleanPath = target;
        return;
    }
    cleanPath = target.substr(0, qpos);
    std::string qstr = target.substr(qpos + 1);
    size_t start = 0;
    while (start < qstr.size()) {
        size_t eq = qstr.find('=', start);
        size_t amp = qstr.find('&', start);
        if (eq == std::string::npos) {
            if (amp == std::string::npos) {
                std::string key = urlDecode(qstr.substr(start));
                if (!key.empty()) outQuery[key] = "";
                break;
            } else {
                std::string key = urlDecode(qstr.substr(start, amp - start));
                if (!key.empty()) outQuery[key] = "";
                start = amp + 1;
            }
        } else {
            if (amp == std::string::npos || amp > eq) {
                std::string key = urlDecode(qstr.substr(start, eq - start));
                size_t vend = (amp == std::string::npos) ? qstr.size() : amp;
                std::string val = urlDecode(qstr.substr(eq + 1, vend - eq - 1));
                outQuery[key] = val;
                if (amp == std::string::npos) break;
                start = amp + 1;
            } else {
                std::string key = urlDecode(qstr.substr(start, amp - start));
                if (!key.empty()) outQuery[key] = "";
                start = amp + 1;
            }
        }
    }
}

bool HttpRequest::parseHeaderLine(const std::string& line, std::string& k, std::string& v) {
    size_t colon = line.find(':');
    if (colon == std::string::npos) return false;
    k = toLowerStr(trim(line.substr(0, colon)));
    v = trim(line.substr(colon + 1));
    return true;
}

bool HttpRequest::validate(std::string& err) const {
    (void)err;
    return true;
}

// Subclass implementations
GetRequest::GetRequest() { method = "GET"; }
bool GetRequest::validate(std::string& err) const {
    if (!body.empty()) {
        err = "GET request should not have a body";
        return false;
    }
    return true;
}
void GetRequest::handle(HttpResponse& res) { (void)res; }

HeadRequest::HeadRequest() { method = "HEAD"; }
bool HeadRequest::validate(std::string& err) const {
    if (!body.empty()) {
        err = "HEAD request should not have a body";
        return false;
    }
    return true;
}
void HeadRequest::handle(HttpResponse& res) { (void)res; }

PostRequest::PostRequest() { method = "POST"; }
bool PostRequest::validate(std::string& err) const { (void)err; return true; }
void PostRequest::handle(HttpResponse& res) { (void)res; }

PutRequest::PutRequest() { method = "PUT"; }
bool PutRequest::validate(std::string& err) const { (void)err; return true; }
void PutRequest::handle(HttpResponse& res) { (void)res; }

PatchRequest::PatchRequest() { method = "PATCH"; }
bool PatchRequest::validate(std::string& err) const { (void)err; return true; }
void PatchRequest::handle(HttpResponse& res) { (void)res; }

DeleteRequest::DeleteRequest() { method = "DELETE"; }
bool DeleteRequest::validate(std::string& err) const { (void)err; return true; }
void DeleteRequest::handle(HttpResponse& res) { (void)res; }

OptionsRequest::OptionsRequest() { method = "OPTIONS"; }
bool OptionsRequest::validate(std::string& err) const { (void)err; return true; }
void OptionsRequest::handle(HttpResponse& res) { (void)res; }

// Factory function
HttpRequest* makeRequestByMethod(const std::string& m) {
    std::string ml = toLowerStr(m);
    if (ml == "get") return new GetRequest();
    if (ml == "head") return new HeadRequest();
    if (ml == "post") return new PostRequest();
    if (ml == "put") return new PutRequest();
    if (ml == "patch") return new PatchRequest();
    if (ml == "delete") return new DeleteRequest();
    if (ml == "options") return new OptionsRequest();
    return 0;
}
