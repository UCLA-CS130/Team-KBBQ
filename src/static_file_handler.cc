#include "static_file_handler.h"

// TODO: Implement
RequestHandler::Status StaticFileHandler::Init(const std::string& uri_prefix, const NginxConfig& config) {
    return RequestHandler::Status::OK;
}

// TODO: Implement
RequestHandler::Status StaticFileHandler::HandleRequest(const Request& request, Response* response) {
    return RequestHandler::Status::OK;
}