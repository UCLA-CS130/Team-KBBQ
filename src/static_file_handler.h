#ifndef STATIC_FILE_HANDLER_H
#define STATIC_FILE_HANDLER_H

#include "request_handler.h"
#include <time.h>
#include <unordered_map>

// FILE TYPES
const std::string TYPE_JPEG  = "image/jpeg";
const std::string TYPE_GIF   = "image/gif";
const std::string TYPE_HTML  = "text/html";
const std::string TYPE_OCT   = "application/octet-stream";
const std::string TYPE_PDF   = "application/pdf";
const std::string TYPE_PNG   = "image/png";
const std::string TYPE_TXT   = "text/plain";

class StaticFileHandler : public RequestHandler {
 public:
    virtual RequestHandler::Status Init(const std::string& uri_prefix, const NginxConfig& config);

    virtual RequestHandler::Status HandleRequest(const Request& request, Response* response);

    std::string get_content_type(const std::string &filename);
    Response::ResponseCode get_file(const std::string& file_path, std::string* contents);
    std::string gen_cookie(std::string::size_type length);
    std::string add_cookie(std::string old_cookie);
    bool check_cookie(std::string cookie, Response* response);

 private:
    std::string prefix;
    std::string root;
    std::string username;
    std::string password;
    time_t timeout;
    std::string original_uri;
    std::unordered_map<std::string, time_t> cookie_map;
};

REGISTER_REQUEST_HANDLER(StaticFileHandler);

#endif  // STATIC_FILE_HANDLER_H