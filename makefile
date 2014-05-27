#
#
#

CXX = g++
FLAGS = -c -Wall
#LIBS = -L/usr/local/lib -L/opt/local/lib -lboost_system -lcrypto -lssl -lpthread
#INCLUDES = -I/opt/local/include/

all: webapp

webapp: app.o swift.o
	$(CXX) app.o swift.o -o webapp

app.o: app.cpp app.h swift.h
	$(CXX) $(FLAGS) app.cpp swift.cpp

swift.o: swift.cpp swift.h
	$(CXX) $(FLAGS) swift.cpp

clean:
	rm -f *.o w

