/**
* SWIFT
* Copyright (c) 2014 Thomas Lextrait <thomas.lextrait@gmail.com>
* All rights reserved
*/

#include <stdio.h>
#include <iostream>

#include "swift.h"

/**
* Swift constructor
*/
Swift::Swift(){
	// Settings
	verbose = true;
}

/**
* Swift destructor
*/
Swift::~Swift(){
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
Swift* Swift::newServer(){
	Swift* swift_server = new Swift();
	return swift_server;
}

/**
* Starts the server on default port
*/
void Swift::Start(){
	Start(_SWIFT_DEFAULT_PORT);
}

/**
* Starts the server on given port
* @param port
*/
void Swift::Start(int port){
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
int Swift::requestHandler(struct mg_connection *conn, enum mg_event ev){

	// Build Swift Request
	SwiftRequest* sr = new SwiftRequest();

	// Show we received a request
	cout << _SWIFT_SYMB_REQ << " " << "request\n";

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
void Swift::setVerbose(bool){
	verbose = true;
}

/**
* Prints welcome message
*/
void Swift::printWelcome(){
	std::cout << _SWIFT_WELCOME << " " << _SWIFT_VERSION << "\n";
}

/* ======================================================== */
/* Swift Request											*/
/* ======================================================== */

SwiftRequest::SwiftRequest(){
	
}

/**
* Constructs a SwiftRequest from a Mongoose connection
*/
SwiftRequest::SwiftRequest(struct mg_connection* conn){
	request_method = conn->
}
