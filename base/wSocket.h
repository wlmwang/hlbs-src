
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

#include "wType.h"

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
			mSocketType = CONNECT_SOCKET;
			mSocketFlag = RECV_DATA;
			mCreateTime = time(NULL);
			mRecvTime = 0;
			mSendTime = 0;
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
		
		int SetNonBlock(bool bNonblock = true);

		int SetTimeout(int iTimeout = 30);	//30s
		int SetSendTimeout(int iTimeout = 30);
		int SetRecvTimeout(int iTimeout = 30);

		int RecvBytes(char *vArray, int vLen);
		int SendBytes(char *vArray, int vLen);

		int & SocketFD() { return mSocketFD; }
		
		string & IPAddr() { return mIPAddr; }
		
		unsigned short & Port() { return mPort; }
		
		int & SocketType() { return mSocketType; }
		
		int & SocketFlag() { return mSocketFlag; }
		
		unsigned long long & RecvTime() { return mRecvTime; }
		
		unsigned long long & SendTime() { return mSendTime; }
		
	protected:
		int mSocketFD;				//网络套接字描述符
		string mIPAddr;				//需要连接或者需要绑定的IP地址
		unsigned short mPort;		//需要连接或者需要绑定的端口
		int mSocketType;			//socket类型：监听socket、连接socket
		int mSocketFlag;			//socket标志：是否收包

		unsigned long long mRecvTime;			//接收到数据包的时间戳
		unsigned long long mSendTime;			//最后一次发送数据包时间戳（主要用户心跳检测）
		unsigned long long mCreateTime;			//socket的创建时间
};

#endif
