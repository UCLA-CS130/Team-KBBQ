#ifndef NOT_FOUND_HANDLER_H
#define NOT_FOUND_HANDLER_H

#include "request_handler.h"

class NotFoundHandler : public RequestHandler {
 public:
    virtual NotFoundHandler::Status Init(const std::string& uri_prefix, const NginxConfig& config);

    virtual NotFoundHandler::Status HandleRequest(const Request& request, Response* response);
};

REGISTER_REQUEST_HANDLER(NotFoundHandler);

#endif //NOT_FOUND_HANDLER