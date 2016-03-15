
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

TARGET=./bin/agentsvrd

OBJECTS=./debug/AgentMain.o \
		./debug/AgentMaster.o \
		./debug/AgentConfig.o \
		./debug/AgentServer.o \
		./debug/AgentServerTask.o \
		./debug/AgentClientTask.o \
		./debug/wSignal.o \
		./debug/wWorker.o \
		./debug/wProcTitle.o \
		./debug/wTcpTask.o \
		./debug/wChannel.o \
		./debug/wChannelTask.o \
		./debug/wTask.o \
		./debug/wIO.o \
		./debug/wFile.o \
		./debug/wMsgQueue.o \
		./debug/wLog.o \
		./debug/wMisc.o \
		./debug/wSocket.o \
		./debug/wShmtx.o \
		./debug/wShm.o

$(TARGET): $(OBJECTS)
	$(CXX) $(OBJECTS) -o $(TARGET) $(LIBS)

all: $(TARGET)

./debug/AgentMain.o: src/AgentMain.cpp
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $@ $<

./debug/AgentMaster.o: src/AgentMaster.cpp
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $@ $<

./debug/AgentConfig.o: src/AgentConfig.cpp
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $@ $<

./debug/AgentServer.o: src/AgentServer.cpp
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $@ $<

./debug/AgentServerTask.o: src/AgentServerTask.cpp
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $@ $<

./debug/AgentClientTask.o: src/AgentClientTask.cpp
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $@ $<

./debug/wWorker.o: $(BASEPATH)/wWorker.cpp
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $@ $<

./debug/wProcTitle.o: $(BASEPATH)/wProcTitle.cpp
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $@ $<

./debug/wChannel.o: $(BASEPATH)/wChannel.cpp
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $@ $<

./debug/wFile.o: $(BASEPATH)/wFile.cpp
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $@ $<

./debug/wIO.o: $(BASEPATH)/wIO.cpp
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $@ $<

./debug/wTask.o: $(BASEPATH)/wTask.cpp
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $@ $<

./debug/wChannelTask.o: $(BASEPATH)/wChannelTask.cpp
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $@ $<

./debug/wMsgQueue.o: $(BASEPATH)/wMsgQueue.cpp
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $@ $<

./debug/wLog.o: $(BASEPATH)/wLog.cpp
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $@ $<

./debug/wMisc.o: $(BASEPATH)/wMisc.cpp
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $@ $<

./debug/wTcpTask.o: $(BASEPATH)/wTcpTask.cpp
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $@ $<

./debug/wSocket.o: $(BASEPATH)/wSocket.cpp
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $@ $<

./debug/wSignal.o: $(BASEPATH)/wSignal.cpp
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $@ $<

./debug/wShmtx.o: $(BASEPATH)/wShmtx.cpp
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $@ $<

./debug/wShm.o: $(BASEPATH)/wShm.cpp
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $@ $<

clean:
	#-rm -f $(TARGET) $(OBJECTS)
	-rm -rf $(DIRS)
	
dir:
	-mkdir $(DIRS)
	chmod 755 -R $(DIRS)