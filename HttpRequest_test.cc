#include <boost/asio.hpp>
#include "gtest/gtest.h"
#include "HttpRequest.h"
#include "HttpConstants.h"

class CreateRequestTest : public ::testing::Test {
protected:
    HttpRequest req;
    boost::asio::streambuf b;

    void write_request(std::string request){
        std::iostream os(&b);
        os << request;
    }
};

TEST_F(CreateRequestTest, SimpleRequest){
    write_request("GET /static/img/file.txt/ HTTP/1.0\r\n");
    int status = req.create_request(b);
    ASSERT_EQ(status, 0);
    EXPECT_EQ("GET /static/img/file.txt/ HTTP/1.0\r\n", req.to_string());
    EXPECT_EQ("static", req.get_type());
    EXPECT_EQ("img/file.txt", req.get_file());
}

TEST_F(CreateRequestTest, InvalidRequest){
    write_request("POST /static/img/file.txt/ HTTP/1.0\r\n");
    EXPECT_EQ(NOT_IMPLEMENTED, req.create_request(b));
}

TEST_F(CreateRequestTest, EmptyRequest){
    write_request("GET / HTTP/1.0\r\n");
    EXPECT_EQ(BAD_REQUEST, req.create_request(b));
}

TEST_F(CreateRequestTest, NoFile){
    write_request("GET /static HTTP/1.0\r\n");
    int status = req.create_request(b);
    ASSERT_EQ(status, 0);
    EXPECT_EQ("GET /static HTTP/1.0\r\n", req.to_string());
    EXPECT_EQ("static", req.get_type());
    EXPECT_EQ("", req.get_file());
}
