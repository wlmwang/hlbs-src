
/**
 * Copyright (C) Anny.
 * Copyright (C) Disvr, Inc.
 */

/**
 *  实现了类似监听端口的服务器
 */

#ifndef _W_TCP_LISTEN_H_
#define _W_TCP_LISTEN_H_

#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <errno.h>

#include "wTcpBase.h"

//wTCPListen返回的错误码
enum ETCPListenError
{
	ETL_IPADDR_NOT_VALID		= -1,		//IP地址非法
	ETL_SOCKET_CREATE_FAILED	= -2,		//socket创建失败
	ETL_SET_REUSE_FAILED		= -3,		//设置重用IP端口失败
	ETL_BIND_FAILED				= -4,		//绑定失败
	ETL_LISTEN_FAILED			= -5,		//监听失败
	ETL_CONNECT_NOT_READY		= -6,		//监听端口未连接
	ETL_ACCEPT_FAILED			= -7,		//accept失败
};

class wTCPListen :public wTCPBase
{
	public:
		wTCPListen() {}
		~wTCPListen() {}

		//使用内部保存的IP端口建立监听端口
		int ListenToAddress()
		{
			//创建socket
			mSocketFD = socket(AF_INET, SOCK_STREAM, 0);
			if(mSocketFD < 0)
			{
				LOG_ERROR("default", "socket failed: %s", strerror(errno));
				return ETL_SOCKET_CREATE_FAILED;
			}

			//重用IP地址
			int iReuseFlag = 1;
			if(setsockopt(mSocketFD, SOL_SOCKET, SO_REUSEADDR, &iReuseFlag, sizeof(int)) != 0)
			{
				LOG_ERROR("default", "set socket address reusable failed: %s", strerror(errno));
				Close();
				return ETL_SET_REUSE_FAILED;
			}

			sockaddr_in stSockAddr;
			memset((void *)&stSockAddr, 0, sizeof(sockaddr_in));
			stSockAddr.sin_family = AF_INET;
			stSockAddr.sin_port = htons(mPort);
			stSockAddr.sin_addr.s_addr = mIPAddress;

			//绑定IP地址
			if(bind(mSocketFD, (const struct sockaddr *)&stSockAddr, sizeof(stSockAddr)) != 0)
			{
				LOG_ERROR("default", "bind ip and port failed: %s", strerror(errno));
				Close();
				return ETL_BIND_FAILED;
			}

			//设置为监听端口
			if(listen(mSocketFD, 1024) != 0) 
			{
				LOG_ERROR("default", "listen failed: %s", strerror(errno));
				Close();
				return ETL_ACCEPT_FAILED;
			}

			SetNoBlock();
			return 0;
		}

		//监听到某个端口
		int ListenToAddress(char *vIPAddress, unsigned short vPort)
		{
			if(vIPAddress == NULL) 
			{
				return ETL_IPADDR_NOT_VALID;
			}

			//如果有连接就先关闭
			Close();

			//设置现在的IP和端口
			mIPAddress = inet_addr(vIPAddress);
			mPort = vPort;

			//使用内部保存的IP端口建立监听端口
			return ListenToAddress();
		}
};

#endif
