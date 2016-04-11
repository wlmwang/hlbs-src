
###############################
# Copyright (C) Anny Wang.
# Copyright (C) Hupu, Inc.
###############################
 
BASEPATH=../base

LIBPATH=../lib

CXX=g++

CXXFLAGS=-g -std=c++11 -Wno-invalid-offsetof

LIBS=-L/usr/lib -lagent_api
INCPATH=-I./$(BASEPATH) -I..

TARGET=test

OBJECTS=test.o

$(TARGET): $(OBJECTS)
	$(CXX) $(OBJECTS) -o $(TARGET) $(LIBS)

all: $(TARGET)

test.o: test.cpp
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $@ $<

clean:
	-rm -f $(TARGET) $(OBJECTS)