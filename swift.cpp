/**
* SWIFT
* Copyright (c) 2014 Thomas Lextrait <thomas.lextrait@gmail.com>
* All rights reserved
*/

#include "swift.h"

namespace swift{

	/* ======================================================== */
	/* Globals													*/
	/* ======================================================== */

	// Maps server id's to swift servers
	std::map<int,Server*> server_map;

	// Global listing of mime types
	std::map<std::string,std::string> mimetypes;

	/* ======================================================== */
	/* Exceptions												*/
	/* ======================================================== */

	Ex_invalid_method ex_invalid_method;
	Ex_null_request ex_null_request;
	Ex_null_uri ex_null_uri;
	Ex_null_http_version ex_null_http_version;
	Ex_request_path_exists ex_request_path_exists;
	Ex_file_not_found ex_file_not_found;
	Ex_mime_types_file_not_found ex_mime_types_file_not_found;
	Ex_no_mime_type ex_no_mime_type;
	Ex_invalid_filename ex_invalid_filename;

	/* ======================================================== */
	/* Server loading											*/
	/* ======================================================== */

	/**
	* Swift constructor
	*/
	Server::Server(){
		// Settings
		max_cache_size = _SWIFT_DEFAULT_CACHE_SIZE;
		verbose = true;
	}

	/**
	* Swift destructor
	*/
	Server::~Server(){
		// destroy the mongoose server
		mg_destroy_server(&mgserver);
	}

	/**
	* Returns a new swift server object
	* @return swift server
	*/
	Server* Server::newServer(){
		Server* swift_server = new Server();
		return swift_server;
	}

	/**
	* Starts the server on default port
	*/
	void Server::Start(){
		Start(_SWIFT_DEFAULT_PORT);
	}

	/**
	* Starts the server on given port
	* @param port
	*/
	void Server::Start(int port){

		// Load MIME types
		if(mimetypes.size() == 0){
			loadMIME("mime.types");
		}

		// Convert the port to string (for mongoose)
		char str_port[10];
		sprintf(str_port, "%d", port);

		// Create a Mongoose server
		int server_id = -1;
		mgserver = mg_create_server(NULL, this->requestHandler, &server_id);
		
		// Keep track of server globally
		addServer(this, server_id);

		// Set the port
		mg_set_option(mgserver, "listening_port", str_port);

		// Display info
		printWelcome();
		std::cout << "Listening on port " << mg_get_option(mgserver, "listening_port") << "..." << std::endl;

		for(;;){
		    mg_poll_server(mgserver, 1000);
		}
	}

	/**
	* Indicates if server with given id exists
	* @param server id
	* @return boolean
	*/
	bool Server::hasServer(int server_id){
		return server_map.count(server_id) > 0;
	}

	/**
	* Adds a server and assigns the given id, to keep track of it
	* @param server object
	* @param server id
	* @return boolean on success
	*/
	bool Server::addServer(Server* server, int server_id){
		bool success = false;
		if(!hasServer(server_id)){
			server_map.insert(std::make_pair(server_id, server));
			success = true;
		}
		return success;
	}

	/**
	* Fetches a server that's being tracked
	* @param server id
	* @return server object, or nullptr if not found
	*/
	Server* Server::getServer(int server_id){
		if(hasServer(server_id)){
			return server_map[server_id];
		}else{
			return nullptr;
		}
	}

	/* ======================================================== */
	/* Request Management										*/
	/* ======================================================== */

	/**
	* Handles requests
	* @param mongoose connection oject
	* @param mongoose event enum
	*/
	int Server::requestHandler(struct mg_connection *conn, enum mg_event ev){
		// Static

		int result = MG_FALSE;

		if(ev == MG_REQUEST){

			// Build Swift Request
			Request* req = new Request(conn);

			// Show we received a request
			std::cout << _SWIFT_SYMB_REQ << " " << req->getURI() << " from " << req->getRemoteIP() << std::endl;

			// Process it
			Server::processRequest(req, conn);

			result = MG_MORE;

		}else if(ev == MG_AUTH){
			result = MG_TRUE;
		}

		return result;
	}

