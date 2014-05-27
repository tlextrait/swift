#
#
#

CXX = g++
FLAGS = -c -Wall
LIBS = -L/usr/local/lib -L/opt/local/lib -lboost_system -lcrypto -lssl -lpthread
INCLUDES = -I/opt/local/include/

all: w

w: webemHello.o
	$(CXX) webemHello.o -o w

webemHello.o: webemHello.cpp tchar.h targetver.h stdafx.h
	$(CXX) $(FLAGS) $(LIBS) $(INCLUDES) webemHello.cpp

clean:
	rm -f *.o w

