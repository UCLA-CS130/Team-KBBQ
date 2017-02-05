#include <boost/asio.hpp>
#include "gtest/gtest.h"
#include "HttpRequest.h"

class CreateRequestTest : public ::testing::Test {
protected:
    HttpRequest req;
    boost::asio::streambuf b;

    void writeRequest(std::string request){
        std::iostream os(&b);
        os << request;
    }
};

TEST_F(CreateRequestTest, SimpleRequest){
    writeRequest("GET /static/img/file.txt/ HTTP/1.0\r\n");
    req.createRequest(b);
    EXPECT_EQ("GET /static/img/file.txt/ HTTP/1.0\r\n", req.ToString());
    EXPECT_EQ("static", req.getType());
    EXPECT_EQ("img/file.txt", req.getFile());

}

TEST_F(CreateRequestTest, InvalidRequest){
    writeRequest("POST /static/img/file.txt/ HTTP/1.0\r\n");
    EXPECT_EQ(-1, req.createRequest(b));
}

TEST_F(CreateRequestTest, EmptyRequest){
    writeRequest("GET / HTTP/1.0\r\n");
    EXPECT_EQ(-1, req.createRequest(b));
}
