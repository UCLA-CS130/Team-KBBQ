#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "static_file_handler.h"
#include <iostream>
#include <fstream>
#include <stdio.h>
#include <vector>
#include <unistd.h>

using ::testing::Return;

class MockRequest : public Request {
public:
    MOCK_CONST_METHOD0(raw_request, std::string());
    MOCK_CONST_METHOD0(uri, std::string());
    MOCK_CONST_METHOD0(method, std::string());
    MOCK_CONST_METHOD0(body, std::string());
    MOCK_CONST_METHOD0(cookie, std::string());
};

// Test fixture
class StaticFileHandlerTests : public ::testing::Test {
protected:
    bool ParseString(const std::string config_string) {
        std::stringstream config_stream(config_string);
        return parser_.Parse(&config_stream, &out_config_);
    }

    void CreateLoginFile() {
        std::ofstream login_file("login.html");
        std::string log = "<form action=\"\" method=\"post\">"
        "Username: <input type=\"text\" name=\"username\"><br>"
        "Password: <input type=\"text\" name=\"password\"/>"
        "<input type=\"submit\" value=\"Submit\"/></form>";
        login_file << log << std::endl;
        login_file.close();
    }

    bool CreateTestFile() {
        std::ofstream test_file("test_file.txt");
        test_file << "foo bar" << std::endl;
        test_file.close();
    }

