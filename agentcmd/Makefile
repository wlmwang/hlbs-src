
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

CXXFLAGS=-g -DUSE_LOG4CPP -DLINUX #--extra-libs=-lgcc #-D_DEBUG_

LIBS=-L./$(LIBPATH)/log4cpp/lib -llog4cpp -L./$(LIBPATH)/tinyxml -ltinyxml -lpthread -static
INCPATH=-I./$(BASEPATH) -I./$(LIBPATH)/log4cpp/include -I./$(LIBPATH)/tinyxml -I./$(COMMANDPATH)

TARGET=./bin/agentcmd
OBJECTS=./debug/AgentCmdMain.o ./debug/AgentCmdConfig.o  ./debug/AgentCmd.o  ./debug/AgentCmdTask.o  ./debug/wLog.o ./debug/wMisc.o  ./debug/wTcpTask.o  ./debug/wSocket.o

$(TARGET): $(OBJECTS)
	$(CXX) $(OBJECTS) -o $(TARGET) $(LIBS)

all: $(TARGET)

./debug/AgentCmdMain.o: src/AgentCmdMain.cpp
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $@ $<

./debug/AgentCmdConfig.o: src/AgentCmdConfig.cpp
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $@ $<

./debug/AgentCmd.o: src/AgentCmd.cpp
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $@ $<

./debug/AgentCmdTask.o: src/AgentCmdTask.cpp
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
