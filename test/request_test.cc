#include "request_handler.h"
#include "gtest/gtest.h"

TEST(ParseTest, Simple){
    std::string request_string = "GET /hello.htm HTTP/1.1\r\n"
                                "User-Agent: Mozilla/4.0 (compatible; MSIE5.01; Windows NT)\r\n"
                                "Host: www.tutorialspoint.com\r\n"
                                "Accept-Language: en-us\r\n"
                                "Accept-Encoding: gzip, deflate\r\n"
                                "Connection: Keep-Alive\r\n\r\n";
    auto request = Request::Parse(request_string);
    ASSERT_TRUE(request);
    EXPECT_EQ(request_string, request->raw_request());
    EXPECT_EQ("GET", request->method());
    EXPECT_EQ("/hello.htm", request->uri());
    EXPECT_EQ("HTTP/1.1", request->version());
    EXPECT_EQ("", request->body());
}