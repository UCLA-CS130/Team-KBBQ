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
	std::string header;
	char req[4];
	char url[100];
	char method[10];
	
    //decode first line
    if(std::getline(request_stream, header) && header != "\r"){
    	
    	sscanf(header.c_str(), "%s %s %s", req, url, method);
		std::cout << header << std::endl;
		std::string req_str(req);
    	if(req_str == "POST"){
    		return -1;
    	}
    	else{
    		
    		//no file
    		std::string url_str(url);  				
    		if(url_str == "/"){
    			file = "";
    			return -1;
    		}
			//erase first slash
			url_str.erase(0, 1);

			std::string delimiter = "/";
			size_t pos = url_str.find(delimiter);
			//set file and type
			type = url_str.substr(0, pos);
			file = url_str.substr(pos+1, std::string::npos);
			
			if( file.back() == '/'){
				file.erase(file.length()-1);
			}

			std::cout << "file is \"" << file << "\", type is \"" << type << "\".\n";
	    	return 0;
    	}
	}	
}

std::string HttpRequest::ToString(){
	return buffer;
};

std::string HttpRequest::getFile(){
	return file;
};

std::string HttpRequest::getType(){
	return type;
};
