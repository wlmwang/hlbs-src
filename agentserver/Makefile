COMMONPATH=../common
LIBPATH=../lib
MESSAGEPATH=../message
CXX=g++
CXXFLAGS=-g -pg -DUSE_LOG4CPP -DLINUX #-D_DEBUG_
LIBS=-L./$(LIBPATH)/log4cpp/lib -llog4cpp -L./$(LIBPATH)/tinyxml -ltinyxml -L./$(LIBPATH)/protobuf/lib -lprotobuf -L./$(LIBPATH)/zlib/lib -lz -lpthread -static
INCPATH=-I./inc/ -I./$(COMMONPATH)/inc -I./$(LIBPATH)/log4cpp/include -I./$(LIBPATH)/zlib/include -I./$(LIBPATH)/tinyxml -I./$(LIBPATH)/protobuf/include -I./$(MESSAGEPATH)

TARGET=./bin/loginsvrd
OBJECTS=./debug/login_main.o ./debug/object_pool.o ./debug/login_server.o ./debug/log.o ./debug/server_tool.o ./debug/message_decoder.o ./debug/message.hxx.pb.o ./debug/servermessage.hxx.pb.o ./debug/clientmessage.hxx.pb.o ./debug/net_head_pb.hxx.pb.o

$(TARGET): $(OBJECTS)
	$(CXX) $(OBJECTS) -o $(TARGET) $(LIBS) $(CXXFLAGS)

all: $(TARGET)

./debug/login_main.o: src/login_main.cpp
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $@ $<

./debug/object_pool.o: src/object_pool.cpp
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $@ $<

./debug/login_server.o: src/login_server.cpp
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $@ $<

./debug/log.o: $(COMMONPATH)/src/log.cpp
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $@ $<

./debug/server_tool.o: $(COMMONPATH)/src/server_tool.cpp
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $@ $<

./debug/message_decoder.o: $(COMMONPATH)/src/message_decoder.cpp
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $@ $<

./debug/message.hxx.pb.o: $(MESSAGEPATH)/message.hxx.pb.cc
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $@ $<

./debug/servermessage.hxx.pb.o: $(MESSAGEPATH)/servermessage.hxx.pb.cc
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $@ $<

./debug/clientmessage.hxx.pb.o: $(MESSAGEPATH)/clientmessage.hxx.pb.cc
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $@ $<

./debug/net_head_pb.hxx.pb.o: $(MESSAGEPATH)/net_head_pb.hxx.pb.cc
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $@ $<

clean:
	-rm -f $(TARGET) $(OBJECTS)
