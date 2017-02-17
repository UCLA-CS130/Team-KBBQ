# Team-KBBQ

Code Sequence:  
  
Webserver_main.cc:  
1. server.parse_config  
    - Ignore comments  
    - Save the port number  
    - For each handler block:  
        1. Parse first line  
        2. RequestHandler* handler = RequestHandler::CreatebyName(handler_name)  
        3. handler->Init(uri, child_block)  
        4. put handler into map (uri -> handler)  
            - If path exists, then error? Duplicate paths are illegal  
  
2. server.run_server -> Creates socket calls session()  
  
Webserver.cc:  
session()  
    1. Read request from the socket  
    2. Parse into Request class  
    3. Find longest prefix in map  
    4. handler->handleRequest(...)  
    5. Write response to socket  
