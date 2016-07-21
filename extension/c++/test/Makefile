
###############################
# Copyright (C) Anny Wang.
# Copyright (C) Hupu, Inc.
###############################

CXX=g++
CXXFLAGS=-g -std=c++11 -Wall -Wno-invalid-offsetof
LDFLAGS=-L/usr/local/lib -lagent_api -llog4cpp -lpthread #-static

BASEPATH=../../../base
COMMPATH=../command
INCPATH=-I./$(BASEPATH) -I./$(COMMPATH) -I../ -L/usr/local/include/log4cpp

TARGET=agent_test

OBJECTS=test.o

$(TARGET): $(OBJECTS)
	$(CXX) $(OBJECTS) -o $(TARGET) $(LDFLAGS)

.PHONY:all
all: $(TARGET)

%.o:%.cpp
	$(CXX) $(CXXFLAGS) $(INCPATH) -c $< -o $@

%.d:%.cpp
	@set -e; rm -f $@; $(CXX) -MM $< $(INCPATH) > $@.$$$$; \
	sed 's,\($*\)\.o[ :]*,\1.o $@ : ,g' < $@.$$$$ > $@; \
	rm -f $@.$$$$

-include $(OBJECTS:.o=.d)

.PHONY:clean
clean:
	-rm -f $(BASEPATH)/*.o $(BASEPATH)/*.d $(BASEPATH)/*.d.*
	-rm -f $(TARGET) $(OBJECTS) ./*.d ./*.d.*