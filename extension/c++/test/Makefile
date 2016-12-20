
###############################
# Copyright (C) Anny Wang.
# Copyright (C) Hupu, Inc.
###############################

CXX=g++ -std=c++11
CXXFLAGS=-O3 -Wall

COREPATH=../../../core
COMMPATH=../../../command
INCPATH=-I./$(COREPATH) -I./$(COMMPATH) -I../
LDFLAGS=-L/usr/local/lib -lprotobuf -lpthread -lagent_api

SRCPATH=./
OBJECTS=./$(SRCPATH)/test.o

$(TARGET): $(OBJECTS)
	$(CXX) $(OBJECTS) -o $(TARGET) $(LDFLAGS)

TARGET=agent_test

.PHONY:all clean install

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LDFLAGS)

%.o:%.cpp
	$(CXX) $(CXXFLAGS) $(INCPATH) -c $< -o $@

%.d:%.cpp
	@set -e; rm -f $@; $(CXX) -MM $< $(INCPATH) > $@.$$$$; \
	sed 's,\($*\)\.o[ :]*,\1.o $@ : ,g' < $@.$$$$ > $@; \
	rm -f $@.$$$$

-include $(OBJECTS:.o=.d)

clean:
	-rm -f $(TARGET) $(SRCPATH)/*.o $(SRCPATH)/*.d $(SRCPATH)/*.d.*