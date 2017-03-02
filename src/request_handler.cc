#include "request_handler.h"
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/classification.hpp>
#include <sstream>
#include <vector>

/*
 * REQUEST
 */
std::unique_ptr<Request> Request::Parse(const std::string& raw_request){

    std::unique_ptr<Request> request(new Request());

    request->body_ = "";
    request->method_ = "";
    request->uri_ = "";
    request->version_ = "";
    request->raw_request_ = raw_request;

    std::istringstream request_stream(raw_request);
    std::string request_line;

    //handle first line
    if (std::getline(request_stream, request_line)) {
        std::vector<std::string> tokens;
        boost::algorithm::split(tokens, request_line, boost::algorithm::is_any_of(" "));

        if (!tokens.empty() && tokens.size() == 3) {
            request->method_ = tokens[0];
            request->uri_ = tokens[1];
            request->version_ = tokens[2].substr(0, tokens[2].length()-1);
        }
        else {
            return nullptr;
        }
    }

    //handle the headers and body
    while (std::getline(request_stream, request_line)){
        std::vector<std::string> tokens;
        std::string header_field;
        std::string header_value;

        if (!request_line.empty() && request_line.at(0) != '\r') {
            std::size_t field_index = request_line.find_first_of(":");
           
            if (field_index != std::string::npos) {
                header_field = request_line.substr(0, field_index);
                header_value = request_line.substr(field_index+2, std::string::npos);
                //delete carriage return
                header_value = header_value.substr(0, header_value.length()-1);
            }
            
            std::cout << "Header field: " << header_field << ", Header value: " << header_value << std::endl;
            request->headers_.push_back( std::make_pair( header_field, header_value));
        }
        else {
            while (std::getline(request_stream, request_line)){
                request->body_ += request_line;
            }
        }
    }
    std::cout << std::endl;

    return request;
}

std::string Request::raw_request() const {
    return raw_request_;
}

std::string Request::method() const {
    return method_;
}

std::string Request::uri() const {
    return uri_;
}

std::string Request::version() const {
    return version_;
}

Request::Headers Request::headers() const {
    return headers_;
}

std::string Request::body() const {
    return body_;
}

void Request::update_header(std::pair<std::string, std::string> header){
    bool updated = false;

    for (auto it = headers_.begin(); it != headers_.end(); it++){
    if (it->first == header.first){
      it->second = header.second;
      updated = true;
      break;
    }
  }
  if (!updated){
    headers_.push_back(header);
  }
  update_raw_request();
}

void Request::update_uri(std::string newUri){
  uri_ = newUri;
  update_raw_request();
}

void Request::update_raw_request(){
  std::string new_raw;
  new_raw = method_ + " " + uri_ + " " + version() + "\r\n";
  
  for (auto header : headers_){
    new_raw += header.first + ": " + header.second + "\r\n";
  }

  new_raw += "\r\n";
  new_raw += body_;
  new_raw += "\r\n";
  
  raw_request_ = new_raw; 
}
/*
 * RESPONSE
 */
void Response::SetStatus(const ResponseCode response_code) {
    status_code_ = response_code;
    switch (response_code) {
        case ResponseCode::OK:
            status_ = "200 OK";
            break;
        case ResponseCode::FOUND:
            status_ = "302 Found";
            break;
        case ResponseCode::BAD_REQUEST:
            status_ = "400 Bad Request";
            break;
        case ResponseCode::NOT_FOUND:
            status_ = "404 Not Found";
            break;
        case ResponseCode::INTERNAL_SERVER_ERROR:
            status_ = "500 Internal Server Error";
            break;
        case ResponseCode::NOT_IMPLEMENTED:
            status_ = "501 Not Implemented";
            break;
    }
}

void Response::AddHeader(const std::string& header_name, const std::string& header_value) {
    // Ignore invalid headers
    if (header_name.empty() || header_value.empty()) {
        return;
    }

    std::pair<std::string, std::string> header(header_name, header_value);
    headers_.push_back(header);
}

void Response::SetBody(const std::string& body) {
    response_body_ = body;
}

std::string Response::ToString() {
    // TODO: What if the status / body are not set?

    // Response code
    std::string response = "HTTP/1.0 " + status_ + "\r\n";

    // Headers
    for (auto &header_pair : headers_) {
        response += header_pair.first + ": " + header_pair.second + "\r\n";
    }

    // End of all headers
    response += "\r\n";

    // Body
    response += response_body_;

    return response;
}

Response::ResponseCode Response::status_code() {
    return status_code_;
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
