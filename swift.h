/**
* SWIFT
* Copyright (c) 2014 Thomas Lextrait <thomas.lextrait@gmail.com>
* All rights reserved
*/

#include "mongoose.h"

#define _SWIFT_VERSION "0.1"

#define _SWIFT_MAX_SERVER_THREADS 255
#define _SWIFT_DEFAULT_PORT 5555

// Swift Server object
class Swift {

		// Mongoose server
		struct mg_server* mgserver;

		// Various settings
		bool verbose;

	public:
		// Constructor/destructor
		Swift();
		~Swift();

		// Server loading
		static Swift* newServer();
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
class SwiftRequest {
		const char *request_method; // "GET", "POST", etc
		const char *uri;            // URL-decoded URI
		const char *http_version;   // E.g. "1.0", "1.1"
		const char *query_string;   // URL part after '?', not including '?', or NULL

		char remote_ip[48];         // Max IPv6 string length is 45 characters
		char local_ip[48];          // Local IP address
		unsigned short remote_port; // Client's port
		unsigned short local_port;  // Local port number

		int num_headers;            // Number of HTTP headers
		struct swift_header {
			const char *name;       // HTTP header name
			const char *value;      // HTTP header value
		} http_headers[30];

		char *content;              // POST (or websocket message) data, or NULL
		size_t content_len;         // Data length
	
	public:
		SwiftRequest();
		SwiftRequest(struct mg_connection* conn);
};
