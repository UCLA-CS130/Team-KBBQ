#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "Webserver.h"
#include "config_parser.h"
#include "HttpRequest.h"
#include "HttpResponse.h"
#include "HttpConstants.h"
#include <fstream>
#include <iostream>
#include <cstdio>

using ::testing::Return;
using ::testing::_;

class MockHttpRequest : public HttpRequest {
public:
    MOCK_METHOD0(to_string, std::string());
    MOCK_METHOD0(get_file, std::string());
    MOCK_METHOD0(get_type, std::string());
    MOCK_METHOD1(create_request, int(boost::asio::streambuf& buffer));
};

//Correct response is made with simple input
TEST(CreateResponseTest, Simple){

    Webserver server;
    std::unique_ptr<HttpResponse> response_ptr;
    MockHttpRequest processed_request;
    ON_CALL(processed_request, create_request(_)).WillByDefault(Return(0));
    ON_CALL(processed_request, get_file()).WillByDefault(Return(""));
    ON_CALL(processed_request, get_type()).WillByDefault(Return("echo"));
    ON_CALL(processed_request, to_string()).WillByDefault(Return("GET /echo HTTP/1.0\r\n\r\n"));

    int status = server.create_response(processed_request, response_ptr);

    //Check that string response is equal
    EXPECT_EQ(0, status);

}

class LoadConfigTest : public ::testing::Test {
protected:
    Webserver server;
    NginxConfigParser parser;
    NginxConfig out_config;
};

//successful load config with child block
TEST_F(LoadConfigTest, ValidConfigTest){
    
    //create a config
    std::stringstream config_stream("server { port 8080; }");
    parser.Parse(&config_stream, &out_config);
    
    bool loaded_config = server.load_configs(out_config, "", 0);
    std::string port = server.get_server_config("port");

    //assert that config was loaded correctly
    ASSERT_TRUE(loaded_config);
    EXPECT_EQ("8080", port);
    
}

//unsuccessful load config with nested child block
TEST_F(LoadConfigTest, NestedConfigTest){
    
    //create a config
    std::stringstream config_stream("server { port 8080; what { is this; } }");
    parser.Parse(&config_stream, &out_config);
    
    bool loaded_config = server.load_configs(out_config, "", 0);

    //assert that config is incorrect
    ASSERT_FALSE(loaded_config);
    
}

//unsuccessful load config
TEST_F(LoadConfigTest, InvalidConfig){

    //create a config
    std::stringstream config_stream("port;");
    parser.Parse(&config_stream, &out_config);
    
    bool loaded_config = server.load_configs(out_config, "", 0);

    //assert that config was loaded correctly
    ASSERT_FALSE(loaded_config);
}


//successful load config, but empty port number
TEST_F(LoadConfigTest, EmptyConfig){
    
    //create a config
    std::stringstream config_stream("listen 8080;");
    parser.Parse(&config_stream, &out_config);
    
    bool loaded_config = server.load_configs(out_config, "", 0);
    std::string port = server.get_server_config("port");

    ASSERT_TRUE(loaded_config);
    EXPECT_EQ("", port);
    
}

//successful load config with two child blocks
TEST_F(LoadConfigTest, ServDirConfigTest){
    
    //create a config
    std::stringstream config_stream("server { port 8080; } directories { /static /root; }");
    parser.Parse(&config_stream, &out_config);
    
    bool loaded_config = server.load_configs(out_config, "", 0);
    std::string port = server.get_server_config("port");
    std::string dir = server.get_dir_config("/static");

    //assert that config was loaded correctly
    ASSERT_TRUE(loaded_config);
    EXPECT_EQ("8080", port);
    EXPECT_EQ("/root", dir);
    
}

//successful load config with statement outside of block
TEST_F(LoadConfigTest, OutsideConfigTest){
    
    //create a config
    std::stringstream config_stream("server { port 8080; } /static /root;");
    parser.Parse(&config_stream, &out_config);
    
    bool loaded_config = server.load_configs(out_config, "", 0);
    std::string port = server.get_server_config("port");
    std::string dir = server.get_server_config("/static");
    std::string dir2 = server.get_dir_config("/static");

    //assert that config was loaded correctly
    ASSERT_TRUE(loaded_config);
    EXPECT_EQ("8080", port);

    //assert that statement outside of block is not loaded
    EXPECT_NE("/root", dir);
    EXPECT_NE("/root", dir2);
    
}

//successful parse config 
TEST(ParseConfigTest, ValidParse){

    //Create necessary classes
    Webserver server;
    std::ofstream config_file("config_test"); 
    
    //assert file can be opened
    ASSERT_TRUE(config_file);
    config_file << "server { port 8080; }";
    config_file.close();

    bool parsed_config = server.parse_config("config_test");
    std::remove("config_test"); 

    //assert that config was loaded correctly
    ASSERT_TRUE(parsed_config);
    
}

//unsuccessful parse config 
TEST(ParseConfigTest, InvalidParse){

    //Create necessary classes
    Webserver server;
    std::ofstream config_file("config_test"); 

    //assert file can be opened
    ASSERT_TRUE(config_file);
    config_file << "server { listen 8080 };";
    config_file.close();
    
    bool parsed_config = server.parse_config("config_test");
    std::remove("config_test"); 
    
    //assert that parse failed
    ASSERT_FALSE(parsed_config);    
}



