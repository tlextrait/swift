/**
* SWIFT
* Copyright (c) 2014 Thomas Lextrait <thomas.lextrait@gmail.com>
* All rights reserved
*/

#include "mongoose.h"

#define _SWIFT_MAX_SERVER_THREADS 255
#define _SWIFT_DEFAULT_PORT 5555

class Swift {

		// Mongoose server
		struct mg_server* mgserver;

		// Various settings
		bool verbose;

	public:
		// Constructor/destructor
		Swift();
		~Swift();

		// Server loading
		static Swift* newServer();
		void Start();
		void Start(int port);

		// Setters/getters
		// void setVerbose(bool)

		// API
		// void addResource();

	private:
		// Master event handler
		static int eventHandler(struct mg_connection *conn, enum mg_event ev);
};
