# Assignment 9 Design Document - Team KBBQ
## Markdown Rendering

Link: <https://ec2-54-202-60-252.us-west-2.compute.amazonaws.com/static/markdown.md>
* Used [cpp-markdown](https://sourceforge.net/projects/cpp-markdown/) library to handle conversion from markdown to html
* Added github markdown css taken from <https://github.com/sindresorhus/github-markdown-css>
* Extension of StaticFileHandler
* This page is also displayed using markdown feature!

## Sessions and Authentication

## Database Interface
### Feature Description
The feature provides a basic commandline interface to a MySQL database. Users
can input SQL queries to select, update, insert, and delete. The webpage
features an input textarea where users can input any valid MySQL command.
After a command is submitted, the webpage retrieves the results from the
server with an AJAX query. The results or error messages are then displayed
below the query box in tabular format.

### Implementation
The interactions with the MySQL server are handled by the [MySQL Connector/C++
library] (https://dev.mysql.com/doc/connector-cpp/en/). A MySQL instance is
installed on the server with a database named "CS130" and a user with root
permissions named "root."

#### Configuration
The database connection configuration is specified in the config file. The
config specifies the database name, the user, and the password. The database
config is required, but the password and user can be left blank. If the
password is left blank, then the connection will attempt to access MySQL
without a password. If the username is left blank, then it will connect with
the default username (which is the Unix login name).

#### Basic functionality
The Init function sets the configurations for the database connection and gets
the MySQL driver.

The HandleRequest function creates a connection to the database and executes
the query. If the query is a select query, then it will return the response in
JSON format. If the query is not a select query, then it will return a string
indicating how many rows were affected. If an error is thrown, then the error
message is caught and returned in the response.

The web interface contains a textarea where users can enter SQL commands. All
valid commands are accepted. When the user hits submit, a JQuery script
captures the input and encodes it as a URI. The script then sends an AJAX GET
request for the results. If the response is in valid JSON format, then the
script builds a table to display the results. Otherwise, it just displays the
result string (either the success message or the error message).

### Demo URL & Walkthrough
The database interface is accessible at
<http://ec2-54-202-60-252.us-west-2.compute.amazonaws.com/private/database.html>.

1. Log in with username `leslie` and password `lam`
2. Enter:


    show tables


  * View the available tables (Actors, Persons)

3. Enter: (Note: Table/column names are case sensitive)


    select * from Actors


  * View the displayed results

4. Enter:


    insert into Actors
    values (
        "Watson, Emma",
        "Harry Potter and the Chamber of Secrets",
         2002,
         "Hermione Granger"
    )


  * Output should say: "Query OK, 1 rows affected"

5. Enter:


    delete from Actors where name like "Watson, Emma"

  * Output should say: "Query OK, 1 rows affected"

6. Enter:


    create table KBBQ (
        ID int NOT NULL AUTO_INCREMENT,
        restaurant varchar(255),
        PRIMARY KEY (ID)
    )

  * Output should say: "Query OK, 0 rows affected"

7. Enter:


    insert into KBBQ (restaurant)
    values ("Bud Namu")


  * Output should say: "Query OK, 1 rows affected"

8. Enter:


    select * from KBBQ


  * Verify that "Bud Namu" exists

9. Enter:


    drop table KBBQ


  * Output should say: "Query OK, 0 rows affected"
