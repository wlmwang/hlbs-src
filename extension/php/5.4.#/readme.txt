
/**
 * Copyright (C) Anny Wang.
 * Copyright (C) Hupu, Inc.
 */

# 该目录下所有文件均来自其他目录，并已其他目录为准
# 建议在编译时实时覆盖该目录所有文件

$ cp -r /usr/local/webserver/hlbs/extension/c++/common/* .
$ cp -r /usr/local/webserver/hlbs/command/* .

# 静态链接库（防止编译时使用动态库，将静态库复制到该目录下） 
# tinyxml protobuf hnet

$ cp -r /usr/local/lib/libtinyxml.a .
$ cp -r /usr/local/lib/libprotobuf.a .
$ cp -r /usr/local/lib/libhnet.a .
