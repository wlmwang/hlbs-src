
###############################
# Copyright (C) Anny.
# Copyright (C) Disvr, Inc.
###############################

#
# Makefile
# 需要预先编译lib目录下的log4cpp|tinyxml|readline软件包
# 
# redhat
# $sudo apt-get install libreadline5-dev
# 
# centos
# $yum -y install readline-devel
# $yum -y install patch
#

BASEPATH=../base

LIBPATH=../lib

COMMANDPATH=../command

CXX=g++ -std=c++11

CXXFLAGS=-g -DUSE_LOG4CPP -DLINUX -D__READLINE_DEBUG#--extra-libs=-lgcc #-D_DEBUG_

LIBS=-L./$(LIBPATH)/log4cpp/lib -llog4cpp -L./$(LIBPATH)/tinyxml -ltinyxml -lpthread -lreadline -lncurses -static
INCPATH=-I./$(BASEPATH) -I./$(LIBPATH)/log4cpp/include -I./$(LIBPATH)/tinyxml -I./$(COMMANDPATH)

TARGET=./bin/agentcmd
OBJECTS=./debug/AgentCmdMain.o ./debug/AgentCmdConfig.o  ./debug/AgentCmd.o  ./debug/AgentCmdTask.o  ./debug/wLog.o ./debug/wMisc.o  ./debug/wTcpTask.o  ./debug/wSocket.o ./debug/wReadline.o

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

./debug/wReadline.o: $(BASEPATH)/wReadline.cpp
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $@ $<

clean:
	-rm -f $(TARGET) $(OBJECTS)
