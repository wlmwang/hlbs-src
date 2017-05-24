
###############################
# Copyright (C) Anny Wang.
# Copyright (C) Hupu, Inc.
###############################

#
# gcc 4.8+(gdb7.6+)
# 需要预先编译vendor目录下的protobuf软件包；core下hnet
# 所需的.so文件建议安装到ldconfig加载路径中(/usr/local/lib)
#

CC		:= g++
CFLAGS	:= -Wall -O3 -std=c++11 -D_USE_LOGGER_ -D_DEBUG_
ARFLAGS	:= -Wl,-dn #-Wl,-Bstatic
LDFLAGS	:= -Wl,-dy #-Wl,-Bdynamic

# 第三方库
DIR_INC		:= -I/usr/local/include -I/usr/local/include/tinyxml -I/usr/local/include/hnet
DIR_LIB		:= -L/usr/local/lib
LIBFLAGS	:= ${DIR_LIB} ${ARFLAGS} -lhnet -ltinyxml ${LDFLAGS} -lpthread

# 主目录,message,command目录
DIR_SRC		:= ./src
DIR_MSG		:= ../message
DIR_CMD		:= ../command
DIR_COM		:= ../common

# 头文件
INCFLAGS	:= ${DIR_INC} -I${DIR_SRC} -I${DIR_MSG} -I${DIR_CMD} -I${DIR_COM}

# 源文件
CPP_SRC	:= $(wildcard ${DIR_SRC}/*.cpp)
CC_SRC	:=

# 编译文件
OBJ		:= $(patsubst %.cpp, ${DIR_SRC}/%.o, $(notdir ${CPP_SRC})) $(patsubst %.cc, ${DIR_SRC}/%.o, $(notdir ${CC_SRC}))

TARGET	:= agentsvrd

.PHONY:all clean install

all: ${TARGET}

${TARGET}: ${OBJ}
	${CC} ${CFLAGS} $^ -o $@ ${LIBFLAGS}

${DIR_SRC}/%.o:${DIR_SRC}/%.cpp
	${CC} ${CFLAGS} ${INCFLAGS} -c $< -o $@

${DIR_SRC}/%.o:${DIR_MSG}/%.cc
	${CC} ${CCFLAGS} ${INCFLAGS} -c $< -o $@

clean:
	-rm -f ${TARGET} ${DIR_SRC}/*.o ${DIR_MSG}/*.o
	-rm -rf ./bin ./log
	
install:
	-rm -rf ./bin ./log
	-mkdir ./bin ./log
	-mv ${TARGET} ./bin
