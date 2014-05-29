/**
* SWIFT
* Copyright (c) 2014 Thomas Lextrait <thomas.lextrait@gmail.com>
* All rights reserved
*/

#include <string>

#include "mongoose.h"

#define _SWIFT_VERSION "0.1"
#define _SWIFT_WELCOME "SWIFT SERVER"
#define _SWIFT_SYMB_REQ "*"

#define _SWIFT_MAX_SERVER_THREADS 255
#define _SWIFT_DEFAULT_PORT 5555

namespace swift{

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
			std::string request_method; // "GET", "POST", etc
			std::string uri;            // URL-decoded URI
			std::string http_version;   // E.g. "1.0", "1.1"
			std::string query_string;   // URL part after '?', not including '?', or NULL

			char remote_ip[48];         // Max IPv6 string length is 45 characters
			char local_ip[48];          // Local IP address
			unsigned short remote_port; // Client's port
			unsigned short local_port;  // Local port number

			int num_headers;            // Number of HTTP headers
			struct swift_header {
				std::string name;       // HTTP header name
				std::string value;      // HTTP header value
			} http_headers[30];

			std::string content;        // POST (or websocket message) data, or NULL
			size_t content_len;         // Data length
		
		public:
			Request();
			Request(struct mg_connection* conn);
	};

}
