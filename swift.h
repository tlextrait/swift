/**
* SWIFT
* Copyright (c) 2014 Thomas Lextrait <thomas.lextrait@gmail.com>
* All rights reserved
*/

#include "mongoose/mongoose.h"

#define _SWIFT_MAX_SERVER_THREADS 255
#define _SWIFT_DEFAULT_PORT 5555

class Swift {

		struct mg_server* servers[_SWIFT_MAX_SERVER_THREADS];
		int server_count;

	public:
		// constructor/destructor
		Swift();
		~Swift();

		// server starting
		static Swift* newServer();
		void Start();
		void Start(int port);

		// API
		// void addResource();
};
