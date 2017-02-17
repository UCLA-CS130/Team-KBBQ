#include "echo_handler.h"

// TODO: Implement
RequestHandler::Status EchoHandler::Init(const std::string& uri_prefix, const NginxConfig& config) {
    return RequestHandler::Status::OK;
}

// TODO: Implement
RequestHandler::Status EchoHandler::HandleRequest(const Request& request, Response* response) {
    return RequestHandler::Status::OK;
}