#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "Webserver.h"
#include "request_handler.h"
#include "config_parser.h"
#include <fstream>
#include <iostream>
#include <cstdio>

using ::testing::Return;
using ::testing::_;

// class MockHttpRequest : public HttpRequest {
// public:
//     MOCK_METHOD0(to_string, std::string());
//     MOCK_METHOD0(get_file, std::string());
//     MOCK_METHOD0(get_type, std::string());
//     MOCK_METHOD1(create_request, int(boost::asio::streambuf& buffer));
// };

// class MockWebserver : public Webserver {
// public:
//     MOCK_METHOD1(get_server_config, std::string(std::string attribute));
// };

// //Correct response is made with simple input
// TEST(CreateResponseTest, Simple){

//     MockWebserver server;
//     std::unique_ptr<HttpResponse> response_ptr;
//     MockHttpRequest processed_request;

//     EXPECT_CALL(processed_request, get_file()).Times(1).WillOnce(Return(""));
//     EXPECT_CALL(processed_request, get_type()).Times(1).WillOnce(Return("echo_request"));
//     EXPECT_CALL(processed_request, to_string()).Times(1).WillOnce(Return("GET /echo_request HTTP/1.0\r\n\r\n"));
//     EXPECT_CALL(server, get_server_config(_)).Times(1).WillOnce(Return("echo_request"));
    
//     int status = server.create_response(processed_request, response_ptr);

//     //Check that string response is equal
//     EXPECT_EQ(0, status);
// }

class LoadConfigTest : public ::testing::Test {
protected:
    Webserver server;
    NginxConfigParser parser;
    NginxConfig out_config;
};

//unsuccessful load config with empty config
TEST_F(LoadConfigTest, EmptyConfigTest){
    
    //create a config
    std::stringstream config_stream("");
    parser.Parse(&config_stream, &out_config);
    
    bool loaded_config = server.load_configs(out_config);

    //assert that config was loaded correctly
    ASSERT_FALSE(loaded_config);
}

//successful load config with port number
TEST_F(LoadConfigTest, PortConfigTest){
    
    //create a config
    std::stringstream config_stream("port 8080;");
    parser.Parse(&config_stream, &out_config);
    
    bool loaded_config = server.load_configs(out_config);

    //assert that config was loaded correctly
    ASSERT_TRUE(loaded_config);
    EXPECT_EQ(8080, server.get_port());
}

//successful load config for EchoHandler
TEST_F(LoadConfigTest, EchoConfigTest){
    
    //create a config
    std::stringstream config_stream("path /echo EchoHandler {}");
    parser.Parse(&config_stream, &out_config);
    
    bool loaded_config = server.load_configs(out_config);
    RequestHandler* handler = server.get_config("/echo");

    //assert that config was loaded correctly
    ASSERT_TRUE(loaded_config);
    EXPECT_TRUE(handler);
}

//successful load config for StaticHandler
TEST_F(LoadConfigTest, StaticConfigTest){
    
    //create a config
    std::stringstream config_stream("path / StaticHandler { root /foo/bar; }");
    parser.Parse(&config_stream, &out_config);
    
    bool loaded_config = server.load_configs(out_config);
    RequestHandler* handler = server.get_config("/");

    //assert that config was loaded correctly
    ASSERT_TRUE(loaded_config);
    EXPECT_TRUE(handler);
}

//successful load config for NotFoundHandler
TEST_F(LoadConfigTest, DefaultConfigTest){
    
    //create a config
    std::stringstream config_stream("default NotFoundHandler {}");
    parser.Parse(&config_stream, &out_config);

    bool loaded_config = server.load_configs(out_config);
    RequestHandler* handler = server.get_config("default");

    //assert that config was loaded correctly
    ASSERT_TRUE(loaded_config);
    EXPECT_TRUE(handler);
}

//unsuccessful load config
TEST_F(LoadConfigTest, InvalidConfigTest){

    //create a config
    std::stringstream config_stream("listen 2020;");
    parser.Parse(&config_stream, &out_config);
    
    bool loaded_config = server.load_configs(out_config);

    //assert that config was loaded correctly
    ASSERT_FALSE(loaded_config);
}

//unsuccessful load config with empty path
TEST_F(LoadConfigTest, NoPathConfigTest){
    
    //create a config
    std::stringstream config_stream("path / StaticHandler {}");
    parser.Parse(&config_stream, &out_config);
    
    bool loaded_config = server.load_configs(out_config);

    //assert that config was loaded correctly
    ASSERT_FALSE(loaded_config);
}

//unsuccessful load config with same mapping
TEST_F(LoadConfigTest, SameConfigTest){
    
    //create a config
    std::stringstream config_stream("path / StaticHandler { root /foo; } path / StaticHandler { root /bar; }");
    parser.Parse(&config_stream, &out_config);
    
    bool loaded_config = server.load_configs(out_config);

    //assert that config was loaded correctly
    ASSERT_FALSE(loaded_config);
}

//unsuccessful load config with EchoHandler child
TEST_F(LoadConfigTest, ChildConfigTest){
    
    //create a config
    std::stringstream config_stream("path /echo EchoHandler { root /foot/bar; }");
    parser.Parse(&config_stream, &out_config);
    
    bool loaded_config = server.load_configs(out_config);

    //assert that config was loaded correctly
    ASSERT_FALSE(loaded_config);
}

//successful parse config
TEST(ParseConfigTest, ValidParseTest){

    //Create necessary classes
    Webserver server;
    std::ofstream config_file("config_test"); 
    
    //assert file can be opened
    ASSERT_TRUE(config_file);
    config_file << "port 8080;";
    config_file.close();

    bool parsed_config = server.parse_config("config_test");
    std::remove("config_test"); 

    //assert that config was loaded correctly
    ASSERT_TRUE(parsed_config);
}

//unsuccessful parse config
TEST(ParseConfigTest, InvalidParseTest){

    //Create necessary classes
    Webserver server;
    std::ofstream config_file("config_test"); 

    //assert file can be opened
    ASSERT_TRUE(config_file);
    config_file << "port 8080";
    config_file.close();
    
    bool parsed_config = server.parse_config("config_test");
    std::remove("config_test"); 
    
    //assert that parse failed
    ASSERT_FALSE(parsed_config);    
}
