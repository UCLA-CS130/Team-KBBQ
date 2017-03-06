#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "reverse_proxy_handler.h"
#include "request_handler.h"
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
class ReverseProxyHandlerTests : public ::testing::Test {
protected:
    bool ParseString(const std::string config_string) {
        std::stringstream config_stream(config_string);
        return parser_.Parse(&config_stream, &out_config_);
    }
    NginxConfigParser parser_;
    NginxConfig out_config_;
    MockRequest processed_request;
};

// TransformRequest Test
TEST(ReverseProxyHandlerTests, TransformRequestTest) {
    ReverseProxyHandler rp_handler;
    NginxConfig config;
    rp_handler.Init("/", config);
    std::string request = "GET /echo HTTP/1.1\r\nUser-Agent: curl/7.35.0\r\nHost: localhost:8080\r\nConnection: open\r\nAccept: */*\r\n\r\n\r\n";
  
    auto req = Request::Parse(request);
    Request transformedReq;
    transformedReq = rp_handler.TransformRequest(*req);
    std::string expectedRequest = "GET echo HTTP/1.1\r\nUser-Agent: curl/7.35.0\r\nHost: \r\nConnection: close\r\nAccept: */*\r\n\r\n\r\r\n";
    ASSERT_EQ(transformedReq.raw_request(), expectedRequest);
}

// ParseLocation Test
TEST(ReverseProxyHandlerTests, ParseLocationTest) {
   ReverseProxyHandler rp_handler;
   std::string host, uri;
   std::string toParse = "/www.ucla.edu/dir1/dir2";
   rp_handler.ParseLocation(toParse,host,uri);
   
   ASSERT_EQ(host,"www.ucla.edu");
   ASSERT_EQ(uri,"/dir1/dir2");    
}
