
###############################
# Copyright (C) Anny Wang.
# Copyright (C) Hupu, Inc.
###############################

CXX=g++ -std=c++11
CXXFLAGS=-O3 -fPIC -pipe -fno-ident -Wall
LDFLAGS=-shared

COREPATH=../../core
COMMPATH=../../command
INCPATH=-I./$(COREPATH) -I./$(COMMPATH)
LDPATH=

SRCPATH=./
OBJECTS=./$(COREPATH)/wStatus.o \
		./$(COREPATH)/wMisc.o \
		./$(COREPATH)/wLogger.o \
		./$(COREPATH)/wSocket.o \
		./$(COREPATH)/wTcpSocket.o \
		./$(COREPATH)/wUnixSocket.o \
		./$(COREPATH)/wTask.o \
		\
		./$(SRCPATH)/agent_api.o

TARGET=libagent_api.so

.PHONY:all clean install

all: $(TARGET)

$(TARGET):$(OBJECTS)
	@echo "Compiling $@"
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LDFLAGS)

%.o:%.cpp
	$(CXX) $(CXXFLAGS) $(INCPATH) -c $< -o $@

%.d:%.cpp
	@set -e; rm -f $@; $(CXX) -MM $< $(INCPATH) > $@.$$$$; \
	sed 's,\($*\)\.o[ :]*,\1.o $@ : ,g' < $@.$$$$ > $@; \
	rm -f $@.$$$$

-include $(OBJECTS:.o=.d)

clean:
	-rm -f $(COREPATH)/*.o $(COREPATH)/*.d $(COREPATH)/*.d.*
	-rm -f $(TARGET) $(SRCPATH)/*.o $(SRCPATH)/*.d $(SRCPATH)/*.d.*
	
install:
	-mv -f $(TARGET) /usr/lib
