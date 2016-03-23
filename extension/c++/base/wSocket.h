
/**
 * Copyright (C) Anny.
 * Copyright (C) Disvr, Inc.
 */

#ifndef _W_SOCKET_H_
#define _W_SOCKET_H_

#include <sys/socket.h>
#include <arpa/inet.h>

#include "wCore.h"
#include "wIO.h"
#include "wMisc.h"

/**
 *  网络TCP套接字的基础类
 */
class wSocket : public wIO
{
	public:
		wSocket();
		void Initialize();
		virtual ~wSocket();
		
		virtual int SetTimeout(int iTimeout = 30);	//单位：秒
		virtual int SetSendTimeout(int iTimeout = 30);
		virtual int SetRecvTimeout(int iTimeout = 30);

		virtual ssize_t RecvBytes(char *vArray, size_t vLen);
		virtual ssize_t SendBytes(char *vArray, size_t vLen);

		int Open();
		int Bind(string sIpAddr ,unsigned int nPort);
		int Listen(string sIpAddr ,unsigned int nPort);
		int Connect(string sIpAddr ,unsigned int nPort);
		int Accept(struct sockaddr* pClientSockAddr, socklen_t *pSockAddrSize);

	protected:
		int mErr;
};

#endif
