/**
* SWIFT
* Copyright (c) 2014 Thomas Lextrait <thomas.lextrait@gmail.com>
* All rights reserved
*/

#include <iostream>

#include "app.h"

int main(){

	Swift* server = Swift::newServer();
	server->Start();

	printf("started\n");

	return 0;
}
