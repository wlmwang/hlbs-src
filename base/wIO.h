
/**
 * Copyright (C) Anny.
 * Copyright (C) Disvr, Inc.
 */

#ifndef _W_IO_H_
#define _W_IO_H_

#include "wType.h"
#include "wLog.h"
#include "wMisc.h"
#include "wNoncopyable.h"

enum SOCK_STATUS
{
	STATUS_UNKNOWN = -1,
	STATUS_UNCONNECT,	//关闭状态
	STATUS_CONNECTED,	//已经建立连接
	STATUS_LISTEN		//已经处于监听状态
};

//套接字
enum SOCK_TYPE
{
	SOCK_UNKNOWN = -1,
	SOCK_LISTEN = 0,
	SOCK_CONNECT,
	SOCK_UNIX,
};

enum IO_TYPE
{
	TYPE_UNKNOWN = -1,
	TYPE_SOCK,	//tcp|udp|unix socket
	TYPE_FILE,
	TYPE_BUF,
	TYPE_SHM,
};

enum IO_FLAG
{
	FLAG_UNKNOWN = -1,
    FLAG_RECV,
    FLAG_SEND,
	FLAG_RVSD
};

class wIO : private wNoncopyable
{
	public:
		wIO();
		void Initialize();
		virtual ~wIO();
		
		int &FD() { return mFD; }
		IO_TYPE &IOType() { return mIOType; }
		IO_FLAG &IOFlag() { return mIOFlag; }
		
		SOCK_TYPE &SockType() { return mSockType;}
		SOCK_STATUS &SockStatus() { return mSockStatus;}
		
		unsigned long long &RecvTime() { return mRecvTime; }
		unsigned long long &SendTime() { return mSendTime; }
		unsigned long long &CreateTime() { return mCreateTime; }

		virtual string &Host() { return mHost; }
		virtual unsigned short &Port() { return mPort; }

		//30s
		virtual int SetTimeout(int iTimeout = 30) {}
		virtual int SetSendTimeout(int iTimeout = 30) {} 
		virtual int SetRecvTimeout(int iTimeout = 30) {}
		
		virtual int SetNonBlock(bool bNonblock = true);
		
		virtual int Open();
		virtual void Close();
		
		virtual ssize_t RecvBytes(char *vArray, size_t vLen) = 0;
		virtual ssize_t SendBytes(char *vArray, size_t vLen) = 0;
		
	protected:
		int mFD;
		IO_TYPE mIOType;
		IO_FLAG mIOFlag;
		
		SOCK_TYPE mSockType;
		SOCK_STATUS mSockStatus;

		string mHost;
		unsigned short mPort;
		
		unsigned long long mRecvTime;	//最后接收到数据包的时间戳，毫秒
		unsigned long long mSendTime;	//最后发送数据包时间戳（主要用户心跳检测），毫秒
		unsigned long long mCreateTime;	//创建时间，毫秒
};

#endif
