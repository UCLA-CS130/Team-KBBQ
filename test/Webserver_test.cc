#include "gtest/gtest.h"
#include "Webserver.h"
#include "request_handler.h"
#include "echo_handler.h"
#include "static_file_handler.h"
#include "not_found_handler.h"
#include "config_parser.h"
#include <fstream>
#include <iostream>
#include <cstdio>

class LoadConfigTest : public ::testing::Test {
public:
    bool parse_load(std::string new_config) {
        std::stringstream config_stream(new_config);
        parser.Parse(&config_stream, &out_config);
        return server.load_configs(out_config);
    }
protected:
    Webserver server;
    NginxConfigParser parser;
    NginxConfig out_config;
};

//unsuccessful load config with empty config
TEST_F(LoadConfigTest, EmptyConfigTest) {
    //create a config
    bool loaded_config = parse_load("");

    //assert that config was loaded correctly
    ASSERT_FALSE(loaded_config);
}

//successful load config with port number
TEST_F(LoadConfigTest, PortConfigTest) {
    //create a config
    bool loaded_config = parse_load("port 8080;");

    //assert that config was loaded correctly
    ASSERT_TRUE(loaded_config);
    EXPECT_EQ(8080, server.get_port());
}

//successful load config for EchoHandler
TEST_F(LoadConfigTest, EchoConfigTest) {
    //create a config
    bool loaded_config = parse_load("path /echo EchoHandler {}");
    RequestHandler* handler = server.get_handler("/echo");

    //assert that config was loaded correctly
    ASSERT_TRUE(loaded_config);
    EXPECT_EQ(typeid(EchoHandler), typeid(*handler));
}

//successful load config for StaticHandler
TEST_F(LoadConfigTest, StaticConfigTest) {
    //create a config
    bool loaded_config = parse_load("path / StaticFileHandler { root /foo/bar; }");
    RequestHandler* handler = server.get_handler("/");

    //assert that config was loaded correctly
    ASSERT_TRUE(loaded_config);
    EXPECT_EQ(typeid(StaticFileHandler), typeid(*handler));
}

//successful load config for NotFoundHandler
TEST_F(LoadConfigTest, DefaultConfigTest) {
    //create a config
    bool loaded_config = parse_load("default NotFoundHandler {}");
    RequestHandler* handler = server.get_handler("default");

    //assert that config was loaded correctly
    ASSERT_TRUE(loaded_config);
    EXPECT_EQ(typeid(NotFoundHandler), typeid(*handler));
}

//unsuccessful load config for unknown handler name
TEST_F(LoadConfigTest, UnknownConfigTest) {
    //create a config
    bool loaded_config = parse_load("default FoundHandler {}");

    //assert that config was loaded correctly
    ASSERT_FALSE(loaded_config);
}

//unsuccessful load config
TEST_F(LoadConfigTest, InvalidConfigTest) {
    //create a config
    bool loaded_config = parse_load("listen 2020;");

    //assert that config was loaded correctly
    ASSERT_FALSE(loaded_config);
}

//unsuccessful load config with empty path
TEST_F(LoadConfigTest, NoPathConfigTest) {
    //create a config
    bool loaded_config = parse_load("path / StaticFileHandler {}");

    //assert that config was loaded correctly
    ASSERT_FALSE(loaded_config);
}

//unsuccessful load config with same mapping
TEST_F(LoadConfigTest, SameConfigTest) {
    //create a config
    bool loaded_config = parse_load("path / StaticFileHandler { root /foo; } path / StaticFileHandler { root /bar; }");

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

class PrefixHandlerTest : public ::testing::Test {
public:
    bool parse_load() {
        std::stringstream config_stream("path /foo StaticFileHandler { root /foo/bar; } default NotFoundHandler {}");
        parser.Parse(&config_stream, &out_config);
        return server.load_configs(out_config);
    }
protected:
    Webserver server;
    NginxConfigParser parser;
    NginxConfig out_config;
};

//successful prefix find
TEST_F(PrefixHandlerTest, ValidPrefixTest) {

    //create a config
    bool loaded_config = parse_load();

    //expect that prefix is found
    ASSERT_TRUE(loaded_config);
    std::string prefix = server.find_prefix("/foo/bar/a.txt");
    EXPECT_EQ("/foo", prefix);
}

//successful handler find
TEST_F(PrefixHandlerTest, ValidGetHandlerTest) {

    //create a config
    bool loaded_config = parse_load();

    //expect that the handler is found
    ASSERT_TRUE(loaded_config);
    RequestHandler* handler = server.get_handler("/foo/a.txt");
    EXPECT_EQ(typeid(StaticFileHandler), typeid(*handler));
}

//unsuccessful handler find
TEST_F(PrefixHandlerTest, InvalidGetHandlerTest) {

    //create a config
    bool loaded_config = parse_load();

    //expect that the handler is default
    ASSERT_TRUE(loaded_config);
    RequestHandler* handler = server.get_handler("/bar/a.txt");
    EXPECT_EQ(typeid(NotFoundHandler), typeid(*handler));
}
