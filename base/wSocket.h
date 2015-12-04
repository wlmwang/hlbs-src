
/**
 * Copyright (C) Anny.
 * Copyright (C) Disvr, Inc.
 */

/**
 *  网络TCP套接字的基础类
 */
#ifndef _W_SOCKET_H_
#define _W_SOCKET_H_

#include <unistd.h>
#include <fcntl.h>
#include <string>

enum SOCKET_TYPE
{
	LISTEN_SOCKET = 1,
	CONNECT_SOCKET,
};

enum SOCKET_FLAG
{
    RECV_DATA = 1,
    SEND_DATA,
};

//套接字状态
enum ESocketStatus
{
	SOCKET_STATUS_UNCONNECT	= 0,	//关闭状态
	SOCKET_STATUS_CONNECTED,		//已经建立连接
	SOCKET_STATUS_LISTEN			//已经处于监听状态
};

class wSocket
{
	public:
		wSocket()
		{
			Initialize();
		}

		virtual ~wSocket()
		{
			Close();
		}

		//初始化
		void Initialize()
		{
			mIPAddr = "";
			mPort = 0;
			mSocketFD = -1;
		}

		bool IsConnected()
		{
			return mSocketFD != -1;
		}

		void Close()
		{
			if(mSocketFD >= 0) 
			{
				close(mSocketFD);
			}
			mSocketFD = -1;
		}

		int SetNonBlock()
		{
			if( mSocketFD < 0) 
			{
				return -1;
			}

			int iFlags = fcntl(mSocketFD, F_GETFL, 0);
			if( iFlags == -1 ) 
			{
				return -1;
			}
			return fcntl(mSocketFD, F_SETFL, iFlags|O_NONBLOCK);
		}
		
		int mSocketFD;				// 网络套接字描述符
		string mIPAddr;				//需要连接或者需要绑定的IP地址
		unsigned short mPort;		// 需要连接或者需要绑定的端口

};

#endif
