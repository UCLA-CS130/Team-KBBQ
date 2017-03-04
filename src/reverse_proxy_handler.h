#ifndef REVERSE_PROXY_HANDLER_H
#define REVERSE_PROXY_HANDLER_H

#include "request_handler.h"
#include <boost/asio.hpp>


class ReverseProxyHandler : public RequestHandler {
 public:
    virtual RequestHandler::Status Init(const std::string& uri_prefix, const NginxConfig& config);
    virtual RequestHandler::Status HandleRequest(const Request& request, Response* response);
    std::string getRawResponse(boost::asio::ip::tcp::socket& socket);
 private:
   Request TransformRequest(const Request& incoming_request);
   Response::ResponseCode ForwardRequest(const Request& request, Response* response, std::string host);
   void ParseLocation(const std::string location, std::string& host, std::string& uri);
   
   std::string prefix_;
   std::string host_;
   std::string urlpath_;
   std::string port_;
};

REGISTER_REQUEST_HANDLER(ReverseProxyHandler);

#endif  // REVERSE_PROXY_HANDLER_H
