
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

CXXFLAGS=-g #--extra-libs=-lgcc -DUSE_LOG4CPP -DLINUX #-D_DEBUG_

LIBS=-L./usr/local/lib -llog4cpp -L./../lib/tinyxml -ltinyxml -lpthread -static
INCPATH=-I./$(BASEPATH) -I./usr/local/include/log4cpp -I./../lib/tinyxml

TARGET=./bin/agentsvrd
OBJECTS=./debug/AgentMain.o ./debug/AgentConfig.o  ./debug/AgentServer.o  ./debug/AgentServerTask.o  ./debug/wLog.o  ./debug/wFunction.o  ./debug/wMisc.o  ./debug/wTcpTask.o  ./debug/wSocket.o

$(TARGET): $(OBJECTS)
	$(CXX) $(OBJECTS) -o $(TARGET) $(LIBS)

all: $(TARGET)

./debug/AgentMain.o: src/AgentMain.cpp
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $@ $<

./debug/AgentConfig.o: src/AgentConfig.cpp
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $@ $<

./debug/AgentServer.o: src/AgentServer.cpp
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $@ $<

./debug/AgentServerTask.o: src/AgentServerTask.cpp
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $@ $<

./debug/wLog.o: $(BASEPATH)/wLog.cpp
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $@ $<

./debug/wFunction.o: $(BASEPATH)/wFunction.cpp
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $@ $<

./debug/wMisc.o: $(BASEPATH)/wMisc.cpp
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $@ $<

./debug/wTcpTask.o: $(BASEPATH)/wTcpTask.cpp
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $@ $<

./debug/wSocket.o: $(BASEPATH)/wSocket.cpp
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $@ $<

clean:
	-rm -f $(TARGET) $(OBJECTS)
