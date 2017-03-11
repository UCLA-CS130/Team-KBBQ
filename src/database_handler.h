#ifndef DATABASE_HANDLER_H
#define DATABASE_HANDLER_H

#include "request_handler.h"
#include "mysql_connection.h"
#include "mysql_driver.h"
#include <cppconn/driver.h>

const std::string FAILED_CONNECTION = "Error: Could not connect to MySQL database.";
const std::string PARAM_ERROR = "Error: Could not find query parameter.";
const std::string INTERNAL_ERROR = "Error: Internal Server Error.";

class DatabaseHandler : public RequestHandler {
 public:
    virtual RequestHandler::Status Init(const std::string& uri_prefix, const NginxConfig& config);

    virtual RequestHandler::Status HandleRequest(const Request& request, Response* response);

    void ErrorResponse(std::string err_msg, Response* response);

 private:
    unsigned char FromHex(unsigned char ch);
    const std::string URLDecode(const std::string& str);

    std::string username_;
    std::string password_;
    sql::mysql::MySQL_Driver *driver_;
};

REGISTER_REQUEST_HANDLER(DatabaseHandler);

#endif  // DATABASE_HANDLER_H