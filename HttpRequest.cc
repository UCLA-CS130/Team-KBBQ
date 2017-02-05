#include <boost/asio.hpp>
#include <iostream>
#include <string>
#include "HttpRequest.h"

int HttpRequest::createRequest(boost::asio::streambuf& request){

    //write request into buffer
    boost::asio::streambuf::const_buffers_type req_data = request.data();
    std::string req_string(boost::asio::buffers_begin(req_data), boost::asio::buffers_begin(req_data) + request.size());
    buffer = req_string;

    std::istream request_stream(&request);
    std::string header, req, url, method;

    //decode first line
    if (std::getline(request_stream, header) && header != "\r") {
        std::size_t found1 = header.find_first_of(" ");

        if (found1 != std::string::npos) {
			req = header.substr(0, found1);
		}

        std::size_t found2 = header.find_last_of(" ");

        if (found2 != std::string::npos) {
			url = header.substr(found1+1, found2-found1-1);
			method = header.substr(found2+1, std::string::npos);
		}

		std::cout << "request is " << req << std::endl;
		std::cout << "url is " << url << std::endl;
		std::cout << "method is " << method << std::endl;

        if (req == "POST") {
            return -1;
        }
        else {
            //no file
            if (url == "/") {
                file = "";
                return -1;
            }
			//erase first slash
			url.erase(0, 1);

			std::string delimiter = "/";
			size_t pos = url.find(delimiter);
			//set file and type
			type = url.substr(0, pos);
			file = url.substr(pos+1, std::string::npos);

			if( file.back() == '/'){
				file.erase(file.length()-1);
			}

			std::cout << "file is \"" << file << "\", type is \"" << type << "\".\n";
            return 0;
        }
	}
}

std::string HttpRequest::ToString() {
	return buffer;
}

std::string HttpRequest::getFile() {
	return file;
}

std::string HttpRequest::getType() {
	return type;
}
