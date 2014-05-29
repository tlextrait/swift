/**
* SWIFT
* Copyright (c) 2014 Thomas Lextrait <thomas.lextrait@gmail.com>
* All rights reserved
*/

#include <string>
#include <exception>

#include "mongoose.h"

#define _SWIFT_VERSION "0.1"
#define _SWIFT_WELCOME "SWIFT SERVER"
#define _SWIFT_SYMB_REQ "*"

#define _SWIFT_MAX_SERVER_THREADS 255
#define _SWIFT_DEFAULT_PORT 5555

namespace swift{

	// HTTP request method types
	enum class Method {
		GET, HEAD, POST, PUT, DELETE, TRACE, OPTIONS, CONNECT, PATCH
	};

	// Swift Server object
	class Server {

			// Mongoose server
			struct mg_server* mgserver;

			// Various settings
			bool verbose;

		public:
			// Constructor/destructor
			Server();
			~Server();

			// Server loading
			static Server* newServer();
			void Start();
			void Start(int port);

			// API
			// void addResource();

			// MISC
			void setVerbose(bool);

		private:

			// Master request handler
			static int requestHandler(struct mg_connection *conn, enum mg_event ev);

			// MISC
			void printWelcome();
	};

	// Swift Request object
	class Request {
			Method request_method;				// "GET", "POST", etc
			std::string request_method_str; 	// "GET", "POST", etc
			std::string uri;            		// URL-decoded URI
			std::string http_version;   		// E.g. "1.0", "1.1"
			std::string query_string;   		// URL part after '?', not including '?', or NULL

			std::string remote_ip;			// Max IPv6 string length is 45 characters
			std::string local_ip;			// Local IP address
			unsigned short remote_port; 	// Client's port
			unsigned short local_port;		// Local port number

			int num_headers;            // Number of HTTP headers
			struct swift_header {
				std::string name;       // HTTP header name
				std::string value;      // HTTP header value
			} http_headers[30];

			std::string content;        // POST (or websocket message) data, or NULL
			size_t content_len;         // Data length
		
		public:
			// Constructor/destructor
			Request();
			Request(struct mg_connection* conn);

			// Getters/setters
			Method getMethod();
			std::string getMethodStr();
			std::string getURI();
			std::string getHTTPVersion();
			std::string getQueryString();
			std::string getRemoteIP();
			std::string getLocalIP();
			unsigned short getRemotePort();
			unsigned short getLocalPort();
			std::string getContent();
			size_t getContentLen();
	};

	// Converts a string to a method enum
	Method str_to_method(std::string request_method);

}
