#ifndef HTTPRESPONSE_HPP
#define HTTPRESPONSE_HPP

#include <map>
#include <string>
#include <vector>

// Forward declaration
class HttpRequest;
class RequestContext;

class HttpResponse {
 private:
  int statusCode;
  std::map<std::string, std::string> headers;
  std::vector<std::string> setCookieHeaders;
  std::string body;
  std::string version;
  std::string statusMessage;

 public:
  HttpResponse();
  ~HttpResponse();

  // Main function
  void setStatus(int code, const std::string& reason);
  void setHeader(const std::string& key, const std::string& value);
  void setBody(const std::string& b);
  void setVersion(const std::string& v);
  void addSetCookieHeader(const std::string& value);
  std::string getHostHeader() const;
  std::vector<std::string> getSetCookieHeaders() const;

  std::string build() const;

  // Error handling methods
  void setError(int code, const std::string& reason);
  void setErrorFromContext(int code, const RequestContext& ctx);

  // Redirect handling
  void setRedirect(int code, const std::string& location);
};

#endif
