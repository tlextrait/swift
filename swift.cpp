/**
* SWIFT
* Copyright (c) 2014 Thomas Lextrait <thomas.lextrait@gmail.com>
* All rights reserved
*/

#include <stdio.h>
#include <iostream>
#include <string>
#include <map>
#include <utility> // make_pair
#include <vector>

#include "swift.h"

namespace swift{

	// Maps server id's to swift servers
	std::map<int,Server*> server_map;

	// Invalid method exception
	class invalid_method_exception: public std::exception{
	  	virtual const char* what() const throw(){
	    	return "Invalid request method";
	  	}
	} ex_invalid_method;

	class null_request_exception: public std::exception{
		virtual const char* what() const throw(){
	    	return "Null request";
	  	}
	} ex_null_request;

	class null_uri: public std::exception{
		virtual const char* what() const throw(){
	    	return "Null URI";
	  	}
	} ex_null_uri;

	class null_http_version: public std::exception{
		virtual const char* what() const throw(){
	    	return "Null HTTP version";
	  	}
	} ex_null_http_version;

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

	/* ======================================================== */
	/* Server loading											*/
	/* ======================================================== */

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

			result = MG_TRUE;

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

				}else{
					if(server->verbose) std::cout << "Access denied" << std::endl;
					/*
					@TODO ACCESS DENIED
					*/
				}

			}else{
				if(server->verbose) std::cout << "No matching endpoint" << std::endl;
				/*
				@TODO 404 !!!
				*/
			}

			mg_printf_data(conn, "Hello! Requested URI is [%s]", conn->uri);
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

		// Add the endpoint
		addEndpoint(request_path, hook);
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

	/* ======================================================== */
	/* MISC														*/
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

	Hook::Hook(){
		is_resource = false;
		preload_resource = false;
	}

	Hook::Hook(std::string request_path, void* callback_function){
		is_resource = false;
		preload_resource = false;
		this->request_path = request_path;
		this->callback_function = callback_function;
	}

	Hook::Hook(std::string request_path, std::string resource_path){
		is_resource = true;
		preload_resource = true;
		this->request_path = request_path;
		this->resource_path = resource_path;
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
	* Sets the callback associated with this API Hook
	* @param void function
	*/
	void Hook::setCallback(void* function){
		callback_function = function;
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
		num_headers = conn->num_headers;

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

}
