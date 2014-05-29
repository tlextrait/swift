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

		// Build Swift Request
		Request* req = new Request();

		// Show we received a request
		std::cout << _SWIFT_SYMB_REQ << " " << "request\n";

		int result = MG_FALSE;

		if (ev == MG_REQUEST) {
			mg_printf_data(conn, "Hello! Requested URI is [%s]", conn->uri);
			result = MG_TRUE;
		} else if (ev == MG_AUTH) {
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

	/* ======================================================== */
	/* Request											*/
	/* ======================================================== */

	Request::Request(){
		
	}

	/**
	* Constructs a SwiftRequest from a Mongoose connection
	*/
	Request::Request(struct mg_connection* conn){
		// Copy all data from the Mongoose connection struct
		request_method = std::string(conn->request_method);
		uri = std::string(conn->uri);
		http_version = std::string(conn->http_version);
		query_string = std::string(conn->query_string);

		// Ports
		remote_port = conn->remote_port;
		local_port = conn->local_port;

		// Headers
		num_headers = conn->num_headers;

		// Contents
		content = std::string(conn->content);
		content_len = conn->content_len;
	}

}
