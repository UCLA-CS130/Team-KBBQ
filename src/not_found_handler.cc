#include "not_found_handler.h"

RequestHandler::Status NotFoundHandler::Init(const std::string& uri_prefix, const NginxConfig& config) {
    // There should be nothing in the config block. Ignore any configs
    return RequestHandler::Status::FILE_NOT_FOUND;
}

RequestHandler::Status NotFoundHandler::HandleRequest(const Request& request, Response* response) {
    std::string not_found_body = "<html><body><h1>404 Not Found</h1></body></html>";
    response->SetStatus(Response::ResponseCode::NOT_FOUND);
    response->AddHeader("Content-Type", "text/plain");
    response->AddHeader("Content-Length", std::to_string(not_found_body.length()));
    response->SetBody(not_found_body);
    return RequestHandler::Status::FILE_NOT_FOUND;
}