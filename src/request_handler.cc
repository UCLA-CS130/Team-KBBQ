#include "request_handler.h"

/*
 * RESPONSE
 */
void Response::SetStatus(const ResponseCode response_code) {
    switch (response_code) {
        case ResponseCode::OK:
            status = "200 OK";
            break;
        case ResponseCode::BAD_REQUEST:
            status = "400 Bad Request";
            break;
        case ResponseCode::NOT_FOUND:
            status = "404 Not Found";
            break;
        case ResponseCode::NOT_IMPLEMENTED:
            status = "501 Not Implemented";
            break;
    }
}

void Response::AddHeader(const std::string& header_name, const std::string& header_value) {
    // Ignore invalid headers
    if (header_name.empty() || header_value.empty()) {
        return;
    }

    std::pair<std::string, std::string> header(header_name, header_value);
    headers.push_back(header);
}

void Response::SetBody(const std::string& body) {
    response_body = body;
}

std::string Response::ToString() {
    // TODO: What if the status / body are not set?

    // Response code
    std::string response = "HTTP/1.0 " + status + "\r\n";

    // Headers
    for (auto &header_pair : headers) {
        response += header_pair.first + ": " + header_pair.second + "\r\n";
    }

    // End of all headers
    response += "\r\n";

    // Body
    response += response_body;

    return response;
}


/*
 * REQUEST HANDLER (BASE)
 */
std::map<std::string, RequestHandler* (*)(void)>* request_handler_builders = nullptr;

// Creates a request handler based on given type. If type doesn't exist, returns null pointer
RequestHandler* RequestHandler::CreateByName(const char* type) {
    const auto type_and_builder = request_handler_builders->find(type);
    if (type_and_builder == request_handler_builders->end()) {
        return nullptr;
    }
    return (*type_and_builder->second)();
}