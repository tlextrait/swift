/**
* SWIFT
* Copyright (c) 2014 Thomas Lextrait <thomas.lextrait@gmail.com>
* All rights reserved
*/

#include "mongoose/mongoose.h"

class Swift {
	public:
		Swift();
		~Swift();

		static Swift* newServer();
		void Start();
};
