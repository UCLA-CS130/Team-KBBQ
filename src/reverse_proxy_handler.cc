#include "reverse_proxy_handler.h"
#include "config_parser.h"
#include <iostream>
#include <boost/asio.hpp>


RequestHandler::Status ReverseProxyHandler::Init(const std::string& uri_prefix, const NginxConfig& config) {
    prefix_ = uri_prefix;
    std::string port_str = "";
    host_ = "";

    for (auto statement : config.statements_){
      if (statement->tokens_.size() > 1 && statement->tokens_[0] == "port"){
	port_str = statement->tokens_[1];
	port_ = stoi(port_str);
      }
      else if (statement->tokens_.size() > 1 && statement->tokens_[0] == "host"){
        host_ = statement->tokens_[1];
      }
    }
    
    if (host_ == "")
      return RequestHandler::Status::INVALID_CONFIG;

    return RequestHandler::Status::OK;
}

RequestHandler::Status ReverseProxyHandler::HandleRequest(const Request& request, Response* response) {
     
    const int MAX_REDIRECTS = 21;
    //Transform request to be forwarded to provided host
    //TODO: raw_request no longer matches other request member - fix that?
    auto transformedRequest = TransformRequest(request);
    
    //Use a host parameter for ForwardRequest instead of updating host_ each time
    std::string nextHost = host_;
    //Now loop for maximum redirects
    for (int i = 0; i < MAX_REDIRECTS; i++){
      std::cout << "Beginning to forward request in ReverseProxyHandler to " << host_ << std::endl;
      
      Response::ResponseCode forwardRC = ForwardRequest(transformedRequest, response, host_);

      //If not redirected, then break and return response
      if (forwardRC != Response::ResponseCode::FOUND){
        break;
      }
    }

    return RequestHandler::Status::OK;    
}

Request ReverseProxyHandler::TransformRequest(const Request& incoming_request) {
  auto transformedRequest = Request::Parse(incoming_request.raw_request());

  //Update Host with that specified in the config
  //TODO: Possibly error check host either in Init 
  std::pair<std::string, std::string> updatedHost("Host", host_);
  transformedRequest->update_header(updatedHost);
  
  //Update Connection header
  //Our implementation will want Connection: close
  std::pair<std::string, std::string> newConnection("Connection", "close");
  transformedRequest->update_header(newConnection);

  //Now update a path
  //TODO: improve this extraction
  if (incoming_request.uri() == prefix_){
    transformedRequest->update_uri("/");  
  }
  else{
    transformedRequest->update_uri(incoming_request.uri());
  }
  
  return (*transformedRequest);
}

bool ReverseProxyHandler::TranslateResponse(Response& received_response){
  return true;
}


//TODO: Refactor this huge function
Response::ResponseCode ReverseProxyHandler::ForwardRequest(const Request& request, Response* response, std::string host){

    //Connection initialization
    boost::asio::io_service io_service;
    boost::system::error_code ec;


    boost::asio::ip::tcp::resolver resolver(io_service);
    boost::asio::ip::tcp::resolver::query query(host, "http");
    boost::asio::ip::tcp::resolver::iterator it = resolver.resolve(query, ec), end;

    if (ec) {
      std::cerr << "Boost resolver could not resolve host: " << host << " and service http" << std::endl;
      return Response::ResponseCode::INTERNAL_SERVER_ERROR;
    }
    boost::asio::ip::tcp::socket socket(io_service);

    //Since this will act like a client, use connect
    boost::asio::connect(socket, it, ec);
    std::cout << "Reverse Proxy now connected to host: " << host << std::endl;
    
    if (ec){
      std::cerr << ec << " in ForwardRequest inside ReverseProxyHandler using host: " << host << std::endl;
      return Response::ResponseCode::INTERNAL_SERVER_ERROR;
    }

    
    //Since we update raw_request private member on each update
    //We can use send this string as our serialized request
    boost::asio::write(socket, boost::asio::buffer(request.raw_request(), request.raw_request().size()));
    std::string raw_response = getRawResponse(socket);


    return Response::OK;
}

std::string ReverseProxyHandler::getRawResponse(boost::asio::ip::tcp::socket& socket){
    std::string raw_response;
   
    boost::asio::streambuf response_stream;
    boost::system::error_code ec;
    std::size_t num_bytes;

    while((num_bytes = boost::asio::read(socket, response_stream, boost::asio::transfer_at_least(1), ec))){
       
      std::string data = std::string(boost::asio::buffers_begin(response_stream.data()), boost::asio::buffers_begin(response_stream.data()) + num_bytes);
     
      raw_response += data;
      response_stream.consume(num_bytes);
    }

   return raw_response;
}
