
/**
 * Copyright (C) Anny.
 * Copyright (C) Disvr, Inc.
 */

#ifndef _W_TYPE_H_
#define _W_TYPE_H_

#include <cctype>

#define SAFE_DELETE(x) { if (x) { delete (x); (x) = NULL; } }
#define SAFE_DELETE_VEC(x) { if (x) { delete [] (x); (x) = NULL; } }

#define SAFE_SUB(x, y) ((x) > (y) ? (x)-(y) : 0)

#define CAST_CMD(x,y,t)	const t* x = (const t*)y;

//单字节无符号整数
typedef unsigned char BYTE;

//双字节无符号整数
typedef unsigned short WORD;

//双字节符号整数
typedef signed short SWORD;

//四字节无符号整数
typedef unsigned int DWORD;

//四字节符号整数
typedef signed int SDWORD;

//八字节无符号整数
typedef unsigned long QWORD;

//八字节符号整数
typedef signed long SQWORD;

//名字的最大长度
#define MAX_NAME_LEN 32

//最长的IP地址字符串长度
#define MAX_IP_LEN 16

//消息缓冲区大小
#define MAX_PACKAGE_LEN 100 * 1024

//客户端消息的最长长度
#define MAX_CLIENT_MSG_LEN MAX_PACKAGE_LEN

//客户端消息的最短长度
#define MIN_CLIENT_MSG_LEN 4

//心跳包间隔时间
#define KEEPALIVE_TIME 1000 /* 1 * 1000 */

#define CHECK_CLIENT_TIME 3000

//客户端未发送消息的间隔超时时间
#define SOCKET_TIMEOUT 300

//客户端从来未发送消息的超时时间
#define SOCKET_SEND_TIMEOUT 300

//每一个服务器之间的发送接收缓冲区的大小
#define MAX_TCP_BUFFER_LEN 10485760 /* 10 * 1024 * 1024 */

#define MAX_SEND_BUFFER_LEN 1048576 /* 1024 * 1024 */

#define MAX_RECV_BUFFER_LEN 100 * 1024



//消息队列的长度
#ifdef _DEBUG_
#define MSG_QUEUE_LEN 4194304 /* 1024 * 1024 * 4 */
#else
#define MSG_QUEUE_LEN 16777216 /* 1024 * 1024 * 16 */
#endif

//断连客户端时所需要的缓冲区长度
#define DISCONNECT_MSG_LEN 30

//网关服务器对客户端的缓冲区大小
#define MAX_CLIENT_MSG_BUFFER_LEN 153600 /* 150 * 1024 */

//服务器之间如果断连，下一次检测重连的时间
#define RECONNECT_TIME 60000 /* 60 * 1000 */

//向服务器注册，下一次检测重连的时间
#define REGISTER_TIME 3000 /* 3 * 1000 */
using namespace std;
#endif
