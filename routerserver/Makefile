
###############################
# Copyright (C) Anny.
# Copyright (C) Disvr, Inc.
###############################

#
# Makefile
# 需要预先编译lib目录下的log4cpp|tinyxml软件包
#
 
BASEPATH=../base

LIBPATH=../lib

COMMANDPATH=../command

CXX=g++

CXXFLAGS=-g -std=c++11 -Wno-invalid-offsetof -DUSE_LOG4CPP -DLINUX #--extra-libs=-lgcc #-D_DEBUG_

LIBS=-L./$(LIBPATH)/log4cpp/lib -llog4cpp -L./$(LIBPATH)/tinyxml -ltinyxml -lpthread -static
INCPATH=-I./$(BASEPATH) -I./$(LIBPATH)/log4cpp/include -I./$(LIBPATH)/tinyxml -I./$(COMMANDPATH)

DIRS=./log ./bin ./debug
TARGET=./bin/routersvrd
OBJECTS=./debug/RouterMain.o ./debug/RouterConfig.o  ./debug/RouterServer.o  ./debug/RouterServerTask.o  ./debug/wLog.o ./debug/wMisc.o  ./debug/wTcpTask.o  ./debug/wSocket.o

$(TARGET): $(OBJECTS)
	$(CXX) $(OBJECTS) -o $(TARGET) $(LIBS)
	#cp $(TARGET) ../routerserver/bin -f

all: $(TARGET)

./debug/RouterMain.o: src/RouterMain.cpp
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $@ $<

./debug/RouterConfig.o: src/RouterConfig.cpp
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $@ $<

./debug/RouterServer.o: src/RouterServer.cpp
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $@ $<

./debug/RouterServerTask.o: src/RouterServerTask.cpp
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $@ $<

./debug/wLog.o: $(BASEPATH)/wLog.cpp
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $@ $<

./debug/wMisc.o: $(BASEPATH)/wMisc.cpp
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $@ $<

./debug/wTcpTask.o: $(BASEPATH)/wTcpTask.cpp
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $@ $<

./debug/wSocket.o: $(BASEPATH)/wSocket.cpp
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $@ $<

clean:
	#-rm -f $(TARGET) $(OBJECTS)
	-rm -rf $(DIRS)

dir:
	-mkdir $(DIRS)
	chmod 755 -R $(DIRS)
