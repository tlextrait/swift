/**
* SWIFT
* Copyright (c) 2014 Thomas Lextrait <thomas.lextrait@gmail.com>
* All rights reserved
*/

#include <stdio.h>
#include <iostream>
#include <string>

#include "swift.h"

namespace swift{

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
		mgserver = mg_create_server(NULL, this->requestHandler);

		// Set the port
		mg_set_option(mgserver, "listening_port", str_port);

		// Display info
		printWelcome();
		std::cout << "Listening on port " << mg_get_option(mgserver, "listening_port") << "..." << std::endl;

		for(;;){
		    mg_poll_server(mgserver, 1000);
		}
	}

	/* ======================================================== */
	/* Private													*/
	/* ======================================================== */

	/**
	* Handles requests
	* @param mongoose connection oject
	* @param mongoose event enum
	*/
	int Server::requestHandler(struct mg_connection *conn, enum mg_event ev){

		int result = MG_FALSE;

		if(ev == MG_REQUEST){

			// Build Swift Request
			Request* req = new Request(conn);

			// Show we received a request
			std::cout << _SWIFT_SYMB_REQ << " " << req->getURI() << " from " << req->getRemoteIP() << std::endl;

			// Process it
			processRequest(req, conn);

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
		mg_printf_data(conn, "Hello! Requested URI is [%s]", conn->uri);
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
