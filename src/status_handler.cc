#include "status_handler.h"
#include <sstream>

RequestHandler::Status StatusHandler::Init(const std::string& uri_prefix, const NginxConfig& config) {
    // Ignore any configs
    return RequestHandler::Status::OK;
}

RequestHandler::Status StatusHandler::HandleRequest(const Request& request, Response* response) {
    int num_requests = ServerStatusTracker::GetInstance().GetNumRequests();
    ServerStatusTracker::RequestList url_requests = ServerStatusTracker::GetInstance().GetRequests();
    ServerStatusTracker::HandlerList handlers = ServerStatusTracker::GetInstance().GetHandlers();

    std::ostringstream contents; 
    contents << "<!DOCTYPE html><html>" <<
                "<h1>Server Status</h1>" <<
                "<h2>Requests</h2>" <<
                "<p>Total Requests: " << num_requests << "</p>" <<
                "<table>" <<
                "<th><td>Request URL</td><td>Response Code</td></th>";

    if (num_requests > 0) {
        for (auto req : url_requests) {  
            contents << "<tr>" <<
                        "<td>" << req.first << "</td>" <<
                        "<td>" << req.second << "</td>" <<
                        "</tr>";
        }
    } else {
        contents << "<tr><td colspan='2'>N/A</td><tr>";
    }

    contents << "</table>" <<
                "<h2>Request Handlers</h2>" <<
                "<table>" <<
                "<th><td>URL Prefix</td><td>Response Handler</td></th>";

    for (auto handler : handlers) {  
        contents << "<tr>" <<
                    "<td>" << handler.first << "</td>" <<
                    "<td>" << handler.second << "</td>" <<
                    "</tr>";
    }

    contents << "</table>" <<
                "</html>";

    std::string status_page = contents.str();

    response->SetStatus(Response::ResponseCode::OK);
    response->AddHeader("Content-Type", "text/html");
    response->AddHeader("Content-Length", std::to_string(status_page.length()));
    response->SetBody(status_page);

    return RequestHandler::Status::OK;
}