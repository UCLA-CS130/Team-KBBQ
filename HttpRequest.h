#ifndef REQUEST_H
#define REQUEST_H

#include <boost/asio.hpp>

class HttpRequest {
public:
	int create_request(boost::asio::streambuf& buffer);
	std::string to_string();
	std::string get_file();
	std::string get_type();

private:
	std::string file;
	std::string buffer;
	std::string type;
};

#endif
