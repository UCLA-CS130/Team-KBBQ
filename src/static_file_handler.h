#ifndef STATIC_FILE_HANDLER_H
#define STATIC_FILE_HANDLER_H

#include "request_handler.h"

class StaticFileHandler : public RequestHandler {
 public:
    virtual RequestHandler::Status Init(const std::string& uri_prefix,
                        const NginxConfig& config);

    virtual RequestHandler::Status HandleRequest(const Request& request,
                                 Response* response);
};

REGISTER_REQUEST_HANDLER(StaticFileHandler);

#endif  // STATIC_FILE_HANDLER_H