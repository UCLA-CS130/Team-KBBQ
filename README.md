# Team KBBQ

## Webserver API

### Request

To create a request object call `auto request = Request::Parse(request)`. Other functions return information on the parsed request.

```cpp
std::unique_ptr<Request> Parse(const std::string& raw_request);
std::string raw_request() const;
std::string method() const;
std::string uri() const;
std::string version() const;
using Headers = std::vector<std::pair<std::string, std::string>>;
Headers headers() const;
std::string body() const;
```
### Response
The response class defines response codes and simple functions to set status, headers, and body of response which can be outputted in `ToString()`.
```cpp
enum ResponseCode {
        OK = 200,
        BAD_REQUEST = 400,
        NOT_FOUND = 404,
        NOT_IMPLEMENTED = 501
};
void SetStatus(const ResponseCode response_code);
void AddHeader(const std::string& header_name, const std::string& header_value);
void SetBody(const std::string& body);
std::string ToString();
```


### Request Handler
```cpp
enum Status {
        OK = 0,
        INVALID_CONFIG = 1,
        INVALID_URI = 2,
        FILE_NOT_FOUND = 3
    };
    
virtual Status Init(const std::string& uri_prefix, const NginxConfig& config) = 0;
```
`Init` initializes the handler. The `uri_prefix` is the value in the config file specified for the current handler. The config must be the child block for this handler ONLY.
```cpp
virtual Status HandleRequest(const Request& request, Response* response) = 0;
```
`HandleRequest` handles an HTTP request, and generates a response. 
```cpp
static RequestHandler* CreateByName(const char* type);
```
Use `CreateByName` to generate a pointer to the handler specified in the argument (eg.  `auto handler = RequestHandler::CreateByName("EchoHandler")`).

#### Types of Handlers (Inherited from Request Handler class)
* EchoHandler - returns request 
* StaticFileHandler - returns specified file 

### Dispatch

In Webserver_main.cc:  
1. `bool parse_config(const char* file_name)`
    - Ignore comments  
    - Save the port number  
    - For each handler block:
      - Parse first line
      - `RequestHandler* handler = RequestHandler::CreatebyName(<handler_name>)`
      - `handler->Init(uri, child_block)`
      - Put handler into map (uri -> handler). Duplicate paths are illegal. 
2. `void run_server(boost::asio::io_service& io_service)` creates socket and calls `void session(boost::asio::ip::tcp::socket sock)`.  

Webserver.cc:  
3. `void session(boost::asio::ip::tcp::socket sock)`
    - Read request from the socket
    - Parse into Request class
    - Find longest prefix in map
    - handler->handleRequest(...)
    - Write response to socket
    
## Build

```
make Webserver
./Webserver <config-file>
```
### Config File Format

Specify a handler for each block. If there is no root folder, leave the block empty. 

```
#comments

port <number>;

#specify uri for each type of handler
path /<uri> <handler-name> {
    root /<directory>;
}

#specify a default handler if no handler matches
default <handler-name>{
    
}
```