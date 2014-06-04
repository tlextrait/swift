/**
* SWIFT
* Copyright (c) 2014 Thomas Lextrait <thomas.lextrait@gmail.com>
* All rights reserved
*/

#include <iostream>

#include "app.h"

int main(){

	swift::Server* server = swift::Server::newServer();

	server->addResource("/readme","README.md");
	server->addResource("/index.html","resources/index.html");
	server->addResource("/js/jquery.min.js","resources/js/jquery.min.js");
	server->addResource("/js/winedex.js","resources/js/winedex.js");
	server->addResource("/css/main.css","css/main.css");

	server->Start();

	return 0;
}
