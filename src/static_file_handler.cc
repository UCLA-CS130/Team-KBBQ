#include "static_file_handler.h"
#include "../cpp-markdown/markdown.h"
#include <algorithm>
#include <fstream>
#include <sstream>

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
    } else if (type == "md") {
        return TYPE_MD;
    } else { // Default to type octet-stream.
        return TYPE_OCT;
    }
}

RequestHandler::Status StaticFileHandler::Init(const std::string& uri_prefix, const NginxConfig& config) {
    prefix = uri_prefix;
    root = "";

    // Iterate through the config block to find the root mapping.
    for (size_t i = 0; i < config.statements_.size(); i++) {
        std::shared_ptr<NginxConfigStatement> stmt = config.statements_[i];
        // Ignore all configs that are not seting root.
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
        }
    }

    if (root.empty()) {
        // Error: No config definition for the root.
        std::cerr << "Error: No root mapping specified for " << uri_prefix <<".\n";
        return RequestHandler::Status::INVALID_CONFIG;
    }

    return RequestHandler::Status::OK;
}

RequestHandler::Status StaticFileHandler::HandleRequest(const Request& request, Response* response) {
    std::string file_path = "";
    std::string contents = "";

    // Get URI.
    std::string filename = request.uri();

    // Check the URI prefix.
    if (filename.find(prefix) != 0) {
        // Something is wrong, the prefix does not match the URI.
        response = nullptr;
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
    std::string content_type = get_content_type(filename);
    response->SetStatus(response_code);
    //check for markdown type before setting to html
    if (content_type == "text/markdown") {
        response->AddHeader("Content-Type", "text/html");
    } else {
        response->AddHeader("Content-Type", content_type);
        response->AddHeader("Content-Length", std::to_string(contents.length()));
    }
    // Set response body
    if (content_type == "text/markdown") {
        markdown::Document doc;
        doc.read(contents);
        std::ostringstream stream;
        doc.write(stream);
        std::string markdown = stream.str();
        //add github styling to markdown
        markdown.insert(0,
            "<link rel=\"stylesheet\" href=\"markdown.css\">"
            "<style>"
                ".markdown-body {"
                   "box-sizing: border-box;"
                    "min-width: 200px;"
                    "max-width: 980px;"
                    "margin: 0 auto;"
                    "padding: 45px;"
                "}"
            "</style>"
            "<body class=\"markdown-body\">");
        markdown.append("</body>");
        //add content length after html is generated
        response->AddHeader("Content-Length", std::to_string(markdown.length()));
        response->SetBody(markdown);

    } else {
        response->SetBody(contents);
    }

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
