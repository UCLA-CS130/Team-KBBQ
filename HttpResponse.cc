#include "HttpResponse.h"
#include "HttpConstants.h"
#include <algorithm>
#include <boost/asio.hpp>

#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fstream>

/**********************************************
 * HTTP RESPONSE
 **********************************************/
// Encode 400 Bad Request into byte stream
std::vector<char> HttpResponse::bad_request_response() {
    std::vector<char> response (BAD_REQUEST_RESP.begin(), BAD_REQUEST_RESP.end());
    return response;
}

// Encode 404 Not Found into byte stream
std::vector<char> HttpResponse::not_found_response() {
    std::vector<char> response (NOT_FOUND_RESP.begin(), NOT_FOUND_RESP.end());
    return response;

}

// Encode 501 Not Implemented into byte stream
std::vector<char> HttpResponse::not_implemented_response() {
    std::vector<char> response (NOT_IMPLEMENTED_RESP.begin(), NOT_IMPLEMENTED_RESP.end());
    return response;

}

/**********************************************
 * ECHO RESPONSE
 **********************************************/
EchoResponse::EchoResponse(std::string req_string) {
    request = req_string;
}

int EchoResponse::generate_response(std::vector<char> &response) {
    std::string content_type = "Content-type: text/plain\r\n\r\n";

    // OK Response Header
    response.insert(response.end(), OK_HEADER.begin(), OK_HEADER.end());

    // Content type plain text
    response.insert(response.end(), content_type.begin(), content_type.end());

    // Echo the request
    response.insert(response.end(), request.begin(), request.end());

    return OK;
}

// Echo the request
int EchoResponse::send(boost::asio::ip::tcp::socket &sock) {
    std::vector<char> response;
    generate_response(response);

    // Send the response headers
    boost::asio::write(sock, boost::asio::buffer(response));
    std::cout << "Response: 200 OK.\n\n";
    return OK;
}

/**********************************************
 * FILE RESPONSE
 **********************************************/
FileResponse::FileResponse(std::string directory, std::string file_name) {
    // Build full file path.
    // Check if file directory already has ending slash
    if (directory.back() == '/') {
        file_path = directory + file_name;
    }
    else {
        // Add slash between directory and filename if no slash at end of directory
        file_path = directory + "/" + file_name;
    }
}

// Get the content type from the file name
std::string FileResponse::get_content_type(const std::string &filename_str) {
    // Find last period
    size_t pos = filename_str.find_last_of('.');
    std::string type;

    // No file extension
    if (pos == std::string::npos || (pos+1) >= filename_str.size()) {
        return TYPE_OCT;
    }

    // Type = everything after last '.'
    type = filename_str.substr(pos+1);

    // Change to all lowercase
    std::transform(type.begin(), type.end(), type.begin(), ::tolower);

    // Get file type
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
    } else { // default to plain text
        return TYPE_TXT;
    }
}

int FileResponse::generate_response(std::vector<char> &response) {
    // Open the file
    int filefd = open(file_path.c_str(), O_RDONLY);
    if (filefd == -1) {
        // File not found
        response = not_found_response();
        return NOT_FOUND;
    }

    // Get file size information
    struct stat fileStat;
    if (fstat(filefd, &fileStat) < 0) {
        // Could not get file size information.
        std::cerr << "Error: Could not get requested file information: " << file_path << ".\n";;

        response = not_found_response();
        return NOT_FOUND;
    }
    // Build HTTP response
    std::string content_type = "Content-type: " + get_content_type(file_path) + "\r\n";
    std::string content_length = "Content-length: " + std::to_string(fileStat.st_size) + "\r\n\r\n";

    close(filefd);

    // OK Response Header
    response.insert(response.end(), OK_HEADER.begin(), OK_HEADER.end());

    // Content type
    response.insert(response.end(), content_type.begin(), content_type.end());

    // Content type
    response.insert(response.end(), content_length.begin(), content_length.end());

    return OK;
}

int FileResponse::send(boost::asio::ip::tcp::socket &sock) {
    std::vector<char> response;
    int status = generate_response(response);

    if (status == NOT_FOUND) {
        boost::asio::write(sock, boost::asio::buffer(response));
        std::cout << "Response: 404 Not Found.\n\n";
        return NOT_FOUND;
    }

    // Send the response headers
    boost::asio::write(sock, boost::asio::buffer(response));

    // Open the file to send back.
    std::ifstream in_stream(file_path.c_str(), std::ios::in | std::ios::binary);
    if (!in_stream) {
        response = not_found_response();
        boost::asio::write(sock, boost::asio::buffer(response));
        std::cout << "Response: 404 Not Found.\n\n";
        return NOT_FOUND;
    }

    char temp_buffer[512];
    while (in_stream.read(temp_buffer, sizeof(temp_buffer)).gcount() > 0) {
        // rep.content.append(temp_buffer, in_stream.gcount());
        boost::asio::write(sock, boost::asio::buffer(temp_buffer, in_stream.gcount()));
    }

    std::cout << "Response: 200 OK.\n\n";
    return OK;
}