#ifndef STATUS_HANDLER_H
#define STATUS_HANDLER_H

#include "request_handler.h"
#include "server_status_tracker.h"

class StatusHandler : public RequestHandler {
 public:
    virtual RequestHandler::Status Init(const std::string& uri_prefix, const NginxConfig& config);

    virtual RequestHandler::Status HandleRequest(const Request& request, Response* response);

};

REGISTER_REQUEST_HANDLER(StatusHandler);

#endif  // STATUS_HANDLER_H