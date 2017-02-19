#include "request_handler.h"
#include "gtest/gtest.h"

TEST(ParseTest, ValidRequest){
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
    ASSERT_EQ(5, request->headers().size());
    EXPECT_EQ("User-Agent", request->headers()[0].first);
    EXPECT_EQ("Host", request->headers()[1].first);
    EXPECT_EQ("Accept-Language", request->headers()[2].first);
    EXPECT_EQ("Accept-Encoding", request->headers()[3].first);
    EXPECT_EQ("Connection", request->headers()[4].first);
    EXPECT_EQ("Mozilla/4.0 (compatible; MSIE5.01; Windows NT)", request->headers()[0].second);
    EXPECT_EQ("www.tutorialspoint.com", request->headers()[1].second);
    EXPECT_EQ("en-us", request->headers()[2].second);
    EXPECT_EQ("gzip, deflate", request->headers()[3].second);
    EXPECT_EQ("Keep-Alive", request->headers()[4].second);
}

TEST(ParseTest, InvalidRequest){
    std::string request_string = "GET HTTP/1.1\r\n\r\n";
    auto request = Request::Parse(request_string);
    ASSERT_FALSE(request);
}
