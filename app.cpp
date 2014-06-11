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

	server->addResource("/","resources/index.html");
	server->addResource("/index.html","resources/index.html");
	server->addResource("/js/jquery.min.js","resources/js/jquery.min.js");
	server->addResource("/js/winedex.js","resources/js/winedex.js");
	server->addResource("/css/main.css","resources/css/main.css");
	server->addResource("/images/wine_glass.png","resources/images/wine_glass.png");
	server->addResource("/images/red_wine.png","resources/images/red_wine.png");
	server->addResource("/images/white_wine.png","resources/images/white_wine.png");
	server->addResource("/css/fonts.css","resources/css/fonts.css");
	server->addResource("/fonts/avenir.woff","resources/fonts/avenir.woff");
	server->addResource("/fonts/avenir.ttf","resources/fonts/avenir.ttf");
	server->addResource("/fonts/avenir.svg","resources/fonts/avenir.svg");

	server->Start();

	return 0;
}
