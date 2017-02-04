#ifndef REQUEST_H
#define REQUEST_H

#include <boost/asio.hpp>

class HttpRequest {

public:

	int createRequest(boost::asio::streambuf& buffer);
	std::string ToString();
	std::string getFile();
	std::string getType();

private:
	std::string file;
	std::string buffer;
	std::string type;
};

#endif