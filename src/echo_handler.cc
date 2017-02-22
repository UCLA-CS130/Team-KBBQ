#include "echo_handler.h"

RequestHandler::Status EchoHandler::Init(const std::string& uri_prefix, const NginxConfig& config) {
    // There should be nothing in the config block. Ignore any configs
    return RequestHandler::Status::OK;
}

// TODO: Implement
RequestHandler::Status EchoHandler::HandleRequest(const Request& request, Response* response) {
    std::string raw_request = request.raw_request();
    response->SetStatus(Response::ResponseCode::OK);
    response->AddHeader("Content-Type", "text/plain");
    response->AddHeader("Content-Length", std::to_string(raw_request.length()));
    response->SetBody(raw_request);
    return RequestHandler::Status::OK;
}