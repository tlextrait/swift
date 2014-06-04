/**
* SWIFT
* Copyright (c) 2014 Thomas Lextrait <thomas.lextrait@gmail.com>
* All rights reserved
*/

#include <string>
#include <exception>
#include <map>
#include <set>
#include <queue>

#include "mongoose.h"

#define _SWIFT_VERSION "0.1"
#define _SWIFT_WELCOME "SWIFT SERVER"
#define _SWIFT_SYMB_REQ "*"

#define _SWIFT_MAX_SERVER_THREADS 255
#define _SWIFT_DEFAULT_PORT 5555
#define _SWIFT_DEFAULT_CACHE_SIZE 2147483648 // 2GB

namespace swift{

	// HTTP request method types
	enum class Method {
		GET, HEAD, POST, PUT, DELETE, TRACE, OPTIONS, CONNECT, PATCH
	};

	// Converts a string to a method enum
	Method str_to_method(std::string request_method);

	// MIME Class
	class MIME {
			// maps extensions to mime types
			std::map<std::string,std::string> types;
		public:
			MIME();
			MIME(std::string file_path);
			std::string getMIME(std::string file_extension);
	};

	// Header class
	class Header {
			std::string name;
			std::string value;
		public:
			// Constructor/destructor
			Header();
			Header(std::string name, std::string value);
			std::string getName();
			std::string getValue();
	};

	// Swift Request class
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

			std::queue<Header*> headers;

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
			int getHeaderCount();
	};

	// Swift response class
	class Response {
			char* content;
			size_t content_len;
			bool binary_mode;

			std::queue<Header*> headers;

		public:
			// Constructor/destructor
			Response();

			void addHeader(Header* header);
			void addHeader(std::string name, std::string value);
			int getHeaderCount();

			void setContent(char* content, int length);
			size_t getContentLen();

			void setBinaryMode(bool binary);
			bool isBinary();

			std::queue<Header*> getHeaderQueue();
	};

	// API Hook
	class Hook {
			std::string request_path;			// request path

			bool is_resource;					// is this a resource?
			std::string resource_path; 			// path to resource file
			bool preload_resource;				// preload the resource?

			std::set<Method> allowed_methods;	// POST, GET... (overrides server settings)
			Response* (*callback_function)(Request*); 	// pointer to function

			std::set<unsigned short> allowed_ports; // (overrides server settings)
		
		public:
			// Constructor/destructor
			Hook();
			Hook(std::string request_path, Response* (*function)(Request*));
			Hook(std::string request_path, std::string resource_path);

			void setRequestPath(std::string path);
			std::string getRequestPath();

			void allowMethod(Method m);
			void disallowMethod(Method m);
			bool isMethodAllowed(Method m);

			void setResourcePath(std::string path);
			std::string getResourcePath();
			void setPreloadResource(bool preload);
			void setIsResource(bool resource);
			bool isResource();

			void setCallback(Response* (*function)(Request*));
			Response* processCallback(Request* req);
	};

	// Swift Server class
	class Server {

			// Mongoose server
			struct mg_server* mgserver;

			// Request paths
			std::map<std::string,Hook*> endpoints;

			// Various settings
			size_t max_cache_size;
			bool verbose;

			// Global server restrictions - default settings, overridden by each API hook
			std::set<Method> allowed_methods;
			std::set<unsigned short> allowed_ports;

		public:
			// Constructor/destructor
			Server();
			~Server();

			// Server loading
			static Server* newServer(); 	// factory
			void Start();
			void Start(int port);

			// API
			void addResource(std::string request_path, std::string file_path);
			void addResource(std::string request_path, std::string file_path, bool preload);
			void addHook(Hook* hook);

			// MISC
			void setCacheSize(size_t size);
			void setVerbose(bool);

		private:

			static int requestHandler(struct mg_connection *conn, enum mg_event ev);
			static void processRequest(Request* req, struct mg_connection *conn);

			static bool hasServer(int server_id);
			static bool addServer(Server* server, int server_id);
			static Server* getServer(int server_id);

			bool addEndpoint(std::string path, Hook* hook);
			bool hasEndpointWithPath(std::string path);
			Hook* getEndpoint(std::string path);
			Response* serveResource(std::string file_path);

			void sendResponse(Response* resp, struct mg_connection *conn);

			// MISC
			void printWelcome();
	};

	/* ======================================================== */
	/* Exceptions												*/
	/* ======================================================== */

	// Invalid method exception
	class Ex_invalid_method: public std::exception{
	  	virtual const char* what() const throw(){
	    	return "Invalid request method";
	  	}
	};

	class Ex_null_request: public std::exception{
		virtual const char* what() const throw(){
	    	return "Null request";
	  	}
	};

	class Ex_null_uri: public std::exception{
		virtual const char* what() const throw(){
	    	return "Null URI";
	  	}
	};

	class Ex_null_http_version: public std::exception{
		virtual const char* what() const throw(){
	    	return "Null HTTP version";
	  	}
	};

	class Ex_request_path_exists: public std::exception{
		virtual const char* what() const throw(){
	    	return "Request path already exists";
	  	}
	};

	class Ex_file_not_found: public std::exception{
		virtual const char* what() const throw(){
	    	return "File not found";
	  	}
	};

	class Ex_mime_types_file_not_found: public std::exception{
		virtual const char* what() const throw(){
	    	return "MIME types file not found";
	  	}
	};

	class Ex_no_mime_type: public std::exception{
		virtual const char* what() const throw(){
	    	return "No MIME type for given file extension";
	  	}
	};

}