    NginxConfigParser parser_;
    NginxConfig out_config_;
    MockRequest processed_request;
    MockRequest new_request;
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

// Valid Init for private files
TEST_F(StaticFileHandlerTests, PrivateInit) {
    ASSERT_TRUE(ParseString("root /foo/bar;\nuser root password;\ntimeout 10;"));

    StaticFileHandler f_handler;
    RequestHandler::Status status = f_handler.Init("/private", out_config_);

    ASSERT_EQ(RequestHandler::Status::OK, status);
}

// Missing Mapping
TEST_F(StaticFileHandlerTests, MissingMapping) {
    ASSERT_TRUE(ParseString("root /foo/bar;\ntimeout 10;"));

    StaticFileHandler f_handler;
    RequestHandler::Status status = f_handler.Init("/private", out_config_);

    ASSERT_EQ(RequestHandler::Status::INVALID_CONFIG, status);
}

// Duplicate Mapping
TEST_F(StaticFileHandlerTests, DuplicateMapping) {
    ASSERT_TRUE(ParseString("root /foo/bar;\nuser root password;\nuser root pass;\ntimeout 10;"));

    StaticFileHandler f_handler;
    RequestHandler::Status status = f_handler.Init("/private", out_config_);

    ASSERT_EQ(RequestHandler::Status::INVALID_CONFIG, status);
}

// Trailing slash in URI
TEST_F(StaticFileHandlerTests, TrailingSlash) {
    EXPECT_CALL(processed_request, uri()).Times(1).WillOnce(Return("/static/test_file.txt"));
    ASSERT_TRUE(ParseString("root ./;"));

    // Create a test file
    CreateTestFile();

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
    CreateTestFile();

    // Initialize handler for uri prefix "/static"
    StaticFileHandler f_handler;
    Response resp;
    ASSERT_EQ(RequestHandler::Status::OK, f_handler.Init("/static", out_config_));

    ASSERT_EQ(RequestHandler::Status::OK, f_handler.HandleRequest(processed_request, &resp));
    EXPECT_EQ("HTTP/1.0 200 OK\r\nContent-Type: text/plain\r\nContent-Length: 8\r\n\r\nfoo bar\n", resp.ToString());
    remove("test_file.txt");
}

// Check that each cookie is unique
TEST(StaticFileHandlerHelperTests, GenerateCookieTest) {
    StaticFileHandler f_handler;
    std::vector<std::string> cookie_list;
    bool all_unique = true;

    for (int i = 0; i < 500; ++i) {
        std::string cookie = f_handler.gen_cookie(20);
        for (auto cook : cookie_list) {
            if (cook == cookie) {
                all_unique = false;
            }
        }

        if (!all_unique) {
            break;
        }

        cookie_list.push_back(cookie);
    }

    EXPECT_EQ(true, all_unique);
}

// Valid add cookie
TEST(StaticFileHandlerHelperTests, AddCookie) {
    StaticFileHandler f_handler;
    std::string new_cookie = f_handler.add_cookie("");

    // Expect that the cookie is okay
    EXPECT_NE("", new_cookie);
}

// New cookie created when duplicate
TEST(StaticFileHandlerHelperTests, DuplicateCookie) {
    StaticFileHandler f_handler;
    std::string new_cookie = f_handler.add_cookie("");
    std::string old_cookie = f_handler.add_cookie(new_cookie);

    // Expect that the cookie is okay
    EXPECT_NE(new_cookie, old_cookie);
}

// No cookie leads to redirect
TEST_F(StaticFileHandlerTests, NewCookie) {
    ASSERT_TRUE(ParseString("root /foo/bar;\nuser root password;\ntimeout 10;"));

    StaticFileHandler f_handler;
    RequestHandler::Status status = f_handler.Init("/private", out_config_);

    ASSERT_EQ(RequestHandler::Status::OK, status);

    Response resp;
    bool cookie_ok = f_handler.check_cookie("", &resp);

    // Expect that the cookie is not okay and resp is set to redirect
    EXPECT_EQ(false, cookie_ok);
    EXPECT_EQ(Response::ResponseCode::FOUND, resp.status_code());
}

// Cookie is valid and no redirect
TEST_F(StaticFileHandlerTests, ValidCookie) {
    ASSERT_TRUE(ParseString("root /foo/bar;\nuser root password;\ntimeout 10;"));

    StaticFileHandler f_handler;
    RequestHandler::Status status = f_handler.Init("/private", out_config_);

    ASSERT_EQ(RequestHandler::Status::OK, status);

    Response resp;

    std::string new_cookie = f_handler.add_cookie("");
    ASSERT_NE("", new_cookie);

    bool cookie_ok = f_handler.check_cookie(new_cookie, &resp);

    // Expect that the cookie is okay and resp is not set
    EXPECT_EQ(true, cookie_ok);
    EXPECT_NE(Response::ResponseCode::FOUND, resp.status_code());
}

// Expired cookie leads to redirect
TEST_F(StaticFileHandlerTests, InvalidCookie) {
    ASSERT_TRUE(ParseString("root /foo/bar;\nuser root password;\ntimeout 1;"));

    StaticFileHandler f_handler;
    RequestHandler::Status status = f_handler.Init("/private", out_config_);

    ASSERT_EQ(RequestHandler::Status::OK, status);

    Response resp;

    std::string new_cookie = f_handler.add_cookie("");
    ASSERT_NE("", new_cookie);

    // Make sure Cookie is expired
    sleep(2);

    bool cookie_ok = f_handler.check_cookie(new_cookie, &resp);

    // Expect that the cookie is okay and resp is not set
    EXPECT_EQ(false, cookie_ok);
    EXPECT_EQ(Response::ResponseCode::FOUND, resp.status_code());
}

// If no cookie redirect to login.html
TEST_F(StaticFileHandlerTests, HandleRedirect) {
    EXPECT_CALL(processed_request, uri()).Times(1).WillOnce(Return("/private/test_file.txt"));

    ASSERT_TRUE(ParseString("root ./; user root password; timeout 10;"));

    // Create a login file
    CreateLoginFile();

    // Initialize handler for uri prefix "/private"
    StaticFileHandler f_handler;
    Response resp;
    ASSERT_EQ(RequestHandler::Status::OK, f_handler.Init("/private", out_config_));

    // Handle the mock request
    ASSERT_EQ(RequestHandler::Status::OK, f_handler.HandleRequest(processed_request, &resp));

    EXPECT_EQ(Response::ResponseCode::FOUND, resp.status_code());
    remove("login.html");
}

// Create a new cookie by logging in and then access a private file
TEST_F(StaticFileHandlerTests, HandleLogin) {
    // Part 1: Set the cookie by entering the correct username and password
    EXPECT_CALL(processed_request, uri()).Times(1).WillOnce(Return("/private/login.html"));
    EXPECT_CALL(processed_request, method()).Times(1).WillOnce(Return("POST"));
    EXPECT_CALL(processed_request, body()).Times(1).WillOnce(Return("username=root&password=password"));
    EXPECT_CALL(processed_request, cookie()).Times(1).WillOnce(Return(""));

    ASSERT_TRUE(ParseString("root ./; user root password; timeout 1000;"));

    // Create a login file
    CreateLoginFile();

    // Create a test file
    CreateTestFile();

    // Initialize handler for uri prefix "/private"
    StaticFileHandler f_handler;
    Response old_resp;
    ASSERT_EQ(RequestHandler::Status::OK, f_handler.Init("/private", out_config_));

    // Handle the first mock request
    ASSERT_EQ(RequestHandler::Status::OK, f_handler.HandleRequest(processed_request, &old_resp));

    // Get the cookie from the response
    std::string cookie_header = "Set-Cookie: private=";
    std::string raw_response = old_resp.ToString();
    size_t first = raw_response.find(cookie_header);
    first = first + cookie_header.length();
    size_t second = raw_response.find("\r", first);
    std::string cookie = raw_response.substr(first, second - first);
    ASSERT_EQ(20, cookie.length());

    // Part 2: Get contents of test_file.txt
    EXPECT_CALL(new_request, uri()).Times(1).WillOnce(Return("/private/test_file.txt"));
    EXPECT_CALL(new_request, method()).Times(1).WillOnce(Return("GET"));
    EXPECT_CALL(new_request, cookie()).Times(1).WillOnce(Return(cookie));

    // Handle the second mock request
    Response new_resp;
    ASSERT_EQ(RequestHandler::Status::OK, f_handler.HandleRequest(new_request, &new_resp));
    EXPECT_EQ("HTTP/1.0 200 OK\r\nContent-Type: text/plain\r\nContent-Length: 8\r\n\r\nfoo bar\n", new_resp.ToString());
    remove("login.html");
    remove("test_file.txt");
}

// Wrong password means no cookie set
TEST_F(StaticFileHandlerTests, HandleBadPassword) {
    // Part 1: Set the cookie by entering the correct username and password
    EXPECT_CALL(processed_request, uri()).Times(1).WillOnce(Return("/private/login.html"));
    EXPECT_CALL(processed_request, method()).Times(1).WillOnce(Return("POST"));
    EXPECT_CALL(processed_request, body()).Times(1).WillOnce(Return("username=root&password=pass"));

    ASSERT_TRUE(ParseString("root ./; user root password; timeout 1000;"));

    // Create a login file
    CreateLoginFile();

    // Initialize handler for uri prefix "/private"
    StaticFileHandler f_handler;
    Response resp;
    ASSERT_EQ(RequestHandler::Status::OK, f_handler.Init("/private", out_config_));

    // Handle the first mock request
    ASSERT_EQ(RequestHandler::Status::OK, f_handler.HandleRequest(processed_request, &resp));

    // Get the cookie from the response
    std::string cookie_header = "Set-Cookie: private=";
    EXPECT_EQ(std::string::npos, resp.ToString().find(cookie_header));
    remove("login.html");
}