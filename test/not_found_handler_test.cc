#include "gtest/gtest.h"
#include "not_found_handler.h"
#include "request_handler.h"

// Basic not found response
TEST(NotFoundHandlerTest, Simple) {
    Request processed_request;

    NotFoundHandler nf_handler;
    Response resp;
    NginxConfig config;
    std::string not_found_body = "HTTP/1.0 404 Not Found\r\n"
                                "Content-Type: text/plain\r\n"
                                "Content-Length: 48\r\n\r\n"
                                "<html><body><h1>404 Not Found</h1></body></html>";

    ASSERT_EQ(RequestHandler::Status::OK, nf_handler.Init("", config));
    nf_handler.HandleRequest(processed_request, &resp);
    EXPECT_EQ(not_found_body, resp.ToString());
}