
/**
 * Copyright (C) Anny Wang.
 * Copyright (C) Hupu, Inc.
 */

#ifndef _W_UDSOCKET_H_
#define _W_UDSOCKET_H_

#include <sys/socket.h>
#include <sys/un.h>
#include <poll.h>

#include "wCore.h"
#include "wIO.h"
#include "wMisc.h"

/**
 *  UNIX Domain Socket基础类
 */
class wUDSocket : public wIO
{
	public:
		wUDSocket() {}
		virtual ~wUDSocket() {}

		virtual ssize_t RecvBytes(char *vArray, size_t vLen);
		virtual ssize_t SendBytes(char *vArray, size_t vLen);

		int Open();
		int Listen(string sPathAddr);
		int Connect(string sPathAddr, float fTimeout = 30);
		int Accept(struct sockaddr* pClientSockAddr, socklen_t *pSockAddrSize);
		
	protected:
		int Bind(string sPathAddr);
};

#endif
