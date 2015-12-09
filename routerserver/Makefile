
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

CXX=g++

CXXFLAGS=-g -DUSE_LOG4CPP -DLINUX #--extra-libs=-lgcc #-D_DEBUG_

LIBS=-L./$(LIBPATH)/log4cpp/lib -llog4cpp -L./../lib/tinyxml -ltinyxml -lpthread -static
INCPATH=-I./$(BASEPATH) -I./$(LIBPATH)/log4cpp/include -I./../lib/tinyxml

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
	-rm -f $(TARGET) $(OBJECTS)
