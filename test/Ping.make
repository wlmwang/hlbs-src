
###############################
# Copyright (C) Anny.
# Copyright (C) Disvr, Inc.
###############################
 
BASEPATH=../base

LIBPATH=../lib

COMMANDPATH=../command

CXX=g++

CXXFLAGS=-g -std=c++11 -Wno-invalid-offsetof -DUSE_LOG4CPP -DLINUX #-D_DEBUG_ #-D_MEM_CHECK #--extra-libs=-lgcc

LIBS=-L/usr/local/lib/log4cpp -llog4cpp -L./$(LIBPATH)/tinyxml -ltinyxml -lpthread -static
INCPATH=-I./$(BASEPATH) -I/usr/local/include/log4cpp -I./$(LIBPATH)/tinyxml -I./$(COMMANDPATH)

TARGET=ping.test

OBJECTS=wIO.o \
		wSocket.o \
		wSignal.o \
		wMisc.o \
		wLog.o \
		wPing.o \
		PingTest.o

$(TARGET): $(OBJECTS)
	$(CXX) $(OBJECTS) -o $(TARGET) $(LIBS)

all: $(TARGET)

wIO.o: $(BASEPATH)/wIO.cpp
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $@ $<

wSocket.o: $(BASEPATH)/wSocket.cpp
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $@ $<

wSignal.o: $(BASEPATH)/wSignal.cpp
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $@ $<

wMisc.o: $(BASEPATH)/wMisc.cpp
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $@ $<

wLog.o: $(BASEPATH)/wLog.cpp
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $@ $<

wPing.o: $(BASEPATH)/wPing.cpp
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $@ $<

PingTest.o: PingTest.cpp
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $@ $<

clean:
	-rm -f $(TARGET) $(OBJECTS)
	