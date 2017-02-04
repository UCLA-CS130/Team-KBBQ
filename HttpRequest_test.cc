#include <boost/asio.hpp>
#include "gtest/gtest.h"
#include "HttpRequest.h"

TEST(CreateRequestTest, Simple){
    HttpRequest req;
    boost::asio::streambuf b;
    std::iostream os(&b);

    os << "GET /static/img/file.txt/ HTTP/1.0\r\n";
    req.createRequest(b);

    std::string result = req.ToString();

    EXPECT_EQ("GET /static/img/file.txt/ HTTP/1.0\r\n", result);
}

TEST(InvalidCreateRequestTest, Simple){
    HttpRequest req;
    boost::asio::streambuf b;
    std::iostream os(&b);

    os << "POST /static/img/file.txt/ HTTP/1.0\r\n";
    
    EXPECT_EQ(-1, req.createRequest(b);
}

