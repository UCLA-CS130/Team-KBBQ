#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "static_file_handler.h"
#include <iostream>
#include <fstream>
#include <stdio.h>

using ::testing::Return;

class MockRequest : public Request {
public:
    MOCK_CONST_METHOD0(raw_request, std::string());
    MOCK_CONST_METHOD0(uri, std::string());
};

// Test fixture
class StaticFileHandlerTests : public ::testing::Test {
protected:
    bool ParseString(const std::string config_string) {
        std::stringstream config_stream(config_string);
        return parser_.Parse(&config_stream, &out_config_);
    }
    NginxConfigParser parser_;
    NginxConfig out_config_;
    MockRequest processed_request;
};

// Get file test
TEST(StaticFileHandlerHelperTests, GetFile) {
    // Create a test file
    std::ofstream test_file("test_file.txt");
    test_file << "this is a test" << std::endl;
    test_file.close();

    StaticFileHandler f_handler;
    std::string contents;
    Response::ResponseCode status = f_handler.get_file("test_file.txt", &contents);

    ASSERT_EQ(Response::ResponseCode::OK, status);
    EXPECT_EQ("this is a test\n", contents);

    remove("test_file.txt");
}

// Check correct file extentions from get_content_type()
TEST(StaticFileHandlerHelperTests, FileExtensionTest) {
    StaticFileHandler f_handler;
    EXPECT_EQ(TYPE_JPEG, f_handler.get_content_type("test.jpg"));
    EXPECT_EQ(TYPE_JPEG, f_handler.get_content_type("test.jpeg"));
    EXPECT_EQ(TYPE_GIF, f_handler.get_content_type("test.gif"));
    EXPECT_EQ(TYPE_HTML, f_handler.get_content_type("test.html"));
    EXPECT_EQ(TYPE_HTML, f_handler.get_content_type("test.htm"));
    EXPECT_EQ(TYPE_OCT, f_handler.get_content_type("test"));
    EXPECT_EQ(TYPE_OCT, f_handler.get_content_type("test.exe"));
    EXPECT_EQ(TYPE_PDF, f_handler.get_content_type("test.pdf"));
    EXPECT_EQ(TYPE_PNG, f_handler.get_content_type("test.png"));
    EXPECT_EQ(TYPE_TXT, f_handler.get_content_type("test.txt"));
}

// Basic valid Init Call
TEST_F(StaticFileHandlerTests, BasicInitTest) {
    ASSERT_TRUE(ParseString("root /foo/bar;"));

    StaticFileHandler f_handler;
    RequestHandler::Status status = f_handler.Init("/static", out_config_);

    ASSERT_EQ(RequestHandler::Status::OK, status);
}

// Missing root
TEST_F(StaticFileHandlerTests, MissingRoot) {
    ASSERT_TRUE(ParseString("foo bar;"));

    StaticFileHandler f_handler;
    RequestHandler::Status status = f_handler.Init("/static", out_config_);

    ASSERT_EQ(RequestHandler::Status::INVALID_CONFIG, status);
}

// Duplicate root
TEST_F(StaticFileHandlerTests, DuplicateRoot) {
    ASSERT_TRUE(ParseString("root /foo/bar;\nroot /bar;"));

    StaticFileHandler f_handler;
    RequestHandler::Status status = f_handler.Init("/static", out_config_);

    ASSERT_EQ(RequestHandler::Status::INVALID_CONFIG, status);
}

// Trailing slash in URI
TEST_F(StaticFileHandlerTests, TrailingSlash) {
    EXPECT_CALL(processed_request, uri()).Times(1).WillOnce(Return("/static/test_file.txt"));
    ASSERT_TRUE(ParseString("root ./;"));

    // Create a test file
    std::ofstream test_file("test_file.txt");
    test_file << "foo bar" << std::endl;
    test_file.close();

    // Initialize handler for uri prefix "/static"
    StaticFileHandler f_handler;
    Response resp;
    ASSERT_EQ(RequestHandler::Status::OK, f_handler.Init("/static", out_config_));

    ASSERT_EQ(RequestHandler::Status::OK, f_handler.HandleRequest(processed_request, &resp));
    EXPECT_EQ("HTTP/1.0 200 OK\r\nContent-Type: text/plain\r\nContent-Length: 8\r\n\r\nfoo bar\n", resp.ToString());
    remove("test_file.txt");
}

// Empty file name
TEST_F(StaticFileHandlerTests, EmptyFileName) {
    EXPECT_CALL(processed_request, uri()).Times(1).WillOnce(Return("/static/"));
    ASSERT_TRUE(ParseString("root ./;"));

    // Initialize handler for uri prefix "/static"
    StaticFileHandler f_handler;
    Response resp;
    ASSERT_EQ(RequestHandler::Status::OK, f_handler.Init("/static", out_config_));

    ASSERT_EQ(RequestHandler::Status::FILE_NOT_FOUND, f_handler.HandleRequest(processed_request, &resp));
}

TEST_F(StaticFileHandlerTests, HandleBasicRequest) {
    EXPECT_CALL(processed_request, uri()).Times(1).WillOnce(Return("/static/test_file.txt"));

    ASSERT_TRUE(ParseString("root ./;"));

    // Create a test file
    std::ofstream test_file("test_file.txt");
    test_file << "foo bar" << std::endl;
    test_file.close();

    // Initialize handler for uri prefix "/static"
    StaticFileHandler f_handler;
    Response resp;
    ASSERT_EQ(RequestHandler::Status::OK, f_handler.Init("/static", out_config_));

    ASSERT_EQ(RequestHandler::Status::OK, f_handler.HandleRequest(processed_request, &resp));
    EXPECT_EQ("HTTP/1.0 200 OK\r\nContent-Type: text/plain\r\nContent-Length: 8\r\n\r\nfoo bar\n", resp.ToString());
    remove("test_file.txt");
}
