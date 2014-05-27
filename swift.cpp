/**
* SWIFT
* Copyright (c) 2014 Thomas Lextrait <thomas.lextrait@gmail.com>
* All rights reserved
*/

#include "swift.h"
#include "mongoose/mongoose.h"

/**
* Swift constructor
*/
Swift::Swift(){
	server_count = 0;
}

/**
* Swift destructor
*/
Swift::~Swift(){
	// destroy each mongoose server thread
	for(int i=0; i<server_count; i++){
		mg_destroy_server(&servers[i]);
	}
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

}
