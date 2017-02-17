#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "echo_handler.h"

using ::testing::Return;

class MockRequest : public Request {
public:
    MOCK_CONST_METHOD0(raw_request, std::string());
};

// Basic echo request
TEST(EchoHandlerTest, BasicEcho) {
    MockRequest processed_request;

    EXPECT_CALL(processed_request, raw_request()).Times(1).WillOnce(Return("foo bar"));

    EchoHandler e_handler;
    Response resp;
    e_handler.HandleRequest(processed_request, &resp);

    EXPECT_EQ("HTTP/1.0 200 OK\r\nContent-Type: text/plain\r\nContent-Length: 7\r\n\r\nfoo bar", resp.ToString());
}