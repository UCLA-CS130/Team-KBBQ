#include "static_file_handler.h"
#include <algorithm>
#include <fstream>
#include <sstream>
#include <time.h>

// Get the content type from the file name.
std::string StaticFileHandler::get_content_type(const std::string& filename_str) {
    // Find last period.
    size_t pos = filename_str.find_last_of('.');
    std::string type;

    // No file extension.
    if (pos == std::string::npos || (pos+1) >= filename_str.size()) {
        return TYPE_OCT;
    }

    // Type = everything after last '.'
    type = filename_str.substr(pos+1);

    // Change to all lowercase.
    std::transform(type.begin(), type.end(), type.begin(), ::tolower);

    // Get file type.
    if (type == "jpeg" || type == "jpg") {
        return TYPE_JPEG;
    } else if (type == "gif") {
        return TYPE_GIF;
    } else if (type == "html" || type=="htm") {
        return TYPE_HTML;
    } else if (type == "pdf") {
        return TYPE_PDF;
    } else if (type == "png") {
        return TYPE_PNG;
    } else if (type == "txt") {
        return TYPE_TXT;
    } else { // Default to type octet-stream.
        return TYPE_OCT;
    }
}

RequestHandler::Status StaticFileHandler::Init(const std::string& uri_prefix, const NginxConfig& config) {
    prefix = uri_prefix;
    root = "";
    username = "";
    password = "";
    timeout = -1;
    original_request = "";
    original_uri = "";
    cookie = {0, -1};

    // Iterate through the config block to find the root mapping.
    for (size_t i = 0; i < config.statements_.size(); i++) {
        std::shared_ptr<NginxConfigStatement> stmt = config.statements_[i];
        // Ignore all configs that are not setting root.
        if (stmt->tokens_.size() != 2) {
            continue;
        } else if (stmt->tokens_[0] == "root") {
            // Set the root value.
            if (root.empty()) {
                root = stmt->tokens_[1];
                if (root.back() == '/') {
                    // Remove trailing slash from root
                    root = root.substr(0, root.length()-1);
                }
            } else {
                // Error: The root value has already been set.
                std::cerr << "Error: Multiple root mappings specified for " << uri_prefix <<".\n";
                return RequestHandler::Status::INVALID_CONFIG;
            }
        } else if (stmt->tokens_[0] == "username") {
            // Set the username value.
            username = stmt->tokens_[1];
        } else if (stmt->tokens_[0] == "password") {
            // Set the password value.
            password = stmt->tokens_[1];
        } else if (stmt->tokens_[0] == "timeout") {
            // Set the time value.
            bool is_number = (stmt->tokens_[1].find_first_not_of("1234567890") == std::string::npos);
            if (is_number) {
                timeout = std::stoi(stmt->tokens_[1]);
            } else {
                // Error: The timeout is not a number.
                std::cerr << "Error: Timeout is not a number.\n";
                return RequestHandler::Status::INVALID_CONFIG;
            }
        }
    }

    if (root.empty()) {
        // Error: No config definition for the root.
        std::cerr << "Error: No root mapping specified for " << uri_prefix <<".\n";
        return RequestHandler::Status::INVALID_CONFIG;
    }

    if ((username.empty() || password.empty() || timeout < 1) && !(username.empty() && password.empty() && timeout < 1)) {
        // Error: Either all empty or all initialized.
        std::cerr << "Error: The three variables need to be all empty or all initialized.\n";
        std::cerr << uri_prefix;
        return RequestHandler::Status::INVALID_CONFIG;
    }

    return RequestHandler::Status::OK;
}

RequestHandler::Status StaticFileHandler::HandleRequest(const Request& request, Response* response) {
    std::string file_path = "";
    std::string contents = "";
    std::string login = "/private/login.html";
    bool redirect = false;

    if (request.raw_request().find(login) != std::string::npos) {
        redirect = true;
    }

    // If serving regular static files or the request is about login.html, skip
    if (!username.empty() && !redirect) {
        time_t seconds;
        seconds = time(NULL);
        // MAYBE ADD MORE CHECKING FOR COOKIE OR SOME SHIT
        if (cookie.key == 0 || ((seconds - cookie.time) > timeout)) {
            // If no cookie or expired, redirect to login
            response->SetStatus(Response::ResponseCode::FOUND);
            response->AddHeader("Location", "/private/login.html");
            response->AddHeader("Content-Type", "text/html");
            response->AddHeader("Content-Length", "228");
            get_file("private_files/login.html", &contents);
            response->SetBody(contents);
            original_request = request.raw_request();
            original_uri = request.uri();
            return RequestHandler::Status::OK;
        }
    }

    if (request.method() == "POST") {
        // Extract username and password
        std::string body = request.body();
        size_t first = body.find("=");
        size_t second = body.find("&");
        std::string user = body.substr(first + 1, second - first - 1);
        size_t third = body.find("=", second);
        std::string pass = body.substr(third + 1);

        if (user == username && pass == password) {
            // Create cookie 
            unsigned long random = rand() % 100000000 + 100000000;
            cookie.key = random;
            time_t seconds;
            seconds = time(NULL);
            cookie.time = seconds;

            // Redirect to the original url
            response->AddHeader("Location", original_uri);
            std::unique_ptr<Request> new_request = Request::Parse(original_request);
            original_request = "";
            original_uri = "";
            return HandleRequest(*new_request, response);
        }
    }

    // Get URI.
    std::string filename = request.uri();

    // Check the URI prefix.
    if (filename.find(prefix) != 0) {
        // Something is wrong, the prefix does not match the URI.
        response = nullptr;
        std::cout << "StaticFileHandler: prefix does not match uri" << std::endl;
        return RequestHandler::Status::INVALID_URI;
    } else {
        // Remove the prefix from the URI to get the file name.
        filename.erase(0, prefix.length());
    }

    // If no file, do not try to open directory.
    if (filename.empty() || filename == "/") {
        response = nullptr;
        std::cout << "StaticFileHandler: Empty file name" << std::endl;
        return RequestHandler::Status::FILE_NOT_FOUND;
    }

    // Open file
    file_path = root + filename;
    std::cout << "StaticFileHandler: Handling request for " + file_path << std::endl;
    Response::ResponseCode response_code = get_file(file_path, &contents);

    if (response_code != Response::ResponseCode::OK) {
        response = nullptr;
        std::cout << "StaticFileHandler: File not found: " + file_path << std::endl;
        return RequestHandler::Status::FILE_NOT_FOUND;
    }

    // Create response headers
    // If There is a redirect, set the response code
    if (response->GetHeader("Location") == "") {
        response->SetStatus(response_code);
    } else {
        response->SetStatus(Response::ResponseCode::FOUND);
    }
    response->AddHeader("Content-Type", get_content_type(filename));
    response->AddHeader("Content-Length", std::to_string(contents.length()));

    // Set response body
    response->SetBody(contents);

    return RequestHandler::Status::OK;
}

Response::ResponseCode StaticFileHandler::get_file(const std::string& file_path, std::string* contents) {
    // Attempt to open file
    std::ifstream in_stream(file_path.c_str(), std::ios::in | std::ios::binary);

    // File doesn't exist
    if (!in_stream) {
        contents = nullptr;
        return Response::ResponseCode::NOT_FOUND;
    }

    // Read file contents
    std::stringstream sstr;
    sstr << in_stream.rdbuf();
    contents->assign(sstr.str());

    return Response::ResponseCode::OK;
}
