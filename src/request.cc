#include "request_handler.h"
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/classification.hpp>
#include <sstream>
#include <vector>

std::unique_ptr<Request> Request::Parse(const std::string& raw_request){
    
    std::unique_ptr<Request> request(new Request());

    request->body_ = "";
    request->raw_request_ = raw_request;

    std::istringstream request_stream(raw_request);
    std::string request_line;

    //handle first line
    if (std::getline(request_stream, request_line)) {
        std::vector<std::string> tokens;
        boost::algorithm::split(tokens, request_line, boost::algorithm::is_any_of(" \r"));

        if (!tokens.empty() && tokens[0] == "GET") {
            request->method_ = tokens[0];
            request->uri_ = tokens[1];
            request->version_ = tokens[2];
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
                header_value = header_value.erase(header_value.length()-1, 1);
            }
            
            std::cout << "header field: " << header_field << ", header value: " << header_value << std::endl;
            request->headers_.push_back( std::make_pair( header_field, header_value));
        }
        else {  
            std::cout << "transitioning to body: " << request_line << std::endl;
            
            while (std::getline(request_stream, request_line)){
                std::cout << "Putting into body: " << request_line << std::endl;
                request->body_ += request_line;
            }
        }
    }

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