#include "gtest/gtest.h"
#include "request_handler.h"

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