	/**
	* Processes given request
	* @param request object
	* @param mongoose connection object
	*/
	void Server::processRequest(Request* req, struct mg_connection *conn){
		// Static

		// Check that we're tracking this server
		int server_id = conn->server_id;

		if(Server::hasServer(server_id)){
			Server* server = Server::getServer(server_id);

			if(server->verbose) std::cout << "Got request from server #" << conn->server_id << std::endl;

			if(server->hasEndpointWithPath(req->getURI())){

				Hook* hook = server->getEndpoint(req->getURI());

				// Check rules
				if(hook->isMethodAllowed(req->getMethod())){

					Response* resp;

					if(hook->isResource()){
						// Just serve the static resource to the client
						if(server->verbose) std::cout << "Serving static resource" << std::endl;
						resp = server->serveResource(hook->getResourcePath());
					}else{
						// Process the attached callback
						if(server->verbose) std::cout << "Serving dynamic callback" << std::endl;
						resp = hook->getCallbackResponse(req);
					}

					// Send response to client
					server->sendResponse(resp, conn);

				}else{
					if(server->verbose) std::cout << "(403) Forbidden" << std::endl;
					/*
					@TODO ACCESS DENIED
					*/
				}

			}else{
				if(server->verbose) std::cout << "(404) No matching endpoint" << std::endl;
				/*
				@TODO 404 !!!
				*/
			}
		}else{
			std::cout << "Server not found #" << server_id << std::endl;
		}

	}

	/* ======================================================== */
	/* API														*/
	/* ======================================================== */

	/**
	* Adds a file resource to the server with default settings
	* @param request path
	* @param file path
	*/
	void Server::addResource(std::string request_path, std::string file_path){
		addResource(request_path, file_path, true);
	}

	/**
	* Adds a file resource to the server
	* @param request path
	* @param file path
	* @param preload the resource
	*/
	void Server::addResource(std::string request_path, std::string file_path, bool preload){
		// Create the API Hook
		Hook* hook = new Hook(request_path, file_path);
		hook->setPreloadResource(preload);
		hook->allowMethod(Method::GET);

		// Add the endpoint
		addEndpoint(request_path, hook);
	}

	/**
	* Adds an API Hook to the server
	* @param hook object
	*/
	void Server::addHook(Hook* hook){
		addEndpoint(hook->getRequestPath(), hook);
	}

	/**
	* Tries to add an endpoint to the server
	* @param endpoint path
	* @param API Hook
	* @return success
	*/
	bool Server::addEndpoint(std::string path, Hook* hook){
		bool success = false;

		if(!hasEndpointWithPath(path)){
			endpoints.insert(std::make_pair(path, hook));
			success = true;
		}else{
			throw ex_request_path_exists;
		}

		return success;
	}

	/**
	* Checks if an endpoint with given path already exists
	* @param endpoint path
	* @return boolean
	*/
	bool Server::hasEndpointWithPath(std::string path){
		return endpoints.count(path) > 0;
	}

	/**
	* Fetches and endpoint with given path
	* @param path
	* @return Hook object
	*/
	Hook* Server::getEndpoint(std::string path){
		if(hasEndpointWithPath(path)){
			return endpoints[path];
		}else{
			return nullptr;
		}
	}

