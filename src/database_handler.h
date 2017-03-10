#ifndef DATABASE_HANDLER_H
#define DATABASE_HANDLER_H

#include "request_handler.h"
#include "mysql_connection.h"
#include "mysql_driver.h"
#include <cppconn/driver.h>

const std::string BASE_PAGE = 
    "<!doctype html>"
    "<html>"
        "<body>"
            "<h1>Database Interface</h1>"
            "<form>"
                "SQL Command:<br>"
                "<input type='text' name='sql'>"
                "<br><br>"
                "<input type='submit' value='Submit'>"
            "</form>"
        "</body"
    "</html>";

class DatabaseHandler : public RequestHandler {
 public:
    virtual RequestHandler::Status Init(const std::string& uri_prefix, const NginxConfig& config);

    virtual RequestHandler::Status HandleRequest(const Request& request, Response* response);

 private:
    std::string username_;
    std::string password_;
    sql::mysql::MySQL_Driver *driver_;
};

REGISTER_REQUEST_HANDLER(DatabaseHandler);

#endif  // DATABASE_HANDLER_H