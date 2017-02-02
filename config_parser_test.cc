#include <fstream>
#include "gtest/gtest.h"
#include "config_parser.h"

// foo bar;
TEST(NginxConfigTest, ToString) {
	NginxConfigStatement statement;
	statement.tokens_.push_back("foo");
	statement.tokens_.push_back("bar");
	EXPECT_EQ(statement.ToString(0), "foo bar;\n");
}

// Test fixture
class NginxStringConfigTest : public ::testing::Test {
protected:
	bool ParseString(const std::string config_string) {
		std::stringstream config_stream(config_string);
		return parser_.Parse(&config_stream, &out_config_);
	}
	NginxConfigParser parser_;
	NginxConfig out_config_;
};

// "foo bar;" with test fixture
TEST_F(NginxStringConfigTest, AnotherSimpleConfig) {
	EXPECT_TRUE(ParseString("foo bar;"));
	EXPECT_EQ(1, out_config_.statements_.size());
	EXPECT_EQ("foo", out_config_.statements_[0]->tokens_[0]);
}

// "foo bar" with test fixture
TEST_F(NginxStringConfigTest, InvalidConfig) {
	EXPECT_FALSE(ParseString("foo bar"));
}

// Nested statements
TEST_F(NginxStringConfigTest, NestedConfig) {
	ASSERT_TRUE(ParseString("server { listen 80; }"));
	EXPECT_EQ(1, out_config_.statements_.size());
	EXPECT_EQ("server", out_config_.statements_[0]->tokens_[0]);
}

// Multiple braces
TEST_F(NginxStringConfigTest, MultipleBraces) {
	ASSERT_TRUE(ParseString("server { listen 80; something { some statement; } }"));
	EXPECT_EQ(1, out_config_.statements_.size());
}

// Mismatched curly braces
TEST_F(NginxStringConfigTest, MismatchedBraces) {
	ASSERT_FALSE(ParseString("server { listen 80; something { some statement; } "));
}

// Test Basic ToString
TEST_F(NginxStringConfigTest, ToString) {
	ASSERT_TRUE(ParseString("foo bar;"));
	EXPECT_EQ("foo bar;\n", out_config_.ToString(0));
}

// Test Nested ToString
TEST_F(NginxStringConfigTest, NestedToString) {
	ASSERT_TRUE(ParseString("parent{ foo \"bar\"; child { statement; }}"));
	EXPECT_EQ("parent {\n  foo \"bar\";\n  child {\n    statement;\n  }\n}\n", out_config_.ToString(0));
}

// Test Empty Config
TEST_F(NginxStringConfigTest, EmptyConfig) {
	ASSERT_FALSE(ParseString(" "));
}

// Test Mismatched single quotes
TEST_F(NginxStringConfigTest, MismatchedQuotes) {
	ASSERT_FALSE(ParseString(" 'foo "));
}

// Test Open bracket after semicolon
TEST_F(NginxStringConfigTest, BadBracket) {
	ASSERT_FALSE(ParseString(" foo bar; { statement; } "));
}

// Test Bad bracket (missing semicolon)
TEST_F(NginxStringConfigTest, BadBracketMissingSemicolon) {
	ASSERT_FALSE(ParseString(" server { statement } "));
}

// Test Bad semicolon
TEST_F(NginxStringConfigTest, BadSemicolon) {
	ASSERT_FALSE(ParseString(" server { statement; }; "));
}

//Checks that a large config is parsed correctly
//Code taken from https://www.nginx.com/resources/wiki/start/topics/examples/full/
TEST(NginxConfigParserTest, FullExampleConfig) {
	NginxConfigParser parser;
	NginxConfig out_config_;
	EXPECT_TRUE(parser.Parse("fullex_config", &out_config_));
}

// Test bad config file
TEST(NginxConfigParserTest, BadConfig) {
	NginxConfigParser parser;
	NginxConfig out_config_;
	EXPECT_FALSE(parser.Parse("bad_config", &out_config_));
}