	/**
	* Serves a static resource to the client
	* @param file path
	*/
	Response* Server::serveResource(std::string file_path){
		Response* resp = new Response();

		struct stat results;
		size_t size = 0;
    
	    if(stat(file_path.c_str(), &results) == 0){
	        size = results.st_size;

	        std::cout << "FILE STAT SIZE = " << size << "\n";

	        // Open the file in binary mode
	        // Note: we don't use ifstreams because of a g++ bug with ios::binary
	        std::fstream myFile;
		    myFile.open(file_path, std::ios::in | std::ios::binary);

		    if(myFile){

		    	// Read file
		    	char* buffer = new char[size];
		    	myFile.read(buffer, size);
		    	resp->setContent(buffer, size);

		    	// Set correct MIME type
		    	try{
		    		std::string mime = getMIMEByFilename(file_path);
		    		

		    		// Unless it's text, set mode as binary
		    		if(!isTextMIME(mime)){
		    			resp->setBinaryMode(true);
		    			resp->addHeader("Content-Type", mime);
		    		}else{
		    			// @TODO charset
		    			resp->addHeader("Content-Type", mime+"; charset=utf-8");
		    		}

		    	}catch(Ex_no_mime_type& e){
		    		std::cout << "MIME type not found for file served: '" << file_path << "'" << std::endl;
		    	}catch(Ex_invalid_filename& e){
		    		std::cout << "Couldn't find MIME type for invalid filename '" << file_path << "'" << std::endl;
		    	}catch(std::exception& e){
		    		std::cout << "Couldn't find MIME type for file '" << file_path << "', unknown error occurred" << std::endl;
		    	}
		    }else{
		    	std::cout << "(404) File couldn't be opened: '" << file_path << "'" << std::endl;
		    	// @TODO File not found (404)
		    }

	    }else{
	    	std::cout << "(404) Resource file not found: '" << file_path << "'" << std::endl;
	        // @TODO File not found (404)
		}

		return resp;
	}

	/**
	* Sends a response to the client
	* @param response object
	* @param mongoose connnection struct
	*/
	void Server::sendResponse(Response* resp, struct mg_connection *conn){

		// Content-length
		if(!resp->hasHeader("Content-Length")){
			resp->addHeader("Content-Length", resp->getContentByteSizeStr());
		}

		// Send headers
		std::queue<Header*> headers = resp->getHeaderQueue();

		if(headers.size() > 0){
			while(!headers.empty()){
				Header* h = headers.front();
				headers.pop();
				// Send header
				mg_send_header(conn, h->getName().c_str(), h->getValue().c_str());
			}
		}

		// @TODO - Temporary header to prevent browser caching
		resp->addHeader("Cache-Control", "max-age=0, post-check=0, pre-check=0, no-store, no-cache, must-revalidate");

		if(resp->isBinary()){
			//mg_send_data(conn, resp->getContent(), resp->getContentLen());
			mg_write(conn, resp->getContent(), resp->getContentLen());
		}else{
			mg_printf_data(conn, "%s", resp->getContent());
		}
	}

	/* ======================================================== */
	/* Server MISC												*/
	/* ======================================================== */

	/**
	* Sets the server's cache size. If it is set at a smaller number than
	* the current usage, the cached files will not be affected.
	*/
	void Server::setCacheSize(size_t size){
		max_cache_size = size;
	}

	/**
	* Makes Swift verbose
	*/
	void Server::setVerbose(bool){
		verbose = true;
	}

	/**
	* Prints welcome message
	*/
	void Server::printWelcome(){
		std::cout << _SWIFT_WELCOME << " " << _SWIFT_VERSION << std::endl;
	}

	/**
	* Converts a string to a method enum
	*/
	Method str_to_method(std::string request_method){
		if(request_method == "GET") return Method::GET;
		else if(request_method == "HEAD") return Method::HEAD;
		else if(request_method == "POST") return Method::POST;
		else if(request_method == "PUT") return Method::PUT;
		else if(request_method == "DELETE") return Method::DELETE;
		else if(request_method == "TRACE") return Method::TRACE;
		else if(request_method == "OPTIONS") return Method::OPTIONS;
		else if(request_method == "CONNECT") return Method::CONNECT;
		else if(request_method == "PATCH") return Method::PATCH;
		else throw ex_invalid_method;
	}
	
	/* ======================================================== */
	/* API Hook													*/
	/* ======================================================== */

