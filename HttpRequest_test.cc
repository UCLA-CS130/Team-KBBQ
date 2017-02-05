#include <boost/asio.hpp>
#include "gtest/gtest.h"
#include "HttpRequest.h"

class CreateRequestTest : public ::testing::Test {
protected:
    HttpRequest req;
    boost::asio::streambuf b;   
};

TEST_F(CreateRequestTest, SimpleRequest){
    std::iostream os(&b);
    os << "GET /static/img/file.txt/ HTTP/1.0\r\n";
    req.createRequest(b);

    std::string result = req.ToString();

    EXPECT_EQ("GET /static/img/file.txt/ HTTP/1.0\r\n", result);
}

TEST_F(CreateRequestTest, InvalidRequest){
    std::iostream os(&b);
    os << "POST /static/img/file.txt/ HTTP/1.0\r\n";
    
    EXPECT_EQ(-1, req.createRequest(b));
}

TEST_F(CreateRequestTest, EmptyRequest){
    std::iostream os(&b);
    os << "GET / HTTP/1.0\r\n";
    
    EXPECT_EQ(-1, req.createRequest(b));
}

