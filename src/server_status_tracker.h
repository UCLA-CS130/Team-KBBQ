#ifndef SERVER_STATUS_TRACKER_H
#define SERVER_STATUS_TRACKER_H

#include "request_handler.h"

// Singleton class to keep track of server request information
class ServerStatusTracker
{
    public:
        static ServerStatusTracker& GetInstance();

        void RecordRequest(const std::string& url, const Response::ResponseCode& response);
        void RecordHandlerMapping(const std::string& prefix, const std::string& handler_name);
        int GetNumRequests();

        using RequestList = std::vector<std::pair<std::string, Response::ResponseCode>>;
        RequestList GetRequests() const;

        using HandlerList = std::vector<std::pair<std::string, std::string>>;
        HandlerList GetHandlers() const;

        // Delete copy and assignment functions
        ServerStatusTracker(ServerStatusTracker const&) = delete;
        void operator=(ServerStatusTracker const&) = delete;

    private:
        ServerStatusTracker() {} // Hide the constructor

        std::vector<std::pair<std::string, Response::ResponseCode>> url_requests_;
        std::vector<std::pair<std::string, std::string>> handlers_;
};

#endif // SERVER_STATUS_TRACKER_H