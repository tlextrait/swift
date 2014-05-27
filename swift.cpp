/**
* SWIFT
* Copyright (c) 2014 Thomas Lextrait <thomas.lextrait@gmail.com>
* All rights reserved
*/

#include <stdio.h>
#include <iostream>

#include "swift.h"
#include "mongoose.h"

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
	mgserver = mg_create_server(NULL, this->eventHandler);

	// Set the port
	mg_set_option(mgserver, "listening_port", str_port);

	// Display info
	std::cout << "Starting on port " << mg_get_option(mgserver, "listening_port") << "\n";

	for(;;){
	    mg_poll_server(mgserver, 1000);
	}
}

/**
* Handles requests
*/
int Swift::eventHandler(struct mg_connection *conn, enum mg_event ev){
	int result = MG_FALSE;

	if (ev == MG_REQUEST) {
		mg_printf_data(conn, "Hello! Requested URI is [%s]", conn->uri);
		result = MG_TRUE;
	} else if (ev == MG_AUTH) {
		result = MG_TRUE;
	}

	return result;
}
