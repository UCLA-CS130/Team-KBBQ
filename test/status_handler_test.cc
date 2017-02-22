#include "gtest/gtest.h"
#include "status_handler.h"
#include "server_status_tracker.h"

// Init should always return OK
TEST(StatusHandlerTest, Init) {
    StatusHandler s_handler;
    NginxConfig config;
    ASSERT_EQ(RequestHandler::Status::OK, s_handler.Init("/echo", config));
}

// Check Output
TEST(StatusHandlerTest, Output) {
    StatusHandler s_handler;
    Request req;
    Response resp;

    std::string expected_contents =
        std::string("HTTP/1.0 200 OK\r\n") +
        "Content-Type: text/html\r\n" +
        "Content-Length: 425\r\n\r\n" +
        "<!DOCTYPE html><html>" +
        "<head>" +
        "<style>table {border-collapse: collapse;}th, td {padding: 8px;text-align: left; border-bottom: 1px solid #ddd;}</style>" +
        "</head>" +
        "<body>" +
        "<h1>Server Status</h1>" +
        "<h2>Requests</h2>" +
        "<p>Total Requests: 2</p>" +
        "<table>" +
        "<th><td>Request URL</td><td>Response Code</td></th>" +
        "<tr>" +
        "<td>/echo</td>" +
        "<td>200</td>" +
        "</tr>" +
        "<tr>" +
        "<td>/static/file.txt</td>" +
        "<td>404</td>" +
        "</tr>" +
        "</table>" +
        "<h2>Request Handlers</h2>" +
        "<table>" +
        "<th><td>URL Prefix</td><td>Response Handler</td></th>" +
        "<tr>" +
        "<td>/echo</td>" +
        "<td>EchoHandler</td>" +
        "</tr>" +
        "<tr>" +
        "<td>/static</td>" +
        "<td>StaticFileHandler</td>" +
        "</tr>" + 
        "</table></body></html>";

    ServerStatusTracker::GetInstance().RecordHandlerMapping("/echo", "EchoHandler");
    ServerStatusTracker::GetInstance().RecordHandlerMapping("/static", "StaticFileHandler");
    ServerStatusTracker::GetInstance().RecordRequest("/echo", Response::ResponseCode::OK);
    ServerStatusTracker::GetInstance().RecordRequest("/static/file.txt", Response::ResponseCode::NOT_FOUND);

    ASSERT_EQ(RequestHandler::Status::OK, s_handler.HandleRequest(req, &resp));
    EXPECT_EQ(expected_contents, resp.ToString());
}
