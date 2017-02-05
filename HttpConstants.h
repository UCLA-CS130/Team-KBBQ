// FILE TYPES
const std::string TYPE_JPEG  = "image/jpeg";
const std::string TYPE_GIF   = "image/gif";
const std::string TYPE_HTML  = "text/html";
const std::string TYPE_OCT   = "application/octet-stream";
const std::string TYPE_PDF   = "application/pdf";
const std::string TYPE_PNG   = "image/png";
const std::string TYPE_TXT   = "text/plain";

// RESPONSE CODES
const int OK = 200;
const int BAD_REQUEST = 400;
const int NOT_FOUND = 404;
const int INTERNAL_ERROR = 500;
const int NOT_IMPLEMENTED = 501;

// RESPONSE HEADERS & TEMPLATES
const std::string OK_HEADER =
    "HTTP/1.0 200 OK\r\n";

const std::string BAD_REQUEST_RESP =
    "HTTP/1.0 400 Bad Request\r\n"
    "Content-type: text/html\r\n"
    "Content-length: 50\r\n"
    "\r\n"
    "<html><body><h1>400 Bad Request</h1></body></html>";

const std::string NOT_FOUND_RESP =
    "HTTP/1.0 404 Not Found\r\n"
    "Content-type: text/html\r\n"
    "Content-length: 48\r\n"
    "\r\n"
    "<html><body><h1>400 Bad Request</h1></body></html>";