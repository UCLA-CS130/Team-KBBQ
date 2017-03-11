#include "database_handler.h"

#include "mysql_connection.h"
#include "mysql_driver.h"

#include <cppconn/driver.h>
#include <cppconn/exception.h>
#include <cppconn/metadata.h>
#include <cppconn/resultset.h>
#include <cppconn/statement.h>

#include <sstream>
#include <vector>

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
    connection->setSchema("CS130");
    if (!connection) {
        // Connection Failed
        ErrorResponse(FAILED_CONNECTION, response);
        return RequestHandler::Status::DATABASE_ERROR;
    } else {
        // URI = /database?query=select+name+from+actors
        std::string uri = request.uri();
        std::string param = "query=";
        std::size_t query_start = uri.find(param);
        if (query_start == std::string::npos) {
            // No query parameter
            ErrorResponse(PARAM_ERROR, response);
            return RequestHandler::Status::DATABASE_ERROR;
        }
        std::string query = URLDecode(uri.substr(uri.find(param) + param.length()));

        try {
            // TODO: Validate query?
            sql::Statement *stmt = connection->createStatement();
            sql::ResultSet *results = stmt->executeQuery(query);
            sql::ResultSetMetaData *col_meta = results->getMetaData();

            // Build JSON Response
            std::string result_string = "{\n";

            // Column names
            if (results->rowsCount() > 0) {
                result_string += "\"cols\": [";
                for (size_t i = 1; i <= col_meta->getColumnCount(); i++) {
                    std::string col_name = col_meta->getColumnName(i);
                    result_string += "\"" + col_name + "\"";
                    if (i != col_meta->getColumnCount()) {
                        result_string += ",";
                    }
                }
                result_string += "],\n\"data\": {\n";
            }

            while (results->next()) {
                int row_id = results->getRow();
                result_string += "\"" + std::to_string(row_id) + "\": [";
                // Columns are 1-indexed..
                for (size_t i = 1; i <= col_meta->getColumnCount(); i++) {
                    std::string value = results->getString(i);
                    result_string += "\"" + value + "\"";
                    if (i != col_meta->getColumnCount()) {
                        result_string += ",";
                    }
                }
                result_string += "],\n";
            }
            if (results->rowsCount() > 0) {
                result_string = result_string.substr(0, result_string.length()-2); // Remove trailing comma
            }
            result_string += "}\n}";

            std::cout << "Database results: " << std::endl << result_string << std::endl;

            // Send response
            response->SetStatus(Response::ResponseCode::OK);
            response->AddHeader("Content-Type", "text/plain");
            response->AddHeader("Content-Length", std::to_string(result_string.length()));
            response->SetBody(result_string);

            // Free resultSet, Statement, and query string pointers
            delete results;
            delete stmt;
        } catch (sql::SQLException &e) {
            /*
              MySQL Connector/C++ throws three different exceptions:

              - sql::MethodNotImplementedException (derived from sql::SQLException)
              - sql::InvalidArgumentException (derived from sql::SQLException)
              - sql::SQLException (derived from std::runtime_error)
            */
            std::stringstream SQL_err_output;
            /* what() (derived from std::runtime_error) fetches error message */
            SQL_err_output << "Error: " << e.what();
            SQL_err_output << " (MySQL error code: " << e.getErrorCode();
            SQL_err_output << ", SQLState: " << e.getSQLState() << " )" << std::endl;

            ErrorResponse(SQL_err_output.str(), response);
            return RequestHandler::Status::DATABASE_ERROR;
        }

        // Free connection pointer
        delete connection;
    }

    return RequestHandler::Status::OK;
}

void DatabaseHandler::ErrorResponse(std::string err_msg, Response* response) {
    response->SetStatus(Response::ResponseCode::OK);
    response->AddHeader("Content-Type", "text/plain");
    response->AddHeader("Content-Length", std::to_string(err_msg.length()));
    response->SetBody(err_msg);
    std::cerr << err_msg << std::endl;
}


// Adapted from DLib: http://dlib.net/dlib/server/server_http.cpp.html
unsigned char DatabaseHandler::FromHex(unsigned char ch) {
    if (ch <= '9' && ch >= '0')
        ch -= '0';
    else if (ch <= 'f' && ch >= 'a')
        ch -= 'a' - 10;
    else if (ch <= 'F' && ch >= 'A')
        ch -= 'A' - 10;
    else
        ch = 0;
    return ch;
}

// Adapted from DLib: http://dlib.net/dlib/server/server_http.cpp.html
const std::string DatabaseHandler::URLDecode(const std::string& str) {
    std::string result;
    std::string::size_type i;
    for (i = 0; i < str.size(); ++i) {
        if (str[i] == '+') {
            result += ' ';
        }
        else if (str[i] == '%' && str.size() > i+2) {
            const unsigned char ch1 = FromHex(str[i+1]);
            const unsigned char ch2 = FromHex(str[i+2]);
            const unsigned char ch = (ch1 << 4) | ch2;
            result += ch;
            i += 2;
        }
        else {
            result += str[i];
        }
    }
    return result;
}