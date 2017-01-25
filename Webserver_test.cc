#include <cstdlib>
#include <iostream>
#include <boost/asio.hpp>
#include "gtest/gtest.h"
#include "webserver.cc"

TEST(WebserverTest, run_serverTest) {
	  boost::asio::io_service io_service;
	  Webserver server;
	  server.run_server(io_service, 8080);
	  tcp::resolver resolver (io_service);
	  tcp::resolver::query query(localhost:8080, "asdf");
    tcp::resolver::iterator endpoint_iterator = resolver.resolve(query);

    tcp::socket socket(io_service);
    boost::asio::connect(socket, endpoint_iterator);

    boost::asio::streambuf request;
    std::ostream request_stream(&request);
    request_stream << "asdfasdf";

    // Send the request.
    boost::asio::write(socket, request);

    // Read the response.
    boost::asio::streambuf response;
    boost::asio::read(socket, response);

    std::istream response_stream(&response);
    std::string response;
    response_stream >> response;

    EXPECT_EQ("HTTP/1.0 200 OK\r\nContent-type: text/plain\r\n\r\nasdfasdf", response);
}