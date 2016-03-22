
CC=gcc
CXX=g++

LIBPATH=/usr/local
BASEPATH=../../base
COMMANDPATH=../../command

CXXFLAGS =-g -std=c++11 -O2 -pipe -fno-ident -fPIC -shared -Wno-invalid-offsetof -DUSE_LOG4CPP -DLINUX

INCPATH=-I./$(BASEPATH) -I./$(LIBPATH)/include/log4cpp -I./$(COMMANDPATH)
LIBS=-L./$(LIBPATH)/lib -llog4cpp -lpthread -fPIC -shared

TARGET=libagent_api

OBJECTS=./debug/wMemPool.o \
		./debug/wSignal.o \
		./debug/wTcpTask.o  \
		./debug/wChannel.o \
		./debug/wChannelTask.o \
		./debug/wTask.o \
		./debug/wIO.o \
		./debug/wFile.o \
		./debug/wMsgQueue.o \
		./debug/wMisc.o \
		./debug/wSocket.o \
		./debug/wLog.o\
		./debug/wShm.o

all: $(TARGET).so

$(TARGET).so:$(OBJECTS)
	@echo "Compiling $@"
	$(CXX) $^ -o $@ $(LIBS)

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

./debug/wLog.o: $(BASEPATH)/wLog.cpp
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $@ $<

./debug/wMsgQueue.o: $(BASEPATH)/wMsgQueue.cpp
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $@ $<

./debug/wMisc.o: $(BASEPATH)/wMisc.cpp
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $@ $<

./debug/wTcpTask.o: $(BASEPATH)/wTcpTask.cpp
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $@ $<

./debug/wSocket.o: $(BASEPATH)/wSocket.cpp
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $@ $<

./debug/wSignal.o: $(BASEPATH)/wSignal.cpp
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $@ $<

./debug/wMemPool.o: $(BASEPATH)/wMemPool.cpp
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $@ $<

./debug/wShm.o: $(BASEPATH)/wShm.cpp
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $@ $<

clean:
	rm -f $(OBJECTS)

dir:
	-mkdir debug
	chmod 755 -R debug
