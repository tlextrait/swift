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

	/**
	* Swift constructor
	*/
	Server::Server(){
		// Settings
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

		mg_set_option(mgserver, "document_root", "resources");

		// Display info
		printWelcome();
		std::cout << "Listening on port " << mg_get_option(mgserver, "listening_port") << "...\n";

		for(;;){
		    mg_poll_server(mgserver, 1000);
		}
	}

	/* ======================================================== */
	/* Private													*/
	/* ======================================================== */

	/**
	* Handles requests
	*/
	int Server::requestHandler(struct mg_connection *conn, enum mg_event ev){

		int result = MG_FALSE;

		if(ev == MG_REQUEST){

			// Build Swift Request
			Request* req = new Request(conn);

			// Show we received a request
			std::cout << _SWIFT_SYMB_REQ << " " << req->getURI() << " from " << req->getRemoteIP();

			mg_printf_data(conn, "Hello! Requested URI is [%s]", conn->uri);
			result = MG_TRUE;

		}else if(ev == MG_AUTH){
			result = MG_TRUE;
		}

		return result;
	}

	/* ======================================================== */
	/* MISC														*/
	/* ======================================================== */

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
		std::cout << _SWIFT_WELCOME << " " << _SWIFT_VERSION << "\n";
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

	Request::Request(){
		
	}

	/**
	* Constructs a SwiftRequest from a Mongoose connection
	*/
	Request::Request(struct mg_connection* conn){
		// Null pointer?
		if(conn == nullptr) throw ex_null_request;

		// Copy all data from the Mongoose connection struct
		request_method_str = std::string(conn->request_method);
		uri = std::string(conn->uri);
		http_version = std::string(conn->http_version);
		query_string = std::string(conn->query_string);

		// Parse method
		request_method = str_to_method(request_method_str);

		// IP Addresses
		remote_ip = std::string(conn->remote_ip);
		local_ip = std::string(conn->local_ip);

		// Ports
		remote_port = conn->remote_port;
		local_port = conn->local_port;

		// Headers
		num_headers = conn->num_headers;

		// Contents
		content = std::string(conn->content);
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
