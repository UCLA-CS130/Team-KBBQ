#include "server_status_tracker.h"

ServerStatusTracker& ServerStatusTracker::GetInstance() {
    static ServerStatusTracker instance;
    return instance;
}

void ServerStatusTracker::RecordRequest(const std::string& url, const Response::ResponseCode& response) {
    std::pair<std::string, Response::ResponseCode> request(url, response);
    url_requests_.push_back(request);
}

void ServerStatusTracker::RecordHandlerMapping(const std::string& prefix, const std::string& handler_name) {
    std::pair<std::string, std::string> handler(prefix, handler_name);
    handlers_.push_back(handler);
}

ServerStatusTracker::RequestList ServerStatusTracker::GetRequests() const {
    return url_requests_;
}

ServerStatusTracker::HandlerList ServerStatusTracker::GetHandlers() const {
    return handlers_;
}

int ServerStatusTracker::GetNumRequests() {
    return url_requests_.size();
}