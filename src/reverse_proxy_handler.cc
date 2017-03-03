#include "reverse_proxy_handler.h"
#include "config_parser.h"
#include <iostream>
#include <boost/asio.hpp>


RequestHandler::Status ReverseProxyHandler::Init(const std::string& uri_prefix, const NginxConfig& config) {
    prefix_ = uri_prefix;
    std::string port_str = "";
    host_ = "";
    urlpath_ = "";

    std::string fullHost = "";
    for (auto statement : config.statements_){
      if (statement->tokens_.size() > 1 && statement->tokens_[0] == "port"){
	port_str = statement->tokens_[1];
	port_ = stoi(port_str);
      }
      else if (statement->tokens_.size() > 1 && statement->tokens_[0] == "host"){
        fullHost = statement->tokens_[1];
      }
    }
    
    if (fullHost == "")
      return RequestHandler::Status::INVALID_CONFIG;

    //Just a host is provided in the config
    if (fullHost.find("/") == std::string::npos){
      host_ = fullHost;
      urlpath_ = "/";
    }
    else {
      std::size_t hostEnd_pos = fullHost.find("/");
      host_ = fullHost.substr(0, hostEnd_pos);
      urlpath_ = fullHost.substr(hostEnd_pos, fullHost.size() - host_.size());
    }
    if (urlpath_.back() != '/')
      urlpath_ += "/";

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
      
      Response::ResponseCode forwardRC = ForwardRequest(transformedRequest, response, nextHost);
      if (forwardRC != Response::ResponseCode::OK){
        std::cerr << "Forwarding request to host failed with code : " << forwardRC << std::endl;
        return RequestHandler::Status::PROXY_ERROR;
      }

      Response::ResponseCode rc = response->status_code();
      
      std::cout << "~~~~Reverse Proxy Response~~~~" << std::endl;
      response->PrintHeaders();

      //If not redirected, then break and return response
      if (rc != Response::ResponseCode::FOUND && rc != Response::ResponseCode::MOVED_PERMANENTLY){
        break;
      }

      std::string redirectLocation = response->GetHeader("Location");
      if (redirectLocation == ""){
        std::cerr << "Received redirect with no Location Header" << std::endl;
        return RequestHandler::Status::PROXY_ERROR; 
      }

      std::string nextURI = "";

      ParseLocation(redirectLocation, nextHost, nextURI);

      std::pair<std::string, std::string> nextHostPair("Host", nextHost);
      transformedRequest.update_header(nextHostPair);
      transformedRequest.update_uri(nextURI); 
     
      std::cout << "Updated Request:" << std::endl << transformedRequest.raw_request() << std::endl;
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

  //This jumble of code is ensuring that the updated uri
  //is formed as a concatenation of the path in the config
  //and of the url path provided

  //This logic is needed as a path of / ends with / whereas
  //every other path (like /proxy) doesn't
  std::string updatedURI = urlpath_;
  if (incoming_request.uri().size() > prefix_.size()){
    std::string uriShortened = incoming_request.uri().substr(prefix_.size());
    
    if (urlpath_.back() != uriShortened.front()){
      updatedURI += incoming_request.uri().substr(prefix_.size());
    }
    else{
      updatedURI += incoming_request.uri().substr(prefix_.size() + 1);
    }
  }

  transformedRequest->update_uri(updatedURI);
 
  std::cout << "UPDATED URI:" <<  updatedURI << std::endl;
  return (*transformedRequest);
}


void ReverseProxyHandler::ParseLocation(const std::string location, std::string& host, std::string& uri){
  std::size_t http_pos = location.find("//", 0);
  std::size_t host_pos = location.find("/", http_pos + 2);

  if (host_pos == std::string::npos){
    uri = "/";
    host = location.substr(http_pos + 2);
  }
  else{
    uri = location.substr(host_pos);
    host = location.substr(http_pos +2, host_pos - http_pos - 2); 
  }
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

    auto parsedResponse = Response::Parse(raw_response);
    if (!parsedResponse){
      return Response::INTERNAL_SERVER_ERROR;
    }

    *response = *parsedResponse;

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