	/**
	* Constructs an API Hook with default settings
	*/
	Hook::Hook(){
		is_resource = false;
		preload_resource = false;
		setCharset(Charset::getDefault());
	}

	/**
	* Constructs an API Hook with a callback function
	* @param request path string
	* @param callback function pointer
	*/
	Hook::Hook(std::string request_path, Response* (*function)(Request*)){
		is_resource = false;
		preload_resource = false;
		this->request_path = request_path;
		this->callback_function = function;
		setCharset(Charset::getDefault());
	}

	/**
	* Constructs an API Hook that serves a static resource
	* @param request path string
	* @param resource path string
	*/
	Hook::Hook(std::string request_path, std::string resource_path){
		is_resource = true;
		preload_resource = true;
		this->request_path = request_path;
		this->resource_path = resource_path;
		setCharset(Charset::getDefault());
	}

	/**
	* Sets the API Hook's request path
	* @param path
	*/
	void Hook::setRequestPath(std::string path){
		request_path = path;
	}

	/**
	* Returns the API Hook's request path
	* @return path string
	*/
	std::string Hook::getRequestPath(){
		return request_path;
	}

	/**
	* Adds given method to the allowed list
	* @param Method enum
	*/
	void Hook::allowMethod(Method m){
		allowed_methods.insert(m);
	}

	/**
	* Disallows the given method
	* @param Method enum
	*/
	void Hook::disallowMethod(Method m){
		allowed_methods.erase(m);
	}

	/**
	* Indicates whether the given method is allowed by this API Hook
	* @param Method enum: POST, GET...
	* @return boolean
	*/
	bool Hook::isMethodAllowed(Method m){
		return allowed_methods.count(m) > 0;
	}

	/**
	* Sets the static resource path for this API Hook and make it a static resource
	* @param path to resource
	*/
	void Hook::setResourcePath(std::string path){
		is_resource = true;
		resource_path = path;
	}

	/**
	* Returns the resource path for this API Hook, if it is a static resource
	*/
	std::string Hook::getResourcePath(){
		return resource_path;
	}

	/**
	* Sets this API Hook to preload static resources or not
	* @param boolean
	*/
	void Hook::setPreloadResource(bool preload){
		preload_resource = preload;
	}

	/**
	* Sets this API Hook as a static resource or not
	* @param boolean
	*/
	void Hook::setIsResource(bool resource){
		is_resource = resource;
	}

	/**
	* Indicates if this API Hook represents a static resource
	* @return boolean
	*/
	bool Hook::isResource(){
		return is_resource;
	}

	/**
	* Returns the charset for this API Hook's resource
	* @return string
	*/
	std::string Hook::getCharset(){
		return charset;
	}

	/**
	* Sets the charset for this resource
	* @param charset string
	*/
	void Hook::setCharset(std::string charset){
		this->charset = charset;
	}

	/**
	* Sets the callback associated with this API Hook
	* @param void function
	*/
	void Hook::setCallback(Response* (*function)(Request*)){
		callback_function = function;
	}

	/**
	* Calls the callback function associated with this API Hook
	* @param Request object
	*/
	Response* Hook::getCallbackResponse(Request* req){
		Response* resp = callback_function(req);
		return resp;
	}

	/* ======================================================== */
	/* Request													*/
	/* ======================================================== */

	/**
	* Constructs a blank request object
	*/
	Request::Request(){}

