#include "gtest/gtest.h"
#include "database_handler.h"

// Test fixture
class DatabaseHandlerTest : public ::testing::Test {
protected:

    bool CreateConfig(const std::string config_string) {
        std::stringstream config_stream(config_string);
        return parser_.Parse(&config_stream, &config_);
    }
    // Connect to database
    bool CreateConnection(const std::string& host, const std::string& username, 
                          const std::string& password, const std::string& db) {
        driver_ = sql::mysql::get_mysql_driver_instance();
        connection_ = driver_->connect(host, username, password);
        if (connection_) {
            connection_->setSchema(db);
            return true;
        }
        return false;
    }

    // Free connection pointer.
    // Must be called at end of every test that makes a connection!
    void ClearConnection() {
        if (connection_) {
            delete connection_;
        }
    }

    DatabaseHandler db_handler_;
    sql::mysql::MySQL_Driver *driver_;
    NginxConfigParser parser_;
    NginxConfig config_;
    sql::Connection *connection_;
};

// Init with valid config
TEST_F(DatabaseHandlerTest, BasicInit) {
    ASSERT_TRUE(CreateConfig("database CS130; username root; password password;"));
    EXPECT_EQ(RequestHandler::Status::OK, db_handler_.Init("/database", config_));
}

// Init with invalid config: duplicate database
TEST_F(DatabaseHandlerTest, DuplicateDatabaseConfig) {
    ASSERT_TRUE(CreateConfig("database CS130; username root; password password; database CS131;"));
    EXPECT_EQ(RequestHandler::Status::INVALID_CONFIG, db_handler_.Init("/database", config_));
}

// Init with invalid config: duplicate username
TEST_F(DatabaseHandlerTest, DuplicateUsernameConfig) {
    ASSERT_TRUE(CreateConfig("database CS130; username root; password password; username CS131;"));
    EXPECT_EQ(RequestHandler::Status::INVALID_CONFIG, db_handler_.Init("/database", config_));
}
// Init with invalid config: duplicate password
TEST_F(DatabaseHandlerTest, DuplicatePasswordConfig) {
    ASSERT_TRUE(CreateConfig("database CS130; username root; password password; password CS131;"));
    EXPECT_EQ(RequestHandler::Status::INVALID_CONFIG, db_handler_.Init("/database", config_));
}
// Init with invalid config: missing database
TEST_F(DatabaseHandlerTest, MissingDatabaseConfig) {
    ASSERT_TRUE(CreateConfig("username root; password password;"));
    EXPECT_EQ(RequestHandler::Status::INVALID_CONFIG, db_handler_.Init("/database", config_));
}

// Test Set Response
TEST_F(DatabaseHandlerTest, SetResponse) {
    Response resp;
    std::string expected =
        "HTTP/1.0 200 OK\r\n"
        "Content-Type: text/plain\r\n"
        "Content-Length: 7\r\n"
        "\r\n"
        "foo bar";
    db_handler_.SetResponse("foo bar", &resp);
    EXPECT_EQ(expected, resp.ToString());
}

// Extract query from valid URI
TEST_F(DatabaseHandlerTest, BasicExtractQuery) {
    std::string uri = "/database/?query=select%20*%20from%20Persons;";
    EXPECT_EQ("select * from Persons;", db_handler_.ExtractQuery(uri));
}

// Extract query from URI with no query parameter
TEST_F(DatabaseHandlerTest, InvalidExtractQuery) {
    std::string uri = "/database/?param=select%20*%20from%20Persons;";
    EXPECT_EQ("", db_handler_.ExtractQuery(uri));
}

// Test ExecuteQuery (Non-select queries)
TEST_F(DatabaseHandlerTest, CreateInsertUpdateDelete) {
    // Connect to local DB with username "root", password "password"
    ASSERT_TRUE(CreateConnection("localhost", "root", "password", "CS130"));

    // Queries to run:
    // Create new test table
    std::string create_table = 
        "CREATE TABLE TestTable ("
            "TestID int NOT NULL AUTO_INCREMENT,"
            "TestValue varchar(255),"
            "PRIMARY KEY (TestID)"
        ");";
    ASSERT_EQ("Query OK, 0 rows affected.", db_handler_.ExecuteQuery(connection_, create_table));

    // Insert into new test table
    std::string insert_table = 
        "INSERT INTO TestTable (TestValue)"
        "Values ('a'), ('b'), ('c');";
    ASSERT_EQ("Query OK, 3 rows affected.", db_handler_.ExecuteQuery(connection_, insert_table));

    // Remove a value
    std::string delete_value =
        "DELETE FROM TestTable WHERE TestValue='c';";
    ASSERT_EQ("Query OK, 1 rows affected.", db_handler_.ExecuteQuery(connection_, delete_value));

    // Clean up by deleting the table
    std::string drop_table =
        "DROP TABLE TestTable;";
    ASSERT_EQ("Query OK, 0 rows affected.", db_handler_.ExecuteQuery(connection_, drop_table));
        
    // Clean up connection
    ClearConnection();
}

// Test ExecuteQuery/GetJSONResults (SelectQuery)
TEST_F(DatabaseHandlerTest, SelectQuery) {
    // Connect to local DB with username "root", password "password"
    ASSERT_TRUE(CreateConnection("localhost", "root", "password", "CS130"));

    // Queries to run:
    // Create new test table
    std::string create_table = 
        "CREATE TABLE TestTable ("
            "TestID int NOT NULL AUTO_INCREMENT,"
            "TestValue varchar(255),"
            "PRIMARY KEY (TestID)"
        ");";
    ASSERT_EQ("Query OK, 0 rows affected.", db_handler_.ExecuteQuery(connection_, create_table));

    // Insert into new test table
    std::string insert_table = 
        "INSERT INTO TestTable (TestValue)"
        "Values ('a'), ('b'), ('c');";
    ASSERT_EQ("Query OK, 3 rows affected.", db_handler_.ExecuteQuery(connection_, insert_table));

    // Select all values
    std::string select_query = "SELECT * FROM TestTable";
    std::string expected_JSON = 
        "{\n"
        "\"cols\": [\"TestID\",\"TestValue\"],\n"
        "\"data\": {\n"
        "\"1\": [\"1\",\"a\"],\n"
        "\"2\": [\"2\",\"b\"],\n"
        "\"3\": [\"3\",\"c\"]}\n}";

    EXPECT_EQ(expected_JSON, db_handler_.ExecuteQuery(connection_, select_query));

    // Clean up by deleting the table
    std::string drop_table =
        "DROP TABLE TestTable;";
    ASSERT_EQ("Query OK, 0 rows affected.", db_handler_.ExecuteQuery(connection_, drop_table));
        
    // Clean up connection
    ClearConnection();
}
