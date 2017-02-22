#include "gtest/gtest.h"
#include "Webserver.h"
#include "request_handler.h"
#include "config_parser.h"
#include <fstream>
#include <iostream>
#include <cstdio>

class LoadConfigTest : public ::testing::Test {
protected:
    Webserver server;
    NginxConfigParser parser;
    NginxConfig out_config;
};

//unsuccessful load config with empty config
TEST_F(LoadConfigTest, EmptyConfigTest) {
    //create a config
    std::stringstream config_stream("");
    parser.Parse(&config_stream, &out_config);
    
    bool loaded_config = server.load_configs(out_config);

    //assert that config was loaded correctly
    ASSERT_FALSE(loaded_config);
}

//successful load config with port number
TEST_F(LoadConfigTest, PortConfigTest) {
    //create a config
    std::stringstream config_stream("port 8080;");
    parser.Parse(&config_stream, &out_config);
    
    bool loaded_config = server.load_configs(out_config);

    //assert that config was loaded correctly
    ASSERT_TRUE(loaded_config);
    EXPECT_EQ(8080, server.get_port());
}

//successful load config for EchoHandler
TEST_F(LoadConfigTest, EchoConfigTest) {
    //create a config
    std::stringstream config_stream("path /echo EchoHandler {}");
    parser.Parse(&config_stream, &out_config);
    
    bool loaded_config = server.load_configs(out_config);
    RequestHandler* handler = server.get_handler("/echo");

    //assert that config was loaded correctly
    ASSERT_TRUE(loaded_config);
    EXPECT_TRUE(handler);
}

//successful load config for StaticHandler
TEST_F(LoadConfigTest, StaticConfigTest) {
    //create a config
    std::stringstream config_stream("path / StaticFileHandler { root /foo/bar; }");
    parser.Parse(&config_stream, &out_config);
    
    bool loaded_config = server.load_configs(out_config);
    RequestHandler* handler = server.get_handler("/");

    //assert that config was loaded correctly
    ASSERT_TRUE(loaded_config);
    EXPECT_TRUE(handler);
}

//successful load config for NotFoundHandler
TEST_F(LoadConfigTest, DefaultConfigTest) {
    //create a config
    std::stringstream config_stream("default NotFoundHandler {}");
    parser.Parse(&config_stream, &out_config);

    bool loaded_config = server.load_configs(out_config);
    RequestHandler* handler = server.get_handler("default");

    //assert that config was loaded correctly
    ASSERT_TRUE(loaded_config);
    EXPECT_TRUE(handler);
}

//unsuccessful load config
TEST_F(LoadConfigTest, InvalidConfigTest) {
    //create a config
    std::stringstream config_stream("listen 2020;");
    parser.Parse(&config_stream, &out_config);
    
    bool loaded_config = server.load_configs(out_config);

    //assert that config was loaded correctly
    ASSERT_FALSE(loaded_config);
}

//unsuccessful load config with empty path
TEST_F(LoadConfigTest, NoPathConfigTest) {
    //create a config
    std::stringstream config_stream("path / StaticFileHandler {}");
    parser.Parse(&config_stream, &out_config);
    
    bool loaded_config = server.load_configs(out_config);

    //assert that config was loaded correctly
    ASSERT_FALSE(loaded_config);
}

//unsuccessful load config with same mapping
TEST_F(LoadConfigTest, SameConfigTest) {
    //create a config
    std::stringstream config_stream("path / StaticFileHandler { root /foo; } path / StaticFileHandler { root /bar; }");
    parser.Parse(&config_stream, &out_config);
    
    bool loaded_config = server.load_configs(out_config);

    //assert that config was loaded correctly
    ASSERT_FALSE(loaded_config);
}

//successful parse config
TEST(ParseConfigTest, ValidParseTest) {
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
TEST(ParseConfigTest, InvalidParseTest) {
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

//successful prefix find
TEST(FindPrefixTest, ValidPrefixTest) {
    //Create necessary classes
    Webserver server;
    NginxConfigParser parser;
    NginxConfig out_config;

    //create a config
    std::stringstream config_stream("path /foo StaticFileHandler { root /foo; }");
    parser.Parse(&config_stream, &out_config);
    
    bool loaded_config = server.load_configs(out_config);

    //expect that prefix is found
    std::string prefix = server.find_prefix("/foo/bar/a.txt");
    EXPECT_EQ("/foo", prefix);
}

//successful buffer to string
TEST(BufStrTest, ValidBufStrTest) {
    //Create necessary classes
    Webserver server;
    std::string output;
    boost::asio::streambuf request;
    std::ostream request_stream(&request);
    request_stream << "asdfasdf";

    //convert to string
    output = server.buffer_to_string(request);

    //expect that prefix is found
    EXPECT_EQ("asdfasdf", output);
}
