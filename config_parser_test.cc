#include "gtest/gtest.h"
#include "config_parser.h"

// Basic config file
TEST(NginxConfigParserTest, SimpleConfig) {
  NginxConfigParser parser;
  NginxConfig out_config;

  bool success = parser.Parse("example_config", &out_config);

  EXPECT_TRUE(success);
}

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
