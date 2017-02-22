#include "gtest/gtest.h"
#include "server_status_tracker.h"

TEST(ServerStatusTest, RecordRequests) {
    ServerStatusTracker::GetInstance().RecordRequest("/status", Response::ResponseCode::OK);
    ServerStatusTracker::GetInstance().RecordRequest("/static/asdf.txt", Response::ResponseCode::OK);
    ServerStatusTracker::GetInstance().RecordRequest("/echo/something.bar", Response::ResponseCode::NOT_FOUND);

    ASSERT_EQ(3, ServerStatusTracker::GetInstance().GetNumRequests());

    ServerStatusTracker::RequestList requests = ServerStatusTracker::GetInstance().GetRequests();
    EXPECT_EQ("/status", requests[0].first);
    EXPECT_EQ(Response::ResponseCode::OK, requests[0].second);

    EXPECT_EQ("/static/asdf.txt", requests[1].first);
    EXPECT_EQ(Response::ResponseCode::OK, requests[1].second);

    EXPECT_EQ("/echo/something.bar", requests[2].first);
    EXPECT_EQ(Response::ResponseCode::NOT_FOUND, requests[2].second);
}

TEST(ServerStatusTest, RecordHandlers) {
    ServerStatusTracker::GetInstance().RecordHandlerMapping("/status", "StatusHandler");
    ServerStatusTracker::GetInstance().RecordHandlerMapping("/static/asdf.txt", "StaticFileHandler");
    ServerStatusTracker::GetInstance().RecordHandlerMapping("/echo", "EchoHandler");

    ServerStatusTracker::HandlerList handlers = ServerStatusTracker::GetInstance().GetHandlers();

    ASSERT_EQ(3, handlers.size());
    EXPECT_EQ("/status", handlers[0].first);
    EXPECT_EQ("StatusHandler", handlers[0].second);

    EXPECT_EQ("/static/asdf.txt", handlers[1].first);
    EXPECT_EQ("StaticFileHandler", handlers[1].second);

    EXPECT_EQ("/echo", handlers[2].first);
    EXPECT_EQ("EchoHandler", handlers[2].second);
}