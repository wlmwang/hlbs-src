
/**
 * Copyright (C) Anny Wang.
 * Copyright (C) Hupu, Inc.
 */

#ifndef _W_SOCKET_H_
#define _W_SOCKET_H_

#include <sys/socket.h>
#include <arpa/inet.h>
#include <poll.h>

#include "wCore.h"
#include "wIO.h"
#include "wLog.h"
#include "wMisc.h"

#define ERR_TIMEO -4	//connect连接超时

/**
 *  网络TCP套接字的基础类
 */
class wSocket : public wIO
{
	public:
		wSocket();
		void Initialize();
		virtual ~wSocket();
		
		virtual int SetTimeout(float fTimeout = 30);	//单位：秒
		virtual int SetSendTimeout(float fTimeout = 30);
		virtual int SetRecvTimeout(float fTimeout = 30);

		virtual ssize_t RecvBytes(char *vArray, size_t vLen);
		virtual ssize_t SendBytes(char *vArray, size_t vLen);

		int Open();
		int SetKeepAlive(int iIdle = 5, int iIntvl = 1, int iCnt = 10);	//tcp保活
		int Bind(string sIpAddr ,unsigned int nPort);
		int Listen(string sIpAddr ,unsigned int nPort);
		int Connect(string sIpAddr ,unsigned int nPort, float fTimeout = 30);
		int Accept(struct sockaddr* pClientSockAddr, socklen_t *pSockAddrSize);
};

#endif