	/**
	* Constructs a Request object from a Mongoose connection object
	*/
	Request::Request(struct mg_connection* conn){

		// Null pointer?
		if(conn == nullptr) throw ex_null_request;
		
		// Copy all data from the Mongoose connection struct
		if(conn->request_method != nullptr){
			request_method_str = std::string(conn->request_method);
			// Parse the method
			request_method = str_to_method(request_method_str);
		}else{
			throw ex_invalid_method;
		}
		
		if(conn->uri != nullptr){
			uri = std::string(conn->uri);
		}else{
			throw ex_null_uri;
		}
		
		if(conn->http_version != nullptr){
			http_version = std::string(conn->http_version);
		}else{
			throw ex_null_http_version;
		}
		
		if(conn->query_string != nullptr){
			query_string = std::string(conn->query_string);
		}else{
			query_string = "";
		}

		// IP Addresses
		if(conn->remote_ip != nullptr) remote_ip = std::string(conn->remote_ip);
		if(conn->local_ip != nullptr) local_ip = std::string(conn->local_ip);
		
		// Ports
		remote_port = conn->remote_port;
		local_port = conn->local_port;

		// Headers
		// @TODO

		// Contents
		if(conn->content != nullptr) content = std::string(conn->content);
		else content = "";
		content_len = conn->content_len;

	}

	Method Request::getMethod(){
		return request_method;
	}

	std::string Request::getMethodStr(){
		return request_method_str;
	}

	std::string Request::getURI(){
		return uri;
	}

	std::string Request::getHTTPVersion(){
		return http_version;
	}

	std::string Request::getQueryString(){
		return query_string;
	}

	std::string Request::getRemoteIP(){
		return remote_ip;
	}

	std::string Request::getLocalIP(){
		return local_ip;
	}

	unsigned short Request::getRemotePort(){
		return remote_port;
	}

	unsigned short Request::getLocalPort(){
		return local_port;
	}

	std::string Request::getContent(){
		return content;
	}

	size_t Request::getContentLen(){
		return content_len;
	}

	int Request::getHeaderCount(){
		return headers.size();
	}

	/* ======================================================== */
	/* Response													*/
	/* ======================================================== */

	/**
	* Constructs a blank Response object
	*/
	Response::Response(){}

	/**
	* Adds a new header object to the response
	* @param header object
	*/
	void Response::addHeader(Header* header){
		// Add to header queue
		headers.push(header);
		// Keep track of lowercase header names used so far
		std::string name = header->getName();
		std::transform(name.begin(), name.end(), name.begin(), ::tolower);
		header_names.insert(name);
	}

	/**
	* Creates a new header and adds it to the response
	* @param header name
	* @param header value
	*/
	void Response::addHeader(std::string name, std::string value){
		// Add to header queue
		Header* header = new Header(name, value);
		headers.push(header);
		// Keep track of lowercase header names used so far
		std::transform(name.begin(), name.end(), name.begin(), ::tolower);
		header_names.insert(name);
	}

	/**
	* Indicates whether the response already has a header with given name
	* @param header name
	* @return boolean
	*/
	bool Response::hasHeader(std::string name){
		return header_names.count(name) > 0;
	}

	/**
	* Returns the number of headers
	* @return integer
	*/
	int Response::getHeaderCount(){
		return headers.size();
	}

	/**
	* Sets the content of the response object
	* @param content string
	* @param char length of content
	*/
	void Response::setContent(char* content, int length){
		this->content_len = length;
		this->content = new char[length];
		strcpy(this->content, content);
	}

	/**
	* Returns the response's content
	* @return char*
	*/
	char* Response::getContent(){
		return content;
	}

	/**
	* Returns the lenght of the content
	* @return size_t
	*/
	int Response::getContentLen(){
		return content_len;
	}

	/**
	* Returns the charset for this response (used for text content)
	* @return string
	*/
	std::string Response::getCharset(){
		return charset;
	}

	/**
	* Sets the charset for this response (used for text content)
	* @param charset string
	*/
	void Response::setCharset(std::string charset){
		this->charset = charset;
	}

	/**
	* Returns the byte size of the content
	* @return size_t
	*/
	size_t Response::getContentByteSize(){
		return content_len;
	}

	/**
	* Returns the byte size of the content as a string
	* @return string
	*/
	std::string Response::getContentByteSizeStr(){
		std::stringstream ss;
		ss << getContentByteSize();

		std::cout << "SIZE WAS " << ss.str() << "\n";

		return ss.str();
	}

