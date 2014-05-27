#
#
#

CXX = g++
FLAGS = -c -Wall
#LIBS = -L/usr/local/lib -L/opt/local/lib -lboost_system -lcrypto -lssl -lpthread
#INCLUDES = -I/opt/local/include/

all: webapp

webapp: app.o swift.o mongoose.o
	$(CXX) app.o swift.o mongoose.o -o webapp

app.o: app.cpp app.h swift.h mongoose.h
	$(CXX) $(FLAGS) app.cpp swift.cpp mongoose.c

swift.o: swift.cpp swift.h mongoose.h
	$(CXX) $(FLAGS) swift.cpp mongoose.c

mongoose.o: mongoose.c mongoose.h
	$(CXX) $(FLAGS) mongoose.c

clean:
	rm -f *.o webapp

