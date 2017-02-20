#ifndef ECHO_HANDLER_H
#define ECHO_HANDLER_H

#include "request_handler.h"

class EchoHandler : public RequestHandler {
 public:
    virtual RequestHandler::Status Init(const std::string& uri_prefix, const NginxConfig& config);

    virtual RequestHandler::Status HandleRequest(const Request& request, Response* response);
};

REGISTER_REQUEST_HANDLER(EchoHandler);

#endif  // ECHO_HANDLER_H