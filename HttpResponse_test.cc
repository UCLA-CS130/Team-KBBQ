#include "gtest/gtest.h"
#include "HttpConstants.h"
#include "HttpResponse.h"
#include <iostream>

TEST(HttpResponseTest, BadRequest) {
    std::vector<char> response = HttpResponse::bad_request_response();
    std::string expected(response.begin(), response.end());
    EXPECT_EQ(expected, BAD_REQUEST_RESP);
}

TEST(HttpResponseTest, NotFound) {
    std::vector<char> response = HttpResponse::not_found_response();
    std::string expected(response.begin(), response.end());
    EXPECT_EQ(expected, NOT_FOUND_RESP);
}

TEST(HttpResponseTest, NotImplemented) {
    std::vector<char> response = HttpResponse::not_implemented_response();
    std::string expected(response.begin(), response.end());
    EXPECT_EQ(expected, NOT_IMPLEMENTED_RESP);
}

// Basic Echo Response test
TEST(HttpResponseTest, EchoResponse) {
    std::vector<char> response;
    std::string expected = "HTTP/1.0 200 OK\r\nContent-type: text/plain\r\n\r\nSome request string here.";
    EchoResponse e_resp("Some request string here.");

    ASSERT_EQ(e_resp.generate_response(response), OK);
    std::string response_str(response.begin(), response.end());
    EXPECT_EQ(expected, response_str);
}

// Basic File Response test
TEST(HttpResponseTest, FileResponseSlash) {
    std::vector<char> response;
    std::string expected = "HTTP/1.0 200 OK\r\nContent-type: text/plain\r\nContent-length: 4\r\n\r\n";
    FileResponse f_resp("example_files/", "asdf.txt");

    ASSERT_EQ(f_resp.generate_response(response), OK);
    std::string response_str(response.begin(), response.end());
    EXPECT_EQ(expected, response_str);
}

// Basic File Response test, no slash at end of directory
TEST(HttpResponseTest, FileResponseNoSlash) {
    std::vector<char> response;
    std::string expected = "HTTP/1.0 200 OK\r\nContent-type: text/plain\r\nContent-length: 4\r\n\r\n";
    FileResponse f_resp("example_files", "asdf.txt");

    ASSERT_EQ(f_resp.generate_response(response), OK);
    std::string response_str(response.begin(), response.end());
    EXPECT_EQ(expected, response_str);
}

// Basic File Response test, no file
TEST(HttpResponseTest, MissingFile) {
    std::vector<char> response;
    FileResponse f_resp("example_files", "unknown.txt");

    ASSERT_EQ(f_resp.generate_response(response), NOT_FOUND);
    std::string response_str(response.begin(), response.end());
    EXPECT_EQ(NOT_FOUND_RESP, response_str);
}

TEST(HttpResponseTest, FileExtensionTest) {
    FileResponse f_resp("test", "test");
    EXPECT_EQ(TYPE_JPEG, f_resp.get_content_type("test.jpg"));
    EXPECT_EQ(TYPE_JPEG, f_resp.get_content_type("test.jpeg"));
    EXPECT_EQ(TYPE_GIF, f_resp.get_content_type("test.gif"));
    EXPECT_EQ(TYPE_HTML, f_resp.get_content_type("test.html"));
    EXPECT_EQ(TYPE_HTML, f_resp.get_content_type("test.htm"));
    EXPECT_EQ(TYPE_OCT, f_resp.get_content_type("test"));
    EXPECT_EQ(TYPE_PDF, f_resp.get_content_type("test.pdf"));
    EXPECT_EQ(TYPE_PNG, f_resp.get_content_type("test.png"));
    EXPECT_EQ(TYPE_TXT, f_resp.get_content_type("test.txt"));
    EXPECT_EQ(TYPE_TXT, f_resp.get_content_type("test.cc"));
}
