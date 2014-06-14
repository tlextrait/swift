#
# Thomas Lextrait, thomas.lextrait@gmail.com
#

CXX = g++
CXXFLAGS = -O3 -std=c++0x -pg -D_DEBUG -g -c -Wall -Wextra -pedantic-errors
LIBS = -lpthread
#LIBS = -L/usr/local/lib -L/opt/local/lib -lboost_system -lcrypto -lssl -lpthread
#INCLUDES = -I/opt/local/include/

all: webapp

webapp: app.o swift.o mongoose.o
	$(CXX) app.o swift.o mongoose.o -o webapp

app.o: app.cpp app.h swift.h mongoose.h
	$(CXX) $(CXXFLAGS) $(LIBS) app.cpp swift.cpp mongoose.c

swift.o: swift.cpp swift.h mongoose.h
	$(CXX) $(CXXFLAGS) $(LIBS) swift.cpp mongoose.c

mongoose.o: mongoose.c mongoose.h
	$(CXX) $(CXXFLAGS) $(LIBS) mongoose.c

# Compile documentation, using 'make doc'
doc: $(DOC)
	echo 'compiling doxygen'
	doxygen doc/doxygen.config

# Clean up object and compiled files
clean:
	rm -f *.o webapp

# These are not directly producing files
.PHONY: all clean doc
