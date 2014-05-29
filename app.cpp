/**
* SWIFT
* Copyright (c) 2014 Thomas Lextrait <thomas.lextrait@gmail.com>
* All rights reserved
*/

#include <iostream>

#include "app.h"

int main(){

	swift::Server* server = swift::Server::newServer();
	server->Start();

	return 0;
}
