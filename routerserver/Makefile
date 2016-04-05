
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

CXXFLAGS=-g -std=c++11 -Wno-invalid-offsetof -DUSE_LOG4CPP -DLINUX #-D_DEBUG_ #-D_MEM_CHECK #--extra-libs=-lgcc

LIBS=-L/usr/local/lib/log4cpp -llog4cpp -L./$(LIBPATH)/tinyxml -ltinyxml -lpthread -static
INCPATH=-I./$(BASEPATH) -I/usr/local/include/log4cpp -I./$(LIBPATH)/tinyxml -I./$(COMMANDPATH)

DIRS=./log ./bin ./debug

TARGET=./bin/routersvrd

OBJECTS=./debug/RouterMain.o \
		./debug/RouterMaster.o \
		./debug/RouterWorker.o \
		./debug/RouterConfig.o \
		./debug/RouterServer.o \
		./debug/RouterServerTask.o \
		./debug/RouterChannelTask.o \
		./debug/SvrQos.o \
		./debug/wMemPool.o \
		./debug/wSignal.o \
		./debug/wWorker.o \
		./debug/wProcTitle.o \
		./debug/wHttpTask.o  \
		./debug/wTcpTask.o  \
		./debug/wChannel.o \
		./debug/wChannelTask.o \
		./debug/wTask.o \
		./debug/wIO.o \
		./debug/wFile.o \
		./debug/wMsgQueue.o \
		./debug/wLog.o \
		./debug/wMisc.o \
		./debug/wSocket.o \
		./debug/wHttp.o \
		./debug/wShmtx.o \
		./debug/wShm.o

$(TARGET): $(OBJECTS)
	$(CXX) $(OBJECTS) -o $(TARGET) $(LIBS)
	#cp $(TARGET) ../routerserver/bin -f

all: $(TARGET)

./debug/RouterMain.o: src/RouterMain.cpp
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $@ $<

./debug/RouterMaster.o: src/RouterMaster.cpp
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $@ $<

./debug/RouterWorker.o: src/RouterWorker.cpp
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $@ $<

./debug/RouterConfig.o: src/RouterConfig.cpp
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $@ $<

./debug/RouterServer.o: src/RouterServer.cpp
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $@ $<

./debug/RouterServerTask.o: src/RouterServerTask.cpp
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $@ $<

./debug/RouterChannelTask.o: src/RouterChannelTask.cpp
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $@ $<

./debug/SvrQos.o: src/SvrQos.cpp
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

./debug/wHttpTask.o: $(BASEPATH)/wHttpTask.cpp
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $@ $<

./debug/wSocket.o: $(BASEPATH)/wSocket.cpp
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $@ $<

./debug/wHttp.o: $(BASEPATH)/wHttp.cpp
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $@ $<

./debug/wSignal.o: $(BASEPATH)/wSignal.cpp
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $@ $<

./debug/wMemPool.o: $(BASEPATH)/wMemPool.cpp
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
