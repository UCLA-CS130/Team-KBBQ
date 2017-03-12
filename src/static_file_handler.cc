#include "static_file_handler.h"
#include <algorithm>
#include <fstream>
#include <sstream>
#include <time.h>
#include <random>
#include <unordered_map>

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
            // Set the timeout value.
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
        bool cookie_ok = check_cookie(request.cookie(), response);

        if (!cookie_ok) {
            // If cookie is not ok, need to redirect
            original_request = request.uri();
            return RequestHandler::Status::OK;
        }
    }

    if (request.method() == "POST" && redirect) {
        // The body returns this: username=USERNAME&password=PASSWORD
        // Extract username and password
        std::string body = request.body();
        size_t first = body.find("=") + 1;
        size_t second = body.find("&");
        std::string user = body.substr(first, second - first);
        size_t third = body.find("=", second);
        std::string pass = body.substr(third + 1);

        if (user == username && pass == password) {
            // Generate and then add the cookie to cookie_map
            std::string new_cookie = add_cookie(request.cookie());

            // Redirect to the original url and set the cookie
            // If the original request was login.html, don't redirect
            if (original_request !=  "") {
                response->AddHeader("Location", original_request);
            }

            response->AddHeader("Set-Cookie", "private=" + new_cookie);
        }

        original_request = "";
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

// Taken from: http://stackoverflow.com/a/24586587
// Generate a random alphanumeric string of any length
std::string StaticFileHandler::gen_cookie(std::string::size_type length)
{
    static auto& chrs = "0123456789"
        "abcdefghijklmnopqrstuvwxyz"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ";

    thread_local static std::mt19937 rg{std::random_device{}()};
    thread_local static std::uniform_int_distribution<std::string::size_type> pick(0, sizeof(chrs) - 2);

    std::string s;

    s.reserve(length);

    while(length--)
        s += chrs[pick(rg)];

    return s;
}

std::string StaticFileHandler::add_cookie(std::string old_cookie) {
    std::unordered_map<std::string, time_t>::const_iterator found = cookie_map.find(old_cookie);

    // If the cookie already exists, but the user enters username and password again, delete the old cookie
    if (found != cookie_map.end()) {
        cookie_map.erase(found);
    }

    // Create a new cookie
    std::string new_cookie = gen_cookie(20);
    found = cookie_map.find(new_cookie);

    time_t now_seconds;
    now_seconds = time(NULL);

    // Add cookie to map, if duplicate created, create again and add
    if (found == cookie_map.end()) {
        cookie_map[new_cookie] = now_seconds;
    } else {
        new_cookie = gen_cookie(20);
        cookie_map[new_cookie] = now_seconds;
    }

    return new_cookie;
}

bool StaticFileHandler::check_cookie(std::string cookie, Response* response) {
    time_t cookie_time = 0;
    std::unordered_map<std::string, time_t>::const_iterator found;

    // Find cookie in the cookie map and get time
    if (cookie != "") {
        found = cookie_map.find(cookie);
        if (found != cookie_map.end()) {
            cookie_time = found->second;
        }
    }

    time_t now_seconds;
    now_seconds = time(NULL);

    if (now_seconds - cookie_time > timeout) {
        // If no cookie or expired, redirect to login and delete old cookie
        if (found != cookie_map.end()) {
            cookie_map.erase(found);
        }

        response->SetStatus(Response::ResponseCode::FOUND);
        response->AddHeader("Location", "/private/login.html");
        response->AddHeader("Set-Cookie", "private=" + cookie + "; expires=Thu, Jan 01 1970 00:00:00 UTC;");
        response->AddHeader("Content-Type", "text/html");
        response->AddHeader("Content-Length", "228");
        std::string contents = "";
        get_file("private_files/login.html", &contents);
        response->SetBody(contents);
        return false;
    }

    return true;
}