	/**
	* Makes this Response a binary one or not
	* @param boolean
	*/
	void Response::setBinaryMode(bool binary){
		binary_mode = binary;
	}

	/**
	* Indicates whether this response operates in binary mode
	* @return boolean
	*/
	bool Response::isBinary(){
		return binary_mode;
	}

	/**
	* Returns the queue of headers, in order
	* @return header queue
	*/
	std::queue<Header*> Response::getHeaderQueue(){
		return headers;
	}

	/* ======================================================== */
	/* Header													*/
	/* ======================================================== */

	/**
	* Constructs a blank Header object
	*/
	Header::Header(){}

	/**
	* Constructs a new Header object
	* @param header name
	* @param header value
	*/
	Header::Header(std::string name, std::string value){
		this->name = name;
		this->value = value;
	}

	/**
	* Returns the header's name
	* @return string
	*/
	std::string Header::getName(){
		return name;
	}

	/**
	* Returns the header's value
	* @return string
	*/
	std::string Header::getValue(){
		return value;
	}

	/* ======================================================== */
	/* Utility													*/
	/* ======================================================== */

	// trim from start
	static inline std::string &ltrim(std::string &s) {
		s.erase(s.begin(), std::find_if(s.begin(), s.end(), std::not1(std::ptr_fun<int, int>(std::isspace))));
		return s;
	}

	// trim from end
	static inline std::string &rtrim(std::string &s) {
		s.erase(std::find_if(s.rbegin(), s.rend(), std::not1(std::ptr_fun<int, int>(std::isspace))).base(), s.end());
		return s;
	}

	// trim from both ends
	static inline std::string &trim(std::string &s) {
		return ltrim(rtrim(s));
	}

	/* ======================================================== */
	/* MIME														*/
	/* ======================================================== */

	void loadMIME(std::string file_path){
		std::ifstream myFile(file_path);
		std::string line;
		if(myFile){
			while(std::getline(myFile, line)){
			    std::istringstream iss(line);

			    if(line.length() >= 3){
				    // Tokenize the line
				    std::vector<std::string> tokens{
				    	std::istream_iterator<std::string>{iss}, 
				    	std::istream_iterator<std::string>{}};
					
					int ctok = 0;
					std::string ctype;
				    for(std::string token: tokens){
				    	if(token.at(0) == '#') break; // it's a comment, break now
				    	else if(ctok==0) ctype = trim(token); // first arg is the mime type
				    	else mimetypes[trim(token)] = ctype; // other args are extensions
				    	++ctok;
				    }
				}
			}
		}else{
			throw ex_mime_types_file_not_found;
		}
	}

	/**
	* Returns the MIME type for given file extension
	* @param file extension string
	* @return MIME type string
	*/
	std::string getMIMEByExtension(std::string file_extension){
		// Lower case the extension
		std::transform(file_extension.begin(), file_extension.end(), file_extension.begin(), ::tolower);
		file_extension = trim(file_extension);

		if(
			file_extension.length() > 0 && 
			mimetypes.count(file_extension) > 0
		){
			return mimetypes.at(file_extension);
		}else{
			throw ex_no_mime_type;
		}
	}

	/**
	* Returns the MIME type for given filename
	* @param filename string
	* @param MIME type string
	*/
	std::string getMIMEByFilename(std::string filename){
		if(
			filename.length() >= 3 && 
			filename.find('.') != std::string::npos
		){
			// Find the extension
			std::string extension = filename.substr(filename.find_last_of(".") + 1);

			if(extension.length() > 0){
				return getMIMEByExtension(extension);
			}else{
				throw ex_invalid_filename;
			}
		}else{
			throw ex_invalid_filename;
		}
	}

	/**
	* Indicates whether the given MIME string is of Text type
	* @param MIME string
	* @return boolean
	*/
	bool isTextMIME(std::string MIME){
		return MIME.length() >= 5 && MIME.find("text/") == 0;
	}

}
