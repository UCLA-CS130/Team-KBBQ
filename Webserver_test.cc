#include "gtest/gtest.h"
#include "Webserver.h"

TEST(CreateResponseTest, Simple){

	Webserver server;
	
	//Create argument input (need to convert string to char*)
	std::string test = "Testing input.";
	char* test_t = &test[0];
	
	//Convert vector response into string response
	std::vector<char> response = server.create_response(test_t, 14);
	std::string response_t(response.begin(), response.end());

	//Check that string response is equal
	EXPECT_EQ(response_t, "HTTP/1.0 200 OK\r\nContent-type: text/plain\r\n\r\nTesting input.");

}
