#include <cstdlib>
#include <iostream>
#include <istream>
#include <ostream>
#include <string>
#include <iterator>
#include <boost/asio.hpp>
#include "Webserver.h"
#include "gtest/gtest.h"

using boost::asio::ip::tcp;
    
TEST(WebserverTest, run_serverTest) {
    //Webserver server;
    //server.parse_config("config");  
    
    boost::asio::io_service io_service;  
    //server.run_server(io_service);

    tcp::resolver resolver (io_service);
    tcp::resolver::query query("localhost", "8080");
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
    boost::system::error_code error;
    std::string answer;

    while (boost::asio::read(socket, response, boost::asio::transfer_at_least(1), error)) {
        std::istream response_stream(&response);
        std::string s(std::istreambuf_iterator<char>(response_stream), {});
        answer = s;
    }

    if (error != boost::asio::error::eof) {
        throw boost::system::system_error(error);
    }

    EXPECT_EQ("HTTP/1.0 200 OK\r\nContent-type: text/plain\r\n\r\nasdfasdf", answer);
}
