#include "database_handler.h"

#include "mysql_connection.h"
#include "mysql_driver.h"
#include <cppconn/driver.h>

RequestHandler::Status DatabaseHandler::Init(const std::string& uri_prefix, const NginxConfig& config) {
    username_ = "";
    password_ = "";

    // Iterate through the config block to find the root mapping.
    for (size_t i = 0; i < config.statements_.size(); i++) {
        std::shared_ptr<NginxConfigStatement> stmt = config.statements_[i];
        // Ignore all invalid configs.
        if (stmt->tokens_.size() != 2) {
            continue;
        } else if (stmt->tokens_[0] == "username") {
            // Set the root value.
            if (username_.empty()) {
                username_ = stmt->tokens_[1];
            } else {
                // Error: The username value has already been set.
                std::cerr << "Error: Multiple usernames specified for database.\n";
                return RequestHandler::Status::INVALID_CONFIG;
            }
        } else if (stmt->tokens_[0] == "password") {
            // Set the root value.
            if (password_.empty()) {
                password_ = stmt->tokens_[1];
            } else {
                // Error: The password value has already been set.
                std::cerr << "Error: Multiple password specified for database.\n";
                return RequestHandler::Status::INVALID_CONFIG;
            }
        } 
    }

    driver_ = sql::mysql::get_mysql_driver_instance();
    return RequestHandler::Status::OK;
}

RequestHandler::Status DatabaseHandler::HandleRequest(const Request& request, Response* response) {
    sql::Connection *connection = driver_->connect("localhost", username_, password_);

    response->SetStatus(Response::ResponseCode::OK);
    response->AddHeader("Content-Type", "text/html");
    response->AddHeader("Content-Length", std::to_string(BASE_PAGE.length()));
    response->SetBody(BASE_PAGE);

    delete connection;
    return RequestHandler::Status::OK;
}