#include "gtest/gtest.h"
#include "request_handler.h"

TEST(RequestTest, ValidRequest){
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

TEST(RequestTest, InvalidRequest){
    std::string request_string = "GET HTTP/1.1\r\n\r\n";
    auto request = Request::Parse(request_string);
    ASSERT_FALSE(request);
}

// Basic ToString test. Check all response codes.
TEST(ResponseTest, ToString) {
    Response resp;

    std::string remaining = 
    "Content-Type: text/plain\r\n"
        "Content-Length: 7\r\n"
        "\r\n"
        "foo bar";

    std::string expected_200 = "HTTP/1.0 200 OK\r\n" + remaining;
    std::string expected_400 = "HTTP/1.0 400 Bad Request\r\n" + remaining;
    std::string expected_404 = "HTTP/1.0 404 Not Found\r\n" + remaining;
    std::string expected_501 = "HTTP/1.0 501 Not Implemented\r\n" + remaining;

    resp.SetStatus(Response::ResponseCode::OK);
    resp.AddHeader("Content-Type", "text/plain");
    resp.AddHeader("Content-Length", "7");
    resp.SetBody("foo bar");
    EXPECT_EQ(expected_200, resp.ToString());

    resp.SetStatus(Response::ResponseCode::BAD_REQUEST);
    EXPECT_EQ(expected_400, resp.ToString());

    resp.SetStatus(Response::ResponseCode::NOT_FOUND);
    EXPECT_EQ(expected_404, resp.ToString());

    resp.SetStatus(Response::ResponseCode::NOT_IMPLEMENTED);
    EXPECT_EQ(expected_501, resp.ToString());
}

// Check that empty headers are ignored
TEST(ResponseTest, AddEmptyHeader) {
    Response resp;

    std::string expected_200 =
        "HTTP/1.0 200 OK\r\n"
        "Content-Length: 7\r\n"
        "\r\n"
        "foo bar";

    resp.SetStatus(Response::ResponseCode::OK);
    resp.AddHeader("Content-Type", ""); // This header will be ignored.
    resp.AddHeader("Content-Length", "7");
    resp.SetBody("foo bar");
    EXPECT_EQ(expected_200, resp.ToString());
}