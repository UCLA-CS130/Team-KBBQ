# Assignment 9 Design Document - Team KBBQ
## Markdown Rendering

Link: <http://ec2-54-202-60-252.us-west-2.compute.amazonaws.com/static/markdown.md>
* Used [cpp-markdown](https://sourceforge.net/projects/cpp-markdown/) library to handle conversion from markdown to html
* Added github markdown css taken from <https://github.com/sindresorhus/github-markdown-css>
* Extension of StaticFileHandler
* This page is also displayed using markdown feature!

## Sessions and Authentication
### Feature Description
The feature allows files to be only accessed upon authentication by certain users. Once authenticated, a cookie is created and is only valid within a certain time limit. Once the time passes, the user must authenticate again to access the private files. Multiple users are supported and can access the files at the same time.

### Implementation
The is an addon to the StaticFileHandler. The core functionality uses the HandleRequest method to send the response. To create a cookie, a random string is generated and then set as a HTTP header. When the server receives a request it checks the header to make sure the cookie is still valid.

#### Configuration
The folder set to be private and users who have access are set in the config file. It is identical to the regular StaticFileHandler config, except there is a timeout and users. Each user must have a username and password, and the timeout must be a number.

#### Basic functionality
On initialization, the users and timeout are stored. When handling a request, the uri is checked to see if it "login.html". If the uri is about login or just a regular static file, the next part is skipped.

If the request is for a private file, the cookie is checked with the check_cookie method. It checks if the cookie from request is already stored or not. If it is, then its creation time is retrieved. If the cookie is not there, time is set to 0. If the current time - cookie time > timeout, redirect to the login page and delete the old cookie.

If the request method is POST and is coming from "login.html", extract the username and password from the body. If the user and password are correct, then add the cookie with the method add_cookie. This method generates a random alphanumeric string of length 20 and adds the string to the cookie map along with the current time. Afterwards, redirect to the original uri and set the cookie in the HTTP header.

### Walkthrough
Link: <http://ec2-54-202-60-252.us-west-2.compute.amazonaws.com/private/login.html>

Login page to access private files

Link 2: <http://ec2-54-202-60-252.us-west-2.compute.amazonaws.com/private/pikachu.jpg>

Direct link to private file, which will redirect to login

The three users who can authenticate are the following:

<table>
  <tr>
    <th>username</th>
    <th>password</th>
  </tr>
  <tr>
    <td>leslie</td>
    <td>lam</td>
  </tr>
  <tr>
    <td>stella</td>
    <td>chung</td>
  </tr>
  <tr>
    <td>thomas</td>
    <td>choi</td>
  </tr>
</table>

1. Access link 2, and redirect to the login page
2. Authenticate using one of the three users, and redirect back to the picture
3. Demo the Database Interface, while cookie still valid, or
4. Wait 4 minutes, which is the timeout time
5. Attempt to access link 2 again, which will redirect to the login page

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